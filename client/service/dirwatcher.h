#ifndef DIRWATCHER_H
#define DIRWATCHER_H
#include <string>
#include <map>

typedef struct t_directory {
	std::wstring path;
	HANDLE handle;
	FILE_NOTIFY_INFORMATION *buffer;
	OVERLAPPED overlapped;
	unsigned long long key;

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
		int addDirectory(const wchar_t *dir);
		int remDirectory(const wchar_t *dir);
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
