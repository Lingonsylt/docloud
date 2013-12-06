#include <windows.h>
#include "dirwatcher.h"

DWORD WINAPI thread_watch(LPVOID param)
{
	dirWatcher *d;
	d = (dirWatcher *)param;
	d->watch();
}

DWORD WINAPI thread_work(LPVOID param)
{
	dirWatcher *d;
	d = (dirWatcher *)param;
	d->work();
}

const wchar_t *list[] = {
	L"h:\\temp\\",
	L"h:\\temp\\ab\\abc.txt",
	NULL
};

int
main(int argc, char *argv[])
{
	dirWatcher *d = new dirWatcher;
	unsigned long threadid;

	d->addDirectory(L"h:\\temp\\ab\\");
	d->addDirectory(L"h:\\temp");
	CreateThread(NULL, 0, thread_watch, d, 0, &threadid);
	wprintf(L"Created thread %d\n", threadid);

	CreateThread(NULL, 0, thread_work, d, 0, &threadid);
	wprintf(L"Created worker thread %d\n", threadid);

	Sleep(50*1000);
}
