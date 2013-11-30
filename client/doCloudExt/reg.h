#ifndef REG_H
#define REG_H
#include <windows.h>

HRESULT RegisterInprocServer(PCWSTR pszModule, const CLSID& clsid, 
    PCWSTR pszFriendlyName, PCWSTR pszThreadModel);
HRESULT UnregisterInprocServer(const CLSID& clsid);

HRESULT RegisterShellExtContextMenuHandler(PCWSTR pszFileType,
    const CLSID& clsid, PCWSTR pszFriendlyName);
HRESULT UnregisterShellExtContextMenuHandler(PCWSTR pszFileType, const CLSID& clsid);

HRESULT RegisterShellOverlayIconIdentifier(const CLSID& clsid, PCWSTR name);
HRESULT UnregisterShellOverlayIconIdentifier(PCWSTR name);

#endif /* end of include guard: REG_H */
