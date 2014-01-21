#ifndef DIRWATCHER_H
#define DIRWATCHER_H
#include <string>
#include <map>
#include <vector>
#include "docloudfile.h"

typedef struct t_directory {
	std::string path;
	HANDLE handle;
	FILE_NOTIFY_INFORMATION *buffer;
	OVERLAPPED overlapped;
	unsigned long long key;
	std::vector<doCloudFile *> files;

	public:
		t_directory(): buffer(NULL), key(0), handle(NULL) { };
		~t_directory() {
			if (handle)
				CloseHandle(handle);
			if (buffer)
				delete buffer;
		}
}directory;


class dirWatcher {
	public:
		dirWatcher();
		~dirWatcher();
		int init();
		int addDirectory(const char *dir);
		int remDirectory(const char *dir);

		int addFile(doCloudFile *file);
		int remFile(doCloudFile *file);

		int loadDirList();

		int watch();
		int work();
	private:
		HANDLE hIOCP;
		HANDLE updatePort;

		SRWLOCK dirLock;
		std::map<unsigned long long, directory *> dirs;

		unsigned long long max_key;
};

#endif /* end of include guard: DIRWATCHER_H */
