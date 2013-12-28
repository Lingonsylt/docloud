#include <windows.h>
#include <stdio.h>
#include "sqlite.h"
#include "sqlitewatcher.h"
#include "docloudfile.h"
#include "reg.h"
#include "common.h"

sqliteWatcher::~sqliteWatcher()
{
}

int
sqliteWatcher::watch()
{
	HANDLE hnotify;
	unsigned long dwWaitStatus;
	const char *db_path;
	int rv;
	
	if (sqlite_connect() == -1)
		return -1;

	db_path = sqlite_get_db_path();

	/* Create changehandles for all paths */
	std::wstring wide_path;
	wide_path = widen(db_path);

	wide_path.erase(wide_path.find_last_of(L"\\/"));
	hnotify = FindFirstChangeNotification(wide_path.c_str(),
		    FALSE, /* Don't watch subtree */
		    FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE);
	if (hnotify == INVALID_HANDLE_VALUE) {
		wprintf(L"ERROR: FindFirstChangeNotification function failed for %s\n", wide_path.c_str());
		debug_windows(L"::%s");
		/* FIXME - what caused this? Does the path exist? */
	} else if(hnotify == NULL) {
		printf("ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
	} else {
		wprintf(L"Added %s to list\n", wide_path.c_str());
	}

	printf("\nWaiting for notification...");
	while (TRUE) 
	{ 
		dwWaitStatus = WaitForSingleObject(hnotify,30000);
		if (dwWaitStatus == WAIT_FAILED) {
			debug_windows(L"WaitForMultipleObjects: %s\n");
			continue;
		}

		if (dwWaitStatus > WAIT_ABANDONED_0) {
			wprintf(L"Abandoned!\n");
			continue;
		}

		/* We've reached this point either because something changed in the file,
		 * or because we've had a timeout -
		 * in both cases, check if we have anything that needs to
		 * be uploaded to the server
		 */

		struct sqlite3_stmt *stmt;
		doCloudFile *dc_file;

		/* Get list of updated files */
		rv = sqlite3_prepare(sqlite_db,
		    "SELECT id FROM docloud_files WHERE updated <> uploaded OR uploaded = 0",
		    -1, &stmt, NULL);
		if (rv == SQLITE_ERROR) {
			printf("sqlite3_prepare(): %s\n",
			    sqlite3_errmsg(sqlite_db));
			sqlite3_finalize(stmt);
			return -1;
		}
		while ((rv = sqlite3_step(stmt)) == SQLITE_ROW) {
			dc_file = new doCloudFile;

			dc_file->getFromId(sqlite3_column_int(stmt, 0));
			printf("File %s [%d] [", dc_file->filename.c_str(), dc_file->id);

			if (PathIsDirectory(widen(dc_file->filename).c_str())) {
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
				std::string dirpath = dc_file->filename;
				dirpath.erase(dirpath.find_last_of("\\/"));
				dirwatcher->addDirectory(dc_file->filename.c_str());
			}

			/* FIXME - hand over the file to the worker-thread,
			 * we want it uploaded to the server
			 */

			std::vector<doCloudFileTag*>::iterator it;
			for (it = dc_file->tags.begin(); it != dc_file->tags.end(); it ++) {
				printf("%d:%s,", (*it)->id, (*it)->name.c_str());
			}
			printf("]\n");
		}

		if (rv == SQLITE_ERROR) {
			printf("sqlite3_prepare(): %s\n",
			    sqlite3_errmsg(sqlite_db));
		}
		sqlite3_finalize(stmt);

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



