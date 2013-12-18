#include <windows.h>
#include "sqlite.h"
#include "docloudfile.h"
#include "reg.h"

struct sqlite3 *sqlite_db = NULL;

const wchar_t *
sqlite_get_db_path16()
{
	static wchar_t db_path16[MAX_PATH];
	static int found_path = 0;
	HRESULT hr;

	if (found_path) return db_path16;

	/* Set default value for subkey */
	hr = RegGetKeyString(HKEY_LOCAL_MACHINE, 
	    L"SOFTWARE\\doCloud\\doCloud", L"database",
	    db_path16, sizeof(db_path16));

	if (SUCCEEDED(hr)) {
		found_path = 1;
		return db_path16;
	}

	/* Key not set! Try to fix it! */
	/* FIXME! Get proper path (to executable maybe?)
	 * or should we even do this?
	 */
	return NULL;
}

const char *
sqlite_get_db_path()
{
	static char db_path[MAX_PATH];
	const wchar_t *db_path16;
	HRESULT hr;


	db_path16 = sqlite_get_db_path16();
	if (db_path16 == NULL)
		return NULL;

	WideCharToMultiByte(CP_UTF8, 0, db_path16, -1,
		db_path, sizeof(db_path), NULL, NULL);
	return db_path;

}

int
sqlite_connect()
{
	int rv;

	if (sqlite_db != NULL) return 0;

	const char *dbPath = sqlite_get_db_path();
	if (dbPath == NULL) {
		wprintf(L"Cannot find database path\n");
	}
	rv = sqlite3_open(dbPath, &sqlite_db);
	if (rv != SQLITE_OK) {
		wprintf(L"Cannot open database: %s\n",
		    sqlite3_errmsg(sqlite_db));
		sqlite3_close(sqlite_db);
		sqlite_db = NULL;
		return -1;
	}
}


sqliteWatcher::sqliteWatcher()
{
}

sqliteWatcher::~sqliteWatcher()
{
}

int
sqliteWatcher::watch()
{
	HANDLE hnotify;
	unsigned long dwWaitStatus;
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	FILETIME lastWrite;
	const wchar_t *db_path;
	int rv;
	
	db_path = sqlite_get_db_path16();
	if (!GetFileAttributesEx(db_path,
		GetFileExInfoStandard,
		&fileInfo)) {
		wprintf(L"ERROR GetFileAttributesEx() failed\n");
		return -1;
	}

	lastWrite.dwLowDateTime = fileInfo.ftLastWriteTime.dwLowDateTime;
	lastWrite.dwHighDateTime = fileInfo.ftLastWriteTime.dwHighDateTime;

	/* Create changehandles for all paths */
	wchar_t *path;

	path = _wcsdup(db_path);
	PathRemoveFileSpec(path);
	hnotify = FindFirstChangeNotification(path,
		    FALSE, /* Don't watch subtree */
		    FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);
	if (hnotify == INVALID_HANDLE_VALUE) {
		wprintf(L"ERROR: FindFirstChangeNotification function failed for %s\n", path);
		debug_windows(L"::%s");
		/* FIXME - what caused this? Does the path exist? */
	} else if(hnotify == NULL) {
		printf("ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
	} else {
		wprintf(L"Added %S to list\n", path);
	}

	printf("\nWaiting for notification...");
	while (TRUE) 
	{ 
		dwWaitStatus = WaitForSingleObject(hnotify,INFINITE);
		if (dwWaitStatus == WAIT_TIMEOUT) {
			wprintf(L".");
			continue;
		} else if (dwWaitStatus == WAIT_FAILED) {
			wchar_t *buf;
			debug_windows(L"WaitForMultipleObjects: %s\n");
			break;
		}

		if (dwWaitStatus > WAIT_ABANDONED_0) {
			wprintf(L"Abandoned!\n");
			continue;
		}

		if (dwWaitStatus != WAIT_OBJECT_0) continue;

	
		if (!GetFileAttributesEx(db_path,
			GetFileExInfoStandard,
			&fileInfo)) {
			wprintf(L"ERROR GetFileAttributesEx() failed\n");
			break;
		}

		if (CompareFileTime(&(fileInfo.ftLastWriteTime), &lastWrite) != 0) {
			lastWrite.dwLowDateTime = fileInfo.ftLastWriteTime.dwLowDateTime;
			lastWrite.dwHighDateTime = fileInfo.ftLastWriteTime.dwHighDateTime;

			struct sqlite3_stmt *stmt;
			doCloudFile *dc_file;

			/* FIXME! We currently have to wait for buffers to
			 * flush, otherwise our SELECT won't return anything
			 */
			Sleep(100);
			/* Get list of updated files */
			rv = sqlite3_prepare(sqlite_db,
			    "SELECT id FROM docloud_files WHERE updated > uploaded",
			    -1, &stmt, NULL);
			if (rv == SQLITE_ERROR) {
				wprintf(L"sqlite3_prepare(): %s\n",
				    sqlite3_errmsg(sqlite_db));
				sqlite3_finalize(stmt);
				return -1;
			}
			while ((rv = sqlite3_step(stmt)) == SQLITE_ROW) {
				dc_file = new doCloudFile;

				dc_file->getFromId(sqlite3_column_int(stmt, 0));
				wprintf(L"File %S [%d] [", dc_file->filename.c_str(), dc_file->id);
				
				std::vector<doCloudFileTag*>::iterator it;
				for (it = dc_file->tags.begin(); it != dc_file->tags.end(); it ++) {
//				for (auto it : dc_file->tags) {
					wprintf(L"%d:%S,", (*it)->id, (*it)->name.c_str());
				}
				wprintf(L"]\n");
			}

			if (rv == SQLITE_ERROR) {
				wprintf(L"sqlite3_prepare(): %s\n",
				    sqlite3_errmsg(sqlite_db));
			}
			sqlite3_finalize(stmt);
		}

		/* Add to list again */
		if (FindNextChangeNotification(hnotify) == FALSE) {
			debug_windows(L"FindNextChangeNotification(): %s\n");
			break;
		}
	}

	FindClose(hnotify);

#if 0
	int ret;
	ret = sqlite3_prepare_v2(db,
	    "SELECT id, blacklisted, filename, updated, uploaded FROM docloud_files WHERE updated > uploaded");
#endif

}



