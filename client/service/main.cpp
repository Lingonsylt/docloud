#include <windows.h>
#include <direct.h>
#include <getopt.h>
#include <stdio.h>
#include "dirwatcher.h"
#include "sqlitewatcher.h"
#include "service.h"

DWORD WINAPI  (__stdcall *service_exec_function)(LPVOID) = NULL;
extern int service_is_running;

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

DWORD WINAPI main_loop(LPVOID param)
{
	dirWatcher *d = new dirWatcher;
	sqliteWatcher *sqlw = new sqliteWatcher();
	unsigned long threadid;

	sqlw->watch();

	return 0;
	d->addDirectory(L"h:\\temp\\ab\\");
	d->addDirectory(L"h:\\temp");
	CreateThread(NULL, 0, thread_watch, d, 0, &threadid);
	wprintf(L"Created thread %d\n", threadid);

	CreateThread(NULL, 0, thread_work, d, 0, &threadid);
	wprintf(L"Created worker thread %d\n", threadid);

	delete d;

	return 0;
}

void
usage(char *filename)
{
	printf("docloud service\n"
	    "  Usage: %s [-iu]\n"
	    "     -i install as service\n"
	    "     -u uninstall service\n", filename);

}

int
main(int argc, char *argv[])
{
	const wchar_t *service_name = L"doCloud";
	char buf[MAX_PATH];
	int install_as_service = 0;
	int run_as_service = 0;
	int uninstall_service_flag = 0;
	int retval;
	char *ptr;

	/* change CWD to the same directory as the .exe-file */
	strncpy(buf, argv[0], sizeof(buf));
	if ((ptr=strrchr(buf,'/')) || (ptr = strrchr(buf, '\\'))) {
		*ptr = '\0';
		if (_chdir(buf) == -1){
			printf("Cannot change working directory to %s! (%s)\n",buf, strerror(errno));
		}
	}

	int ch;
	while (((ch = getopt(argc, argv, "iud:I:S1"))) != -1) {
		switch (ch) {
			case 'i': /* Install as service */
				install_as_service = 1;
			break;
			case 'u': /* Uninstall as service */
				uninstall_service_flag = 1;
			break;
			case 'S': /* Run as service */
				run_as_service = 1;
				service_exec_function = &main_loop;
				break;
			default:
				usage(argv[0]);
			break;
		}
	}

	if (install_as_service) {
		wchar_t filename[MAX_PATH];
		if (!GetModuleFileName(NULL, filename, sizeof(filename))) {
//				debug_windows("Cannot get filename of executable (%s)!\n");
		}

		std::wstring cmd;
		cmd += L"\"";
		cmd += filename;
		cmd += L"\" -S";

		retval = install_service(service_name,
			    service_name, cmd.c_str(), NULL, NULL);
		if (retval != -1) {
			wprintf(L"Service successfully installed under name %s!\n", service_name);
		}
		return 0;
	}else if (run_as_service) {
		wprintf(L"Starting service\n");
		retval = service_start(service_name, argc, argv);
	}else if (uninstall_service_flag) {
		retval = uninstall_service(service_name);
		if (retval != -1) {
			wprintf(L"Service successfully uninstalled!\n");
		}
		return 0;
	} 

	if (! run_as_service)
		retval = main_loop(NULL);
}
