#include <windows.h>
#include <sqlite3.h>
#include "sqlite.h"
#include "docloudfile.h"

doCloudFile::doCloudFile():
	id(-1),
	blacklisted(0),
	updated(0),
	uploaded(0)
{

}

int
doCloudFile::clear()
{
	id = -1;
	filename = L"";
	blacklisted = 0;
	updated = 0;
	uploaded = 0;
	tags.clear();
}

int
doCloudFile::getFileTags()
{
	int rv;
	const wchar_t *cstr_name;
	struct sqlite3_stmt *stmt;

	if (this->id == -1) return 0;

	rv = sqlite3_prepare(sqlite_db,
	    "SELECT id, name FROM docloud_file_tags INNER JOIN docloud_tags ON id = tag_id WHERE file_id = ?",
	    -1, &stmt, NULL);

	if (rv == SQLITE_ERROR) {
		wprintf(L"Cannot prepare statement: %s\n", sqlite3_errmsg(sqlite_db));
		return -1;
	}
	sqlite3_bind_int(stmt, 1, this->id);

	doCloudFileTag *tag;
	while ((rv = sqlite3_step(stmt)) == SQLITE_ROW) {
		tag = new doCloudFileTag;
		tag->id = sqlite3_column_int(stmt, 0);
		cstr_name = (wchar_t*)sqlite3_column_text16(stmt, 1);
		tag->name = cstr_name;
		this->tags.push_back(tag);
	}

	sqlite3_finalize(stmt);
}

int
doCloudFile::getFromId(int searchId) {
	struct sqlite3_stmt *stmt;
	int rv;

	clear();
	if (sqlite_connect() == -1)
		return -1;

	rv = sqlite3_prepare(sqlite_db,
	    "SELECT id, filename, blacklisted, updated, uploaded FROM docloud_files WHERE id = ?",
	    -1, &stmt, NULL);
	if (rv == SQLITE_ERROR) {
		wprintf(L"Cannot prepare statement: %s\n", sqlite3_errmsg(sqlite_db));
		id = -1;
		return -1;
	}
	sqlite3_bind_int(stmt, 1, searchId);

	rv = sqlite3_step(stmt);

	if (rv != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		if (rv == SQLITE_DONE) {
			/* No file with that id */
			wprintf(L"No file with that id!\n");
			id = -1;
			return 0;
		}
		/* Error occurred! */
		wprintf(L"sqlite3_step(): %s!\n", sqlite3_errmsg(sqlite_db));
		return -1;
	}

	const wchar_t *cstr_name;
	this->id = sqlite3_column_int(stmt, 0);
	cstr_name = (wchar_t*)sqlite3_column_text16(stmt, 1);
	this->blacklisted = sqlite3_column_int(stmt, 2);
	this->updated = sqlite3_column_int(stmt, 3);
	this->uploaded = sqlite3_column_int(stmt, 4);
	this->filename = cstr_name;

	sqlite3_finalize(stmt);

	getFileTags();

	return 1;
}


int
doCloudFile::getFromPath(const wchar_t *path)
{
	struct sqlite3_stmt *stmt;
	std::string query;
	int n_subdirs;
	const wchar_t *ptr;
	int ret;
	int i;

	clear();
	if (sqlite_connect() == -1)
		return -1;

	/* Search for matching parents */
	for (n_subdirs = 0, ptr = path; ptr != NULL;) {
		ptr = wcschr(ptr, L'\\');
		if (ptr != NULL) ptr ++;
		if ((ptr - path) > MIN_PATH_LEN)
			n_subdirs++;
	}

	query = "SELECT id, filename, blacklisted, updated, uploaded FROM docloud_files WHERE filename in (?,";
	for (i = 0; i < n_subdirs; i++) {
		query += "?";
		if (i != n_subdirs - 1) query += ",";
	}
	query += ") ORDER BY filename DESC LIMIT 1";

	ret = sqlite3_prepare_v2(sqlite_db, query.c_str(), -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		wprintf(L"Cannot prepare statement %s: %s\n",
		    query.c_str(), sqlite3_errmsg(sqlite_db));
		return -1;
	}

	/* First, bind complete filename */
	ret = sqlite3_bind_text16(stmt, 1, path, -1, NULL);
	/* Now, bind all subdirs */
	for (i = 2, ptr = path; ptr != NULL;) {
		ptr = wcschr(ptr, L'\\');
		if (ptr == NULL) break;
		if ((ptr - path) > MIN_PATH_LEN) {
			/* Note - sqlite3_bind_text16 wants the length in
			 * BYTES, not in characters!
			 */
			ret = sqlite3_bind_text16(stmt, i, path, ptr - path, NULL);
			wprintf(L"%d: %ld %.*S\n", i, 
			    ((long long)ptr - (long long)path) /
			    sizeof(wchar_t),
			    ((long long)ptr - (long long)path)
			    /sizeof(wchar_t),
			    path);
			i++;
		}
		ptr ++;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE) {
		wprintf(L"No match found for %S\n", path);
		sqlite3_finalize(stmt);
		return 0;
	} else if (ret != SQLITE_ROW) {
		wprintf(L"Error in %d sqlite3_step(): %s\n", __LINE__,
		    sqlite3_errmsg(sqlite_db));
		sqlite3_finalize(stmt);
		return -1;
	}

	const wchar_t *cstr_name;
	this->id = sqlite3_column_int(stmt, 0);
	cstr_name = (wchar_t*)sqlite3_column_text16(stmt, 1);
	this->blacklisted = sqlite3_column_int(stmt, 2);
	this->updated = sqlite3_column_int(stmt, 3);
	this->uploaded = sqlite3_column_int(stmt, 4);
	this->filename = cstr_name;

	sqlite3_finalize(stmt);
	getFileTags();

	return 0;
}


