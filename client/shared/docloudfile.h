#ifndef DOCLOUDFILE_H
#define DOCLOUDFILE_H
#include <string>
#include <vector>

typedef struct __doCloudFileTag {
		int id;
		std::string name;
}doCloudFileTag;

class doCloudFile {
	public:
		doCloudFile();
		~doCloudFile();
		int getFromId(int searchId);
		int getFromPath(const char *path);
		int clear();

		int save();
	
		int id;
		std::string filename;
		int blacklisted;
		time_t updated;
		time_t uploaded;
		std::vector<doCloudFileTag*> tags;

		int matches_parent;
	private:
		int getFileTags();
};
#endif /* end of include guard: DOCLOUDFILE_H */
