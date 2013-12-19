#ifndef SQLITEWATCHER_H
#define SQLITEWATCHER_H
#include <sqlite3.h>
#include <shlwapi.h>
#include <vector>
#include <string>
#include "dirwatcher.h"

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
class sqliteWatcher {
	public:
		sqliteWatcher():dirwatcher(NULL) {};
		sqliteWatcher(dirWatcher *dw):dirwatcher(dw) { }
		~sqliteWatcher();
		
		int watch();
		int setDirWatcher(dirWatcher *dw) { dirwatcher = dw; };
	private:
		dirWatcher *dirwatcher;
};

#endif /* end of include guard: SQLITEWATCHER_H */
