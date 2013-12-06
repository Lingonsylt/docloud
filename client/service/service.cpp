#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <errno.h>
#include <string>
#include "service.h"

static SERVICE_STATUS ssStatus;

#define DEBUG_ERR 0
#define DEBUG_INFO 1
#define debug(level, fmt, ...) { \
		wprintf(fmt, __VA_ARGS__); \
	}
#define debug_windows(level, fmt) {\
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
SERVICE_STATUS_HANDLE hServiceStatus;
static HANDLE killServiceEvent;
static HANDLE hthread;
static const wchar_t *glb_service_name;
extern DWORD WINAPI  (__stdcall *service_exec_function)(LPVOID);
int service_is_running = 0;

/* Internal functions */
static void WINAPI service_callback(DWORD dwCtrlCode);
static int service_report_status(int currentstate, int exitcode, int service_exitcode, int checkpoint, int waithint);
static int start_service_thread();
static void WINAPI service_start_main_func(int argc, char *argv[]);

/* Exported functions */
int install_service (const wchar_t *service_name, const wchar_t *display_name, 
		const wchar_t *service_path, const wchar_t *username, const wchar_t *password);
int uninstall_service(const wchar_t *service_name);
int service_start(const wchar_t *service_name, int argc, char *argv[]);
void kill_service(int error_code);
  
int install_service (const wchar_t *service_name, const wchar_t *display_name, 
		const wchar_t *service_path, const wchar_t *username, const wchar_t *password)
{
	SC_HANDLE schSCManager;
	SC_HANDLE service;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (schSCManager == NULL) {
		debug_windows(DEBUG_ERR, L"Cannot open service control manager! (%s)\n");
		return -1;
	}

	/* Create a service using default settings */
	service = CreateService (schSCManager, 
			service_name, 
			display_name,
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_DEMAND_START,
			SERVICE_ERROR_NORMAL,
			service_path,
			NULL,
			NULL,
			NULL,
			username,
			password);

	if (service == NULL) {
		debug_windows(DEBUG_ERR, L"Cannot create service (%s)!\n");
		return -1;
	}

	/* Close all servicehandles, since we wont need them anymore */
	CloseServiceHandle(service);
	CloseServiceHandle(schSCManager);

	return 0;
}

static void WINAPI service_callback(DWORD dwCtrlCode)
{

	if ((dwCtrlCode == SERVICE_CONTROL_STOP)  ||
	     (dwCtrlCode == SERVICE_CONTROL_SHUTDOWN)) {
		debug(DEBUG_INFO, L"Service recived stop-command\n", "");
		ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
		service_report_status(SERVICE_STOP_PENDING, NO_ERROR, 0, 1, 3000);
		kill_service(0);
		return;
	}
 	service_report_status(ssStatus.dwCurrentState, NO_ERROR, 0, 0, 0);
}

void kill_service(int error_code)
{
	service_is_running = 0;
	SetEvent(killServiceEvent);
	service_report_status(SERVICE_STOPPED, NO_ERROR, error_code, 0, 0);
}

static int start_service_thread()
{
	unsigned id = 0;
	unsigned long threadid;

	service_is_running = 1;
	hthread = CreateThread(NULL, 0, service_exec_function, NULL, 0, &threadid);
	if (!hthread) {
		debug_windows(DEBUG_ERR, L"Cannot create thread! (%s)\n");
		kill_service(-1);
		return -1;
	}

	debug(DEBUG_INFO, L"Created thread with id %d\n", id);
	return 0;
}

static void WINAPI service_start_main_func(int argc, char *argv[])
{

	memset(&ssStatus, 0, sizeof(SERVICE_STATUS));
	ssStatus.dwCurrentState = SERVICE_START_PENDING;
	ssStatus.dwCheckPoint = 1;

	if ((hServiceStatus = RegisterServiceCtrlHandler(glb_service_name, &service_callback)) == 0) {
		debug_windows(DEBUG_ERR, L"Cannot register service handler (%s)!\n");
		return;	
	}

	if (!service_report_status(SERVICE_START_PENDING, NO_ERROR, 0, 1, 3000)) 
		return;

	if ((killServiceEvent = CreateEvent(0,TRUE,FALSE,0)) == NULL) {
		debug_windows(DEBUG_ERR, L"Cannot create killServiceEvent (%s)!\n");
		return;
	}

	if (!service_report_status(SERVICE_START_PENDING, NO_ERROR, 0, 2, 1000)) 
		return;

	if (start_service_thread() == -1) {
		kill_service(-1);
		return;
	}

	ssStatus.dwCurrentState = SERVICE_RUNNING;
	if (! service_report_status(SERVICE_RUNNING, NO_ERROR, 0, 0, 0)) 
		return;

	WaitForSingleObject(killServiceEvent, INFINITE);
	CloseHandle(killServiceEvent);
	CloseHandle(hthread); /* Kill off thread handle aswell */
}

int service_start(const wchar_t *service_name, int argc, char *argv[])
{
	wchar_t *service_name_cpy;

	service_name_cpy = _wcsdup(service_name);
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ service_name_cpy, (LPSERVICE_MAIN_FUNCTION) service_start_main_func },
		{ NULL, NULL }
	};

	glb_service_name = service_name_cpy;

	if (!StartServiceCtrlDispatcher (dispatchTable)) {
		debug_windows(DEBUG_ERR, L"Cannot start service control dispatcher (%s)!\n");
		return -1;
	}

	return 0;
}

static int service_report_status(int currentstate, int exitcode, int service_exitcode, int checkpoint, int waithint)
{
	int retval;

	ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

	if (currentstate == SERVICE_START_PENDING)
		ssStatus.dwControlsAccepted = 0;
	else
		ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	if (service_exitcode == 0) 
		ssStatus.dwWin32ExitCode = exitcode;
	else
		ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;

	ssStatus.dwServiceSpecificExitCode = service_exitcode;
	ssStatus.dwCheckPoint = checkpoint;
	ssStatus.dwWaitHint = waithint;
	ssStatus.dwCurrentState = currentstate;

	retval = SetServiceStatus(hServiceStatus, &ssStatus);
	if (retval == -1) {
		debug_windows(DEBUG_ERR, L"SetServiceStatus failed with error '%s'!\n");
		kill_service(-1);
	}
	return retval;
}

int uninstall_service(const wchar_t *service_name)
{
	SC_HANDLE schSCManager;
	SC_HANDLE service;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == NULL) {
		debug_windows(DEBUG_ERR, L"Cannot open service control manager (%s)!\n")
		return -1;
	}

	/* Open service with DELETE access */
	service = OpenService(schSCManager, service_name, DELETE);
	if (service == NULL) {
		debug_windows(DEBUG_ERR, L"Cannot open service (%s)!\n");
		return -1;
	}

	if (!DeleteService(service)) {
		debug_windows(DEBUG_ERR, L"Cannot delete service (%s)!\n");
		return -1;
	}

	CloseServiceHandle(service);
	CloseServiceHandle(schSCManager);

	return 0;
}
