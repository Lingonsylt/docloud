#include <windows.h>
#include <new>
#include <shlwapi.h>
#include <strsafe.h>
#include "reg.h"

HRESULT
RegSetKeyString(HKEY hkey, PCWSTR subkey_name, PCWSTR value_name, PCWSTR data)
{
	HRESULT hr;
	HKEY subhkey = NULL;

	// Creates the specified registry key. If the key already exists, the 
	// function opens it. 
	hr = HRESULT_FROM_WIN32(
	    RegCreateKeyEx(hkey, subkey_name, 0, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &subhkey, NULL));

	if (!SUCCEEDED(hr)) return hr;

	if (data != NULL)
	{
		// Set the specified value of the key.
		DWORD cbData = lstrlen(data) * sizeof(*data);
		hr = HRESULT_FROM_WIN32(RegSetValueEx(subhkey, value_name, 0, 
			REG_SZ, reinterpret_cast<const BYTE *>(data), cbData));
	}

	RegCloseKey(subhkey);
	return hr;
}


HRESULT
RegGetKeyString(HKEY hkey, PCWSTR subkey_name, PCWSTR value_name, PWSTR data, DWORD data_sz)
{
	HRESULT hr;
	HKEY subhkey = NULL;

	// Try to open the specified registry key. 
	hr = HRESULT_FROM_WIN32(RegOpenKeyEx(hkey, subkey_name, 0, 
		KEY_READ, &subhkey));

	if (!SUCCEEDED(hr)) return hr;

	// Get the data for the specified value name.
	hr = HRESULT_FROM_WIN32(RegQueryValueEx(subhkey, value_name, NULL, 
		NULL, reinterpret_cast<LPBYTE>(data), &data_sz));

	RegCloseKey(subhkey);
	return hr;
}


//
//   FUNCTION: RegisterInprocServer
//
//   PURPOSE: Register the in-process component in the registry.
//
//   PARAMETERS:
//   * pszModule - Path of the module that contains the component
//   * clsid - Class ID of the component
//   * pszFriendlyName - Friendly name
//   * pszThreadModel - Threading model
//
//   NOTE: The function creates the HKCR\CLSID\{<CLSID>} key in the registry.
// 
//   HKCR
//   {
//      NoRemove CLSID
//      {
//          ForceRemove {<CLSID>} = s '<Friendly Name>'
//          {
//              InprocServer32 = s '%MODULE%'
//              {
//                  val ThreadingModel = s '<Thread Model>'
//              }
//          }
//      }
//   }
//
HRESULT RegisterInprocServer(PCWSTR pszModule, const CLSID& clsid, 
    PCWSTR pszFriendlyName, PCWSTR pszThreadModel)
{
	if (pszModule == NULL || pszThreadModel == NULL)
		return E_INVALIDARG;

	HRESULT hr;

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];

	// Create the HKCR\CLSID\{<CLSID>} key.
	hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
	if (SUCCEEDED(hr)) {
		hr = RegSetKeyString(HKEY_CLASSES_ROOT, szSubkey, NULL, pszFriendlyName);
		// Create the HKCR\CLSID\{<CLSID>}\InprocServer32 key.
		if (SUCCEEDED(hr)) {
			hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
			    L"CLSID\\%s\\InprocServer32", szCLSID);
			if (SUCCEEDED(hr))
			{
				// Set the default value of the InprocServer32 key to the 
				// path of the COM module.
				hr = RegSetKeyString(HKEY_CLASSES_ROOT,szSubkey, NULL, pszModule);
				if (SUCCEEDED(hr))
				{
					// Set the threading model of the component.
					hr = RegSetKeyString(HKEY_CLASSES_ROOT,szSubkey,
					    L"ThreadingModel", pszThreadModel);
				}
			}
		}
	}
	return hr;
}


//
//   FUNCTION: UnregisterInprocServer
//
//   PURPOSE: Unegister the in-process component in the registry.
//
//   PARAMETERS:
//   * clsid - Class ID of the component
//
//   NOTE: The function deletes the HKCR\CLSID\{<CLSID>} key in the registry.
//
HRESULT UnregisterInprocServer(const CLSID& clsid)
{
	HRESULT hr = S_OK;

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];

	// Delete the HKCR\CLSID\{<CLSID>} key.
	hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
	if (SUCCEEDED(hr)) {
		hr = HRESULT_FROM_WIN32(SHDeleteKey(HKEY_CLASSES_ROOT, szSubkey));
	}

	return hr;
}


//
//   FUNCTION: RegisterShellExtContextMenuHandler
//
//   PURPOSE: Register the context menu handler.
//
//   PARAMETERS:
//   * pszFileType - The file type that the context menu handler is 
//     associated with. For example, '*' means all file types; '.txt' means 
//     all .txt files. The parameter must not be NULL.
//   * clsid - Class ID of the component
//   * pszFriendlyName - Friendly name
//
//   NOTE: The function creates the following key in the registry.
//
//   HKCR
//   {
//      NoRemove <File Type>
//      {
//          NoRemove shellex
//          {
//              NoRemove ContextMenuHandlers
//              {
//                  {<CLSID>} = s '<Friendly Name>'
//              }
//          }
//      }
//   }
//
HRESULT RegisterShellExtContextMenuHandler(
    PCWSTR pszFileType, const CLSID& clsid, PCWSTR pszFriendlyName)
{
	if (pszFileType == NULL)
		return E_INVALIDARG;

	HRESULT hr;

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];

	// If pszFileType starts with '.', try to read the default value of the 
	// HKCR\<File Type> key which contains the ProgID to which the file type 
	// is linked.
	if (*pszFileType == L'.') {
		wchar_t szDefaultVal[260];
		hr = RegGetKeyString(HKEY_CLASSES_ROOT, pszFileType, NULL, szDefaultVal,
		    sizeof(szDefaultVal));

		// If the key exists and its default value is not empty, use the 
		// ProgID as the file type.
		if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0') {
			pszFileType = szDefaultVal;
		}
		
		// Create the key HKCR\<File Type>\shellex\ContextMenuHandlers\{<CLSID>}
		hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
		    L"%s\\shellex\\ContextMenuHandlers\\%s", pszFileType, szCLSID);
		if (SUCCEEDED(hr)) {
			// Set the default value of the key.
			hr = RegSetKeyString(HKEY_CLASSES_ROOT, szSubkey, NULL, pszFriendlyName);
		}
	} else {
		// Create the key HKCR\<File Type>\shellex\ContextMenuHandlers\<Friendly name> = {<CLSID>}
		hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
		    L"%s\\shellex\\ContextMenuHandlers\\%s", pszFileType, pszFriendlyName);
		if (SUCCEEDED(hr)) {
			// Set the default value of the key.
			hr = RegSetKeyString(HKEY_CLASSES_ROOT, szSubkey, NULL, szCLSID);
		}
	}

	return hr;
}


//
//   FUNCTION: UnregisterShellExtContextMenuHandler
//
//   PURPOSE: Unregister the context menu handler.
//
//   PARAMETERS:
//   * pszFileType - The file type that the context menu handler is 
//     associated with. For example, '*' means all file types; '.txt' means 
//     all .txt files. The parameter must not be NULL.
//   * clsid - Class ID of the component
//
//   NOTE: The function removes the {<CLSID>} key under 
//   HKCR\<File Type>\shellex\ContextMenuHandlers in the registry.
//
HRESULT UnregisterShellExtContextMenuHandler(
    PCWSTR pszFileType, const CLSID& clsid)
{
	if (pszFileType == NULL)
		return E_INVALIDARG;

	HRESULT hr;

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];

	// If pszFileType starts with '.', try to read the default value of the 
	// HKCR\<File Type> key which contains the ProgID to which the file type 
	// is linked.
	if (*pszFileType == L'.') {
		wchar_t szDefaultVal[260];
		hr = RegGetKeyString(HKEY_CLASSES_ROOT, pszFileType, NULL, szDefaultVal,
		    sizeof(szDefaultVal));

		// If the key exists and its default value is not empty, use the 
		// ProgID as the file type.
		if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0') {
			pszFileType = szDefaultVal;
		}
	}

	// Remove the HKCR\<File Type>\shellex\ContextMenuHandlers\{<CLSID>} key.
	hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
	    L"%s\\shellex\\ContextMenuHandlers\\%s", pszFileType, szCLSID);
	if (SUCCEEDED(hr)) {
		hr = HRESULT_FROM_WIN32(SHDeleteKey(HKEY_CLASSES_ROOT, szSubkey));
	}

	return hr;
}


HRESULT
RegisterShellOverlayIconIdentifier(const CLSID& clsid, PCWSTR name)
{
	HRESULT hr;
	wchar_t szCLSID[MAX_PATH];
	wchar_t subkey[MAX_PATH];

	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	hr = StringCchPrintf(subkey, ARRAYSIZE(subkey),
	    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s",
	    name);
	if (!SUCCEEDED(hr)) return hr;
	/* Set default value for subkey */
	return RegSetKeyString(HKEY_LOCAL_MACHINE, subkey, NULL, szCLSID);
}

HRESULT
UnregisterShellOverlayIconIdentifier(PCWSTR name)
{
	HRESULT hr;
	wchar_t subkey[MAX_PATH];
	hr = StringCchPrintf(subkey, ARRAYSIZE(subkey),
	    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s",
	    name);
	if (!SUCCEEDED(hr)) return hr;
	return HRESULT_FROM_WIN32(SHDeleteKey(HKEY_LOCAL_MACHINE, subkey));
}
