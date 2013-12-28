#include <windows.h>
#include <new>
#include <shlwapi.h>

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

#include "common.h"
#include "reg.h"


HRESULT
RegSetKeyString(HKEY hkey, const char * subkey_name, const char * value_name, const char * data)
{
	HRESULT hr;
	HKEY subhkey = NULL;

	// Creates the specified registry key. If the key already exists, the 
	// function opens it. 
	hr = HRESULT_FROM_WIN32(
	    RegCreateKeyEx(hkey, widen(subkey_name).c_str(), 0, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &subhkey, NULL));

	if (!SUCCEEDED(hr)) return hr;

	if (data != NULL)
	{
		// Set the specified value of the key.
		std::wstring wide = widen(data);
		const wchar_t *wide_value_name;

		if (value_name == NULL) wide_value_name = NULL;
		else wide_value_name = widen(value_name).c_str();

		DWORD cbData = lstrlen(wide.c_str()) * sizeof(*(wide.c_str()));
		wprintf(L"RegSetValueEx(subhkey, %s, 0, REG_SZ, %s, %d)\n",
		    wide_value_name, wide.c_str(), cbData);

		hr = HRESULT_FROM_WIN32(RegSetValueEx(subhkey, wide_value_name, 0, 
			REG_SZ, reinterpret_cast<const BYTE *>(wide.c_str()), cbData));
	}

	RegCloseKey(subhkey);
	return hr;
}

char *
RegGetKeyString(HKEY hkey, const char * subkey_name, const char * value_name)
{
	HRESULT hr;
	HKEY subhkey = NULL;
	DWORD sz;
	PWSTR str;

	// Try to open the specified registry key. 
	hr = HRESULT_FROM_WIN32(RegOpenKeyEx(hkey, widen(subkey_name).c_str(), 0, 
		KEY_READ, &subhkey));

	if (!SUCCEEDED(hr)) return NULL;

	// Get the data for the specified value name.
	hr = HRESULT_FROM_WIN32(RegQueryValueEx(subhkey, widen(value_name).c_str(), NULL, NULL, NULL, &sz));
	if (!SUCCEEDED(hr)) {
		return NULL;
	}

	str = new wchar_t[sz];
	hr = HRESULT_FROM_WIN32(RegQueryValueEx(subhkey, widen(value_name).c_str(), NULL, NULL,
		reinterpret_cast<LPBYTE>(str), &sz));
	if (!SUCCEEDED(hr)) {
		delete str;
		return NULL;
	}

	RegCloseKey(subhkey);

	const char *narrow_str  = narrow(str).c_str();
	sz = strlen(narrow_str) + 1;
	char *output_str = new char[sz];
	strcpy_s(output_str, sz, narrow_str);
	return output_str;
}


HRESULT
RegGetKeyString(HKEY hkey, const char * subkey_name, const char * value_name, char *data, DWORD data_sz)
{
	HRESULT hr;
	HKEY subhkey = NULL;

	// Try to open the specified registry key. 
	hr = HRESULT_FROM_WIN32(RegOpenKeyEx(hkey, widen(subkey_name).c_str(), 0, 
		KEY_READ, &subhkey));

	if (!SUCCEEDED(hr)) return hr;

	// Get the data for the specified value name.
	hr = HRESULT_FROM_WIN32(RegQueryValueEx(subhkey, widen(value_name).c_str(), NULL, 
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
HRESULT RegisterInprocServer(const char * pszModule, const CLSID& clsid, 
    const char * pszFriendlyName, const char * pszThreadModel)
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
		hr = RegSetKeyString(HKEY_CLASSES_ROOT, narrow(szSubkey).c_str(), NULL, pszFriendlyName);
		// Create the HKCR\CLSID\{<CLSID>}\InprocServer32 key.
		if (SUCCEEDED(hr)) {
			hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
			    L"CLSID\\%s\\InprocServer32", szCLSID);
			if (SUCCEEDED(hr))
			{
				// Set the default value of the InprocServer32 key to the 
				// path of the COM module.
				hr = RegSetKeyString(HKEY_CLASSES_ROOT, narrow(szSubkey).c_str(), NULL, pszModule);
				if (SUCCEEDED(hr))
				{
					// Set the threading model of the component.
					hr = RegSetKeyString(HKEY_CLASSES_ROOT, narrow(szSubkey).c_str(),
					    "ThreadingModel", pszThreadModel);
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
    const char * pszFileType, const CLSID& clsid, const char * pszFriendlyName)
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
		char *defaultVal;
		defaultVal =  RegGetKeyString(HKEY_CLASSES_ROOT, pszFileType, NULL);

		// If the key exists and its default value is not empty, use the 
		// ProgID as the file type.
		if (defaultVal != NULL && defaultVal[0] != L'\0') {
			pszFileType = defaultVal;
		}
		
		// Create the key HKCR\<File Type>\shellex\ContextMenuHandlers\{<CLSID>}
		hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
		    L"%S\\shellex\\ContextMenuHandlers\\%s", pszFileType, szCLSID);
		if (defaultVal) delete defaultVal;

		if (SUCCEEDED(hr)) {
			// Set the default value of the key.
			hr = RegSetKeyString(HKEY_CLASSES_ROOT, narrow(szSubkey).c_str(), NULL, pszFriendlyName);
		}
	} else {
		// Create the key HKCR\<File Type>\shellex\ContextMenuHandlers\<Friendly name> = {<CLSID>}
		hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
		    L"%S\\shellex\\ContextMenuHandlers\\%S", pszFileType, pszFriendlyName);
		if (SUCCEEDED(hr)) {
			// Set the default value of the key.
			hr = RegSetKeyString(HKEY_CLASSES_ROOT, narrow(szSubkey).c_str(), NULL, narrow(szCLSID).c_str());
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
    const char * pszFileType, const CLSID& clsid)
{
	if (pszFileType == NULL)
		return E_INVALIDARG;

	HRESULT hr;

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];
	char *defaultVal = NULL;

	// If pszFileType starts with '.', try to read the default value of the 
	// HKCR\<File Type> key which contains the ProgID to which the file type 
	// is linked.
	if (*pszFileType == '.') {

		defaultVal = RegGetKeyString(HKEY_CLASSES_ROOT, pszFileType, NULL);

		// If the key exists and its default value is not empty, use the 
		// ProgID as the file type.
		if (defaultVal != NULL && defaultVal[0] != '\0') {
			pszFileType = defaultVal;
		}
	}

	// Remove the HKCR\<File Type>\shellex\ContextMenuHandlers\{<CLSID>} key.
	hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), 
	    L"%S\\shellex\\ContextMenuHandlers\\%s", pszFileType, szCLSID);
	if (defaultVal) delete defaultVal;

	if (SUCCEEDED(hr)) {
		hr = HRESULT_FROM_WIN32(SHDeleteKey(HKEY_CLASSES_ROOT, szSubkey));
	}

	return hr;
}


HRESULT
RegisterShellOverlayIconIdentifier(const CLSID& clsid, const char * name)
{
	wchar_t szCLSID[MAX_PATH];
	char subkey[MAX_PATH];
	int ret;

	StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

	ret = snprintf(subkey, sizeof(subkey),
	    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s",
	    name);
	if (ret <= 0) return 0;
	/* Set default value for subkey */
	return RegSetKeyString(HKEY_LOCAL_MACHINE, subkey, NULL, narrow(szCLSID).c_str());
}

HRESULT
UnregisterShellOverlayIconIdentifier(const char * name)
{
	HRESULT hr;
	wchar_t subkey[MAX_PATH];
	hr = StringCchPrintf(subkey, ARRAYSIZE(subkey),
	    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%S",
	    name);
	if (!SUCCEEDED(hr)) return hr;
	return HRESULT_FROM_WIN32(SHDeleteKey(HKEY_LOCAL_MACHINE, subkey));
}
