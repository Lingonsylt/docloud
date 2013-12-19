#include <windows.h>
#include <stdio.h>
#include "sqlite.h"
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

	wprintf(L"Path: %s\n", db_path16);
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
