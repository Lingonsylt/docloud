#ifndef SQLITE_H
#define SQLITE_H
#include <sqlite3.h>
#include <shlwapi.h>
#include <vector>
#include <string>

#define debug_windows(fmt) {\
LPTSTR __dbg_win_ptr, __dbg_win_ptr_2; \
if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, \
			GetLastError(), 0, (LPTSTR)&__dbg_win_ptr, 0, NULL) != 0) { \
	for(__dbg_win_ptr_2 = __dbg_win_ptr;isprint(*__dbg_win_ptr_2);__dbg_win_ptr_2++); \
	*__dbg_win_ptr_2 = '\0'; \
	wprintf(fmt, __dbg_win_ptr); \
	LocalFree(__dbg_win_ptr);	\
}else \
	wprintf(fmt, "(Unknown error)"); \
}


#define MIN_PATH_LEN 3

extern struct sqlite3 *sqlite_db;

typedef struct __doCloudFileTag {
		int id;
		std::wstring name;
}doCloudFileTag;

class doCloudFile {
	public:
		doCloudFile();
		~doCloudFile();
		int getFromId(int searchId);
		int getFromPath(const wchar_t *path);
		int clear();
	
		int id;
		std::wstring filename;
		int blacklisted;
		time_t updated;
		time_t uploaded;
		std::vector<doCloudFileTag*> tags;
	private:
		int getFileTags();
};


class sqliteWatcher {
	public:
		sqliteWatcher();
		~sqliteWatcher();
		
		int watch();
	private:
};

#endif /* end of include guard: SQLITE_H */
