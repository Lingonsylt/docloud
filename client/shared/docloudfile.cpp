#include <windows.h>
#include <sqlite3.h>
#include "sqlite.h"
#include "docloudfile.h"
#include "debug.h"

doCloudFile::doCloudFile():
	id(-1),
	blacklisted(0),
	updated(0),
	uploaded(0),
	matches_parent(0)
{

}

doCloudFile::~doCloudFile()
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
	matches_parent = 0;
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

	log("query: %s\n", query.c_str());
	/* First, bind complete filename */
	ret = sqlite3_bind_text16(stmt, 1, path, -1, NULL);

	std::wstring tmpstr = path;
	int col = 2;

	/* Now, bind all subdirs */
	for (;;) {
		tmpstr.erase(tmpstr.find_last_of(L"\\/"));
		if (tmpstr.length() <= MIN_PATH_LEN)
			break;

		logw(L" binding %S\n", tmpstr.c_str());
		sqlite3_bind_text16(stmt, col++, tmpstr.c_str(), -1, NULL);
	}
#if 0
	for (i = 2, ptr = path; ptr != NULL;) {
		ptr = wcschr(ptr, L'\\');
		if (ptr == NULL) break;
		if ((ptr - path) > MIN_PATH_LEN) {
			/* Note - sqlite3_bind_text16 wants the length in
			 * BYTES, not in characters!
			 */
			ret = sqlite3_bind_text16(stmt, i, path, ptr - path, NULL);
			logw(L"%d: %ld %.*S\n", i, 
			    ((long long)ptr - (long long)path) /
			    sizeof(wchar_t),
			    ((long long)ptr - (long long)path)
			    /sizeof(wchar_t),
			    path);
			i++;
		}
		ptr ++;
	}
#endif

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE) {
		logw(L"No match found for %s\n", path);
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

	if (this->filename != path)
		this->matches_parent = 1;

	sqlite3_finalize(stmt);
	getFileTags();

	return 0;
}


int
doCloudFile::save()
{
	struct sqlite3_stmt *stmt;
	int ret;
	time_t updated;

	if (sqlite_connect() == -1)
		return -1;

	wprintf(L"Updating db-info for file %s\n", filename.c_str());

	if (id == -1) {
		ret = sqlite3_prepare_v2(sqlite_db,
		    "INSERT INTO docloud_files (filename, blacklisted, uploaded, updated) VALUES (?, ?, ?, ?)",
		    -1, &stmt, NULL);
	} else {
		ret = sqlite3_prepare_v2(sqlite_db,
		    "UPDATE docloud_files SET filename = ?, blacklisted = ?, updated = ?, uploaded = ? WHERE id = ?",
		    -1, &stmt, NULL);
		log(
		    "UPDATE docloud_files SET filename = '%s', blacklisted = %d, updated = %d, uploaded = %d WHERE id = %d",
		    filename.c_str(), blacklisted, updated, uploaded, id);
		ret += sqlite3_bind_int(stmt, 5, id);
	}
	ret += sqlite3_bind_text16(stmt, 1, filename.c_str(), -1, NULL);
	ret += sqlite3_bind_int(stmt, 2, blacklisted);
	ret += sqlite3_bind_int(stmt, 3, updated);
	ret += sqlite3_bind_int(stmt, 3, uploaded);

	if (ret != SQLITE_OK) {
		log("Could not prepare statement: %s\n",
		    sqlite3_errmsg(sqlite_db));
		sqlite3_finalize(stmt);
		return -1;
	}

	ret = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (ret == SQLITE_DONE) {
		if (id == -1)
			id = sqlite3_last_insert_rowid(sqlite_db);
		return 0;
	} else {
		log("sqlite3_step: %s\n", sqlite3_errmsg(sqlite_db));
	}
	return -1;
}
