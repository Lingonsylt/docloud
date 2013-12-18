#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <map>
#include <string>
#include "dirwatcher.h"

/* strsafe.h-wrapper for mingw, in order to suppress
 * warnings due to lack of strsafe.lib
 */
#ifdef __MINGW32__
#ifdef __CRT__NO_INLINE
#undef __CRT__NO_INLINE
#define DID_UNDEFINE__CRT__NO_INLINE
#endif
extern "C" {
#endif
#include <strsafe.h>
#ifdef __MINGW32__
}
#ifdef DID_UNDEFINE__CRT__NO_INLINE
#define __CRT__NO_INLINE
#endif
#endif

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

#define BUFFER_SIZE 5

dirWatcher::dirWatcher(): hIOCP(NULL), max_key(0)
{
	/* used to send information to workers */
	updatePort = CreateIoCompletionPort((HANDLE)INVALID_HANDLE_VALUE,
		NULL, 0, 0);
	if (!updatePort) {
		debug_windows(L"CreateIoCompletionPort(updatePort): %s\n");
	}
}

dirWatcher::~dirWatcher()
{
	if (hIOCP != NULL)
		CloseHandle(hIOCP);
	if (updatePort != NULL)
		CloseHandle(updatePort);
}

int
dirWatcher::init() {
	unsigned long nb;

	hIOCP = CreateIoCompletionPort((HANDLE)INVALID_HANDLE_VALUE,
	    NULL, 0, 1);

	if (!hIOCP) {
		debug_windows(L"CreateIoCompletionPort: %s\n");
		return -1;
	}

	InitializeSRWLock(&dirLock);

	return 0;
}

int dirWatcher::addDirectory(const wchar_t *path)
{
	HANDLE h;
	directory *dir;
	std::wstring wstr;

	if (hIOCP == NULL)
		this->init();


	/* First, check if we're watching a subdirectory to this directory */
	wprintf(L"Tryint to add path %s\n", path);
	wstr = path;
	AcquireSRWLockShared(&dirLock);
	std::map<unsigned long long, directory *>::iterator m;
	for (m = dirs.begin(); m != dirs.end(); m++) {
		if (wstr.find(m->second->path,0) == 0) {
			/* We're already watching either this directory or parent */
			wprintf(L"Already watching parent %s, skipping %s\n", m->second->path.c_str(), path);
			ReleaseSRWLockShared(&dirLock);
			return 0;
		} else if(m->second->path.find(wstr, 0) == 0) {
			/* We're the parent of this directory! */
			wprintf(L"We're (%s) parent of %s, removing it\n", path, m->second->path.c_str());
			ReleaseSRWLockShared(&dirLock);

			AcquireSRWLockExclusive(&dirLock);
			this->dirs.erase(m);
			ReleaseSRWLockExclusive(&dirLock);
			AcquireSRWLockShared(&dirLock);

			if (dirs.empty()) break;

			/* we've messed up the iterator */
			m = dirs.begin();
		}
	}
	ReleaseSRWLockShared(&dirLock);

	h = CreateFile(path,
	    FILE_LIST_DIRECTORY,
	    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	    NULL,
	    OPEN_EXISTING,
	    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
	    NULL);

	if (h == INVALID_HANDLE_VALUE) {
		debug_windows(L"CreateFile: %s\n");
		return 0;
	}

	dir = new directory;
	dir->handle = h;
	dir->path = path;
	dir->buffer = new FILE_NOTIFY_INFORMATION[BUFFER_SIZE];
	dir->key = max_key++;

	/* Clear overlapped structure */
	memset(&(dir->overlapped), 0, sizeof(dir->overlapped));

	if (CreateIoCompletionPort(dir->handle, hIOCP, (ULONG_PTR)dir->key, 0) == NULL) {
		debug_windows(L"CreateIoCompletionPort: %s\n");
		delete dir;
		return -1;
	}

	unsigned long nb;
	if (!ReadDirectoryChangesW(dir->handle, dir->buffer,
		sizeof(FILE_NOTIFY_INFORMATION) * BUFFER_SIZE,
		TRUE,
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, 
		&nb,
		&dir->overlapped,
		NULL))
	{
		debug_windows(L"ReadDirectoryChangesW():%s\n");
		delete dir;
		return -1;
	}
	wprintf(L"Added path %ls to watchlist successfully\n", dir->path.c_str());

	AcquireSRWLockExclusive(&dirLock);
	this->dirs[dir->key] = dir;
	ReleaseSRWLockExclusive(&dirLock);
	return 0;
}

int dirWatcher::remDirectory(const wchar_t *path)
{
	std::map<unsigned long long, directory *>::iterator it;

	AcquireSRWLockExclusive(&dirLock);
	for (it = dirs.begin(); it != dirs.end(); it++) {
		if (it->second->path == path) {
			this->dirs.erase(it);
			break;
		}
	}
	ReleaseSRWLockExclusive(&dirLock);
	return 0;
}

int 
dirWatcher::watch()
{
	unsigned long nb;
	unsigned long long key;
	OVERLAPPED *overlappedptr;
	directory *dir;

	for (;;) {
		if (!GetQueuedCompletionStatus(hIOCP, &nb, &key, &overlappedptr, INFINITE)) {
			debug_windows(L"GetQueuedCompletionStatus(): %s\n");
		}

		std::map<unsigned long long, directory *>::iterator it;

		AcquireSRWLockShared(&dirLock);
		it = this->dirs.find(key);
		if (it == this->dirs.end()) {
			/* We're not watching this anymore.. */
			if (this->dirs.empty())
				return 0;
			continue;
		}
		dir = it->second;

		FILE_NOTIFY_INFORMATION* pIter = dir->buffer;

		while (pIter)
		{
			pIter->FileName[pIter->FileNameLength / sizeof(TCHAR)] = 0;

			/* Tell the workers we've found something */
			std::wstring *wstr = new std::wstring(dir->path);
			wstr->append(L"\\");
			wstr->append(pIter->FileName);
			PostQueuedCompletionStatus(updatePort, pIter->Action, (ULONG_PTR)wstr, NULL);

			if(pIter->NextEntryOffset == 0UL)
				break;	

			if ((DWORD)((BYTE*)pIter - (BYTE*)dir->buffer) > (BUFFER_SIZE * sizeof(FILE_NOTIFY_INFORMATION)))
				pIter = dir->buffer;

			pIter = (PFILE_NOTIFY_INFORMATION) ((LPBYTE)pIter + pIter->NextEntryOffset);
		}

		// Add directory again
		if (!ReadDirectoryChangesW(dir->handle, dir->buffer,
			sizeof(FILE_NOTIFY_INFORMATION) * BUFFER_SIZE,
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, 
			&nb,
			&dir->overlapped,
			NULL))
		{
			debug_windows(L"ReadDirectoryChangesW():%s\n");
			ReleaseSRWLockShared(&dirLock);

			AcquireSRWLockExclusive(&dirLock);
			this->dirs.erase(dir->key);
			ReleaseSRWLockExclusive(&dirLock);
			continue;
		}
		ReleaseSRWLockShared(&dirLock);

		wprintf(L"Added path %s to watchlist successfully\n", dir->path.c_str());
	}
	return 0;
}

int
dirWatcher::work()
{
	DWORD action;
	std::wstring *path;
	ULONG_PTR ptr;
	OVERLAPPED *overlapped;

	for (;;) {
		HRESULT ret = GetQueuedCompletionStatus(updatePort, &action, &ptr, &overlapped, INFINITE);
		if (ret == false) {
			debug_windows(L"GetQueuedCompletionStatus():%s\n");
			if (overlapped == NULL)
				wprintf(L"Did not dequeue packet!\n");
			continue;
		}
		path = (std::wstring*)ptr;

		if (path == NULL) continue;

		wprintf(L"[worker] Updated path %s (%d)\n", path->c_str(), action);
	}
}
