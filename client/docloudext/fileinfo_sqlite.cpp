#include <string>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include "sqlite.h"
#include "fileinfo.h"

/* Used for debugging */
#define log(str, ...)

const char *suffix_table[] = { 
	".doc",
	".docx",
	".pdf",
	NULL
};

int docloud_check_subdirs(struct file_info *file) {
	struct sqlite3_stmt *stmt;
	std::string query;
	int n_subdirs;
	char *ptr;
	int ret;
	int i;

	/* Search for matching parents */
	for (n_subdirs = 0, ptr = file->filename; ptr != NULL;) {
		ptr = strchr(ptr, (int)'\\');
		if (ptr != NULL) ptr ++;
		if ((ptr - file->filename) > MIN_PATH_LEN)
			n_subdirs++;
	}

	query = "SELECT id, blacklisted FROM docloud_files WHERE filename in (";
	for (i = 0; i < n_subdirs; i++) {
		query += "?";
		if (i != n_subdirs - 1) query += ",";
	}
	query += ") ORDER BY filename DESC LIMIT 1";

	log("%s\n", query.c_str());
	ret = sqlite3_prepare_v2(sqlite_db, query.c_str(), -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		log("Cannot prepare statement %s: %s\n",
		    query.c_str(), sqlite3_errmsg(sqlite_db));
		return DC_ERROR;
	}

	for (i = 1, ptr = file->filename; ptr != NULL;) {
		ptr = strchr(ptr, (int)'\\');
		if (ptr == NULL) break;
		if ((ptr - file->filename) > MIN_PATH_LEN) {
			ret = sqlite3_bind_text(stmt, i, file->filename, ptr - file->filename, NULL);
			log("%d: %.*s\n", i, (long long)ptr - (long long)file->filename, file->filename);
			i++;
		}
		ptr ++;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		log("Found parent, with id %d\n", sqlite3_column_int(stmt, 0));
		if (sqlite3_column_int(stmt, 1) != 0)
			file->parent_flags = DC_PARENT_BLACKLISTED;
		else
			file->parent_flags = DC_PARENT_ADDED;
	} else if (ret == SQLITE_DONE) {
		file->parent_flags = DC_PARENT_NONE;
	} else {
		log("Error in %d sqlite3_step(): %s\n", __LINE__,
		    sqlite3_errmsg(sqlite_db));
		sqlite3_finalize(stmt);
		return DC_ERROR;
	}

	sqlite3_finalize(stmt);
	return DC_OK;
}

int docloud_get_file_info(struct file_info *file)
{
	struct sqlite3_stmt *stmt;
	char *filename;
	int ret;

	printf("file: %p\n", file);
	printf("file->filename: %p\n", file->filename);
	filename = file->filename;
	if (sqlite_connect() == -1)
		return DC_ERROR;

	ret = sqlite3_prepare_v2(sqlite_db,
	    "SELECT id, blacklisted FROM docloud_files WHERE filename = ?",
	    -1, &stmt, NULL);
	if (ret != SQLITE_OK) {
		log("Cannot prepare statement: %d: %s\n",
		    ret, sqlite3_errmsg(sqlite_db));
		sqlite3_close(sqlite_db);
		sqlite_db = NULL;
		return DC_ERROR;
	}

	ret = sqlite3_bind_text(stmt, 1, filename, -1, NULL);
	if (ret != SQLITE_OK) {
		log("Cannot bind argument: %s\n",
		    sqlite3_errmsg(sqlite_db));
		sqlite3_finalize(stmt);
		sqlite3_close(sqlite_db);
		return DC_ERROR;
	}

	ret = sqlite3_step(stmt);
	if (ret == SQLITE_ROW) {
		/* We found our match, update fields! */
		file->id = sqlite3_column_int(stmt, 0);
		file->blacklisted = sqlite3_column_int(stmt, 1);
	} else if (ret == SQLITE_DONE) {
		if (!(file->parent_flags & DC_PARENT_IGNORE)) {
			sqlite3_finalize(stmt);
			stmt = NULL;
			ret = docloud_check_subdirs(file);
		}
	} else {
		log("Error in %d sqlite3_step(): %s\n", __LINE__,
		    sqlite3_errmsg(sqlite_db));
		sqlite3_finalize(stmt);
		return DC_ERROR;
	}

	sqlite3_finalize(stmt);
	return DC_OK;
}

int docloud_add_file(struct file_info *file)
{
	struct sqlite3_stmt *stmt;
	int ret;
	time_t updated;

	if (sqlite_connect() == -1)
		return DC_ERROR;

	updated = time(NULL);
	if (file->id == -1) {
		ret = sqlite3_prepare_v2(sqlite_db,
		    "INSERT INTO docloud_files (filename, blacklisted, uploaded, updated) VALUES (?, ?, 0, ?)",
		    -1, &stmt, NULL);
	} else {
		ret = sqlite3_prepare_v2(sqlite_db,
		    "UPDATE docloud_files SET filename = ?, blacklisted = ?, updated = ? WHERE id = ?",
		    -1, &stmt, NULL);
		log(
		    "UPDATE docloud_files SET filename = '%s', blacklisted = %d, updated = %d WHERE id = %d",
		    file->filename, file->blacklisted, updated, file->id);
		ret += sqlite3_bind_int(stmt, 4, file->id);
	}
	ret += sqlite3_bind_text(stmt, 1, file->filename, -1, NULL);
	ret += sqlite3_bind_int(stmt, 2, file->blacklisted);
	ret += sqlite3_bind_int(stmt, 3, updated);

	if (ret != SQLITE_OK) {
		log("Could not prepare statement: %s\n",
		    sqlite3_errmsg(sqlite_db));
		if (stmt)
			sqlite3_finalize(stmt);
		sqlite3_close(sqlite_db);
		sqlite_db = NULL;
		return DC_ERROR;
	}

	ret = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (ret == SQLITE_DONE) {
		if (file->id == -1)
			file->id = sqlite3_last_insert_rowid(sqlite_db);
		return DC_OK;
	} else {
		log("sqlite3_step: %s\n", sqlite3_errmsg(sqlite_db));
	}
	return DC_ERROR;
}

int docloud_get_tree_info(char *filename, struct file_info *file)
{
	int ret;
	char *ptr;
	char *start;
	int check_parent = 0;

	start = filename;
	ptr = start + strlen(start);
	while (ptr != start) {
		ret = docloud_get_file_info(file);
		if (ret == DC_NO_MATCH) {
			while (*ptr != '\\' && ptr != start)
				ptr --;
			if (ptr != start) {
				*ptr = '\0';
				check_parent = 1;
				printf("Trying %s instead\n", start);
			}
		} else if (ret == DC_MATCH_FULL) {
			printf("Found match for %s : %d %d\n", file->filename,
			    file->id, file->blacklisted);
			if (check_parent == 1)
				return DC_MATCH_PARENT;
			return DC_MATCH_FULL;
		} else if (ret == DC_ERROR) {
			printf("Error occurred!\n");
			return DC_ERROR;
		}
	}
	return 0;
}

int docloud_close_db()
{
	int ret = 0;
	if (sqlite_db != NULL) {
		ret = sqlite3_close(sqlite_db);
		sqlite_db = NULL;
	}
	return ret;
}

int docloud_is_correct_filetype(char *filename)
{
	char *ptr;

	ptr = strrchr(filename, '.');
	if (ptr == NULL) return 0; /* No suffix, ignore */

	for (int i = 0; suffix_table[i] != NULL; i ++) {
		if (strcmp(ptr, suffix_table[i]) == 0)
			return 1; /* We're satisfied */
	}
	
	return 0;
}
