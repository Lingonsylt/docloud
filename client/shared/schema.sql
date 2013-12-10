
CREATE TABLE docloud_files
(
	id INTEGER PRIMARY KEY,
	filename VARCHAR(250) COLLATE NOCASE,
	blacklisted BOOLEAN,
	updated INTEGER,
	uploaded INTEGER
);

CREATE INDEX docloud_files_idx ON docloud_files
	( filename );

CREATE TABLE docloud_tags
(
	id INTEGER PRIMARY KEY,
	name VARCHAR(50) COLLATE NOCASE
);

CREATE TABLE docloud_file_tags
(
	file_id INTEGER,
	tag_id INTEGER
);
