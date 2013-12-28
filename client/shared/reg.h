#ifndef REG_H
#define REG_H
#include <windows.h>

HRESULT RegSetKeyString(HKEY hkey, const char *subkey_name, const char * value_name, const char * data);
HRESULT RegGetKeyString(HKEY hkey, const char * subkey_name, const char * value_name, char *data, DWORD data_sz);
char *RegGetKeyString(HKEY hkey, const char * subkey_name, const char * value_name);

HRESULT RegisterInprocServer(const char * pszModule, const CLSID& clsid, 
    const char * pszFriendlyName, const char * pszThreadModel);
HRESULT UnregisterInprocServer(const CLSID& clsid);

HRESULT RegisterShellExtContextMenuHandler(const char * pszFileType,
    const CLSID& clsid, const char * pszFriendlyName);
HRESULT UnregisterShellExtContextMenuHandler(const char * pszFileType, const CLSID& clsid);

HRESULT RegisterShellOverlayIconIdentifier(const CLSID& clsid, const char * name);
HRESULT UnregisterShellOverlayIconIdentifier(const char * name);

#endif /* end of include guard: REG_H */
