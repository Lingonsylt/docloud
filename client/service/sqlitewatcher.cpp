#include <windows.h>
#include <stdio.h>
#include "sqlite.h"
#include "sqlitewatcher.h"
#include "docloudfile.h"
#include "reg.h"

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
	
	if (sqlite_connect() == -1)
		return -1;

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
		wprintf(L"Added %s to list\n", path);
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
			    "SELECT id FROM docloud_files WHERE updated <> uploaded OR uploaded = 0",
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
				wprintf(L"File %s [%d] [", dc_file->filename.c_str(), dc_file->id);

				if (PathIsDirectory(dc_file->filename.c_str())) {
					/* FIXME - handle the case where we have added a file inside
					 * this directory and then blacklist it -
					 * we should still be listening for changes in the directory
					 * but only act when the added file changes
					 */
					if (dc_file->blacklisted)
						dirwatcher->remDirectory(dc_file->filename.c_str());
					else if (dc_file->uploaded == 0) /* We havent seen this before */
						dirwatcher->addDirectory(dc_file->filename.c_str());
				} else if (! dc_file->blacklisted) {
					std::wstring dirpath = dc_file->filename;
					dirpath.erase(dirpath.find_last_of(L"\\/"));
					dirwatcher->addDirectory(dc_file->filename.c_str());
				}

				std::vector<doCloudFileTag*>::iterator it;
				for (it = dc_file->tags.begin(); it != dc_file->tags.end(); it ++) {
//				for (auto it : dc_file->tags) {
					wprintf(L"%d:%s,", (*it)->id, (*it)->name.c_str());
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



