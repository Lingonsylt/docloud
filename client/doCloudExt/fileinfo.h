#ifndef FILEINFO_H
#define FILEINFO_H

struct file_info {
	int id;
	int blacklisted;
	int parent_flags;
	char *filename;
};

#define DC_ERROR -1
#define DC_OK 0
#define DC_NO_MATCH 0
#define DC_MATCH_FULL 1
#define DC_MATCH_PARENT 2

#define DC_PARENT_NONE 0
#define DC_PARENT_ADDED 1
#define DC_PARENT_BLACKLISTED 2
#define DC_PARENT_IGNORE 4

int docloud_get_file_info(struct file_info *file);
int docloud_add_file(struct file_info *file);
int docloud_remove_file(struct file_info *file);
int docloud_get_tree_info(char *filename, struct file_info *file);
int docloud_close_db();
int docloud_is_correct_filetype(char *filename);

#endif /* end of include guard: FILEINFO_H */
