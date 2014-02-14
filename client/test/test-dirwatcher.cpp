#include <windows.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <direct.h>
#include "dirwatcher.h"
#include "common.h"
#include "test.h"

std::string docloud_temp_filename;
int callback_called = 0;
int callback_correct_path = 0;

int
callback_function(const char *path)
{
	callback_called = 1;
	if (docloud_temp_filename == path)
		callback_correct_path = 1;
	return 0;
}

DWORD WINAPI thread_dirwatch(LPVOID param)
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


int main(int argc, const char *argv[])
{
	dirWatcher *dw;
	wchar_t temp_path[MAX_PATH+1];
	unsigned long threadid;
	char *docloud_temp_path;
	FILE *fd;

	dw = new dirWatcher;

#if defined(_WIN32)
	GetTempPath(MAX_PATH, temp_path);
	std::string tmpstr = narrow(temp_path);
	docloud_temp_path = _tempnam(tmpstr.c_str(), "docloud");
	_mkdir(docloud_temp_path);
#else
	mkdir(strPath.c_str(), 777);
#endif

	TEST("dirWatcher->init()", (dw->init() == 0));
	dw->setCallback(callback_function);

	TEST("dirWatcher->addDirectory()", (dw->addDirectory(docloud_temp_path) == 0));

	CreateThread(NULL, 0, thread_dirwatch, dw, 0, &threadid);
	printf("Created thread for dirwatcher %d\n", threadid);

	CreateThread(NULL, 0, thread_work, dw, 0, &threadid);
	printf("Created worker thread %d\n", threadid);

	Sleep(100);
	/* Create file in our temporary directory */
	docloud_temp_filename = docloud_temp_path;
	docloud_temp_filename += "\\xyz_temp_filename.docx";
	printf("temp filename: %s\n", docloud_temp_filename.c_str());
	fd = fopen(docloud_temp_filename.c_str(), "w");
	if (fd == NULL) {
		TEST_ERR("create tempfile");
		return -1;
	}
	fprintf(fd, "abc");
	fclose(fd);
	Sleep(100);

	TEST("dirWatcher->work(): callback", callback_called);
	TEST("dirWatcher->work(): correct path", callback_correct_path);

	unlink(docloud_temp_filename.c_str());
	unlink(docloud_temp_path);

	return 0;
}
