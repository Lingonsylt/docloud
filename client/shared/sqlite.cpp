#include <windows.h>
#include <stdio.h>
#include "sqlite.h"
#include "reg.h"
#include "config.h"

struct sqlite3 *sqlite_db = NULL;

#define DB_NAME "\\db.sqlite"

const char *
sqlite_get_db_path()
{
	static char *db_path = NULL;

	if (db_path) return db_path;

	std::string install_path;
	int len;

	install_path = config::getStr("install_path");
	install_path += DB_NAME;

	len = install_path.length() + 1;
	db_path = new char[len];
	strcpy_s(db_path, len, install_path.c_str());

	printf("Path: %s\n", db_path);
	return db_path;

	/* If key is not set! Try to fix it? */
	/* FIXME! Get proper path (to executable maybe?)
	 * or should we even do this?
	 */
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
