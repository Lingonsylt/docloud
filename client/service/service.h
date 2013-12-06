#ifndef __SERVICE_H
#define __SERVICE_H

int install_service (const wchar_t *service_name, const wchar_t *display_name, 
		const wchar_t *service_path, const wchar_t *username, const wchar_t *password);
int uninstall_service(const wchar_t *service_name);
int service_start(const wchar_t *service_name, int argc, char *argv[]);
void kill_service(int error_code);

#endif
