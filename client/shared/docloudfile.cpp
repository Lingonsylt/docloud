#include <windows.h>
#include <sqlite3.h>
#include <stdio.h>
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
	filename = "";
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
	const char *cstr_name;
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
		tag->name = (const char *)sqlite3_column_text(stmt, 1);
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
		printf("Cannot prepare statement: %s\n", sqlite3_errmsg(sqlite_db));
		id = -1;
		return -1;
	}
	sqlite3_bind_int(stmt, 1, searchId);

	rv = sqlite3_step(stmt);

	if (rv != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		if (rv == SQLITE_DONE) {
			/* No file with that id */
			printf("No file with that id!\n");
			id = -1;
			return 0;
		}
		/* Error occurred! */
		printf("sqlite3_step(): %s!\n", sqlite3_errmsg(sqlite_db));
		return -1;
	}

	this->id = sqlite3_column_int(stmt, 0);
	this->filename = (const char *)sqlite3_column_text(stmt, 1);
	this->blacklisted = sqlite3_column_int(stmt, 2);
	this->updated = sqlite3_column_int(stmt, 3);
	this->uploaded = sqlite3_column_int(stmt, 4);

	sqlite3_finalize(stmt);

	getFileTags();

	return 1;
}


int
doCloudFile::getFromPath(const char *path)
{
	struct sqlite3_stmt *stmt;
	std::string query;
	int n_subdirs;
	std::string path_str;
	std::vector<std::string> path_list;
	int ret;
	int i;

	clear();
	if (sqlite_connect() == -1)
		return -1;

	/* Search for matching parents */
	path_str = path;
	while (path_str.find_last_of("\\/") != std::string::npos) {
		path_str.erase(path_str.find_last_of("\\/"));
		if (path_str.length() < MIN_PATH_LEN) break;
		path_list.push_back(path_str);
	}
	n_subdirs = path_list.size();

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
	ret = sqlite3_bind_text(stmt, 1, path, -1, NULL);

	int col = 2;

	/* Now, bind all subdirs */
	std::vector<std::string>::iterator it;
	for (it = path_list.begin(); it != path_list.end(); it++) {
		log(" binding %s\n", (*it).c_str());
		sqlite3_bind_text(stmt, col++, (*it).c_str(), -1, NULL);
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_DONE) {
		log("No match found for %s\n", path);
		sqlite3_finalize(stmt);
		return 0;
	} else if (ret != SQLITE_ROW) {
		printf("Error in %d sqlite3_step(): %s\n", __LINE__,
		    sqlite3_errmsg(sqlite_db));
		sqlite3_finalize(stmt);
		return -1;
	}

	this->id = sqlite3_column_int(stmt, 0);
	this->filename = (const char *)sqlite3_column_text(stmt, 1);
	this->blacklisted = sqlite3_column_int(stmt, 2);
	this->updated = sqlite3_column_int(stmt, 3);
	this->uploaded = sqlite3_column_int(stmt, 4);

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

	printf("Updating db-info for file %s\n", filename.c_str());

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
	ret += sqlite3_bind_text(stmt, 1, filename.c_str(), -1, NULL);
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
