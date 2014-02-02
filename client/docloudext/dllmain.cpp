
#include "docloudext.h"
#include "shellext.h"
#include "reg.h"
#include "common.h"


// {BFD98515-CD74-48A4-98E2-13D209E3EE4F}
// When you write your own handler, you must create a new CLSID by using the 
// "Create GUID" tool in the Tools menu, and specify the CLSID value here.
	
// {DF86C2D1-7C49-4F26-BEB9-4205987039E2}
const CLSID CLSID_doCloudExt = 
{ 0xdf86c2d1, 0x7c49, 0x4f26, { 0xbe, 0xb9, 0x42, 0x5, 0x98, 0x70, 0x39, 0xe2 } };

//{ 0xBFD98515, 0xCD74, 0x48A4, { 0x98, 0xE2, 0x13, 0xD2, 0x09, 0xE3, 0xEE, 0x4F } };
//
const char *installContextHandlers[] = {
	"*",
	"Folder",
	"Directory",
	NULL
};


HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;

#ifdef __cplusplus
extern "C" {
#endif

#define DLLEXPORT __declspec(dllexport)
#define STDAPI_EXPORTED EXTERN_C DLLEXPORT HRESULT STDAPICALLTYPE

//EXTERN_C DLLEXPORT BOOL STDAPICALLTYPE DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved);
STDAPI_EXPORTED DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv);
STDAPI_EXPORTED DllCanUnloadNow(void);
STDAPI_EXPORTED DllRegisterServer(void);
STDAPI_EXPORTED DllUnregisterServer(void);
#ifdef __cplusplus
}
#endif



HRESULT
DllRegisterServerCPP(void)
{
	HRESULT hr;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Register the component.
	hr = RegisterInprocServer(narrow(szModule).c_str(), CLSID_doCloudExt, 
	    "doCloud.Ext Class", 
	    "Apartment");
	if (SUCCEEDED(hr))
	{
		// Register the context menu handler. The context menu handler is 
		// associated with the .cpp file class.
		for (int i = 0; installContextHandlers[i] != NULL; i++)
			hr = RegisterShellExtContextMenuHandler(installContextHandlers[i], CLSID_doCloudExt, "doCloud.Ext");
	}

	hr = RegisterShellOverlayIconIdentifier(CLSID_doCloudExt, "doCloud");
	return hr;
}


HRESULT
DllUnregisterServerCPP(void)
{
	HRESULT hr = S_OK;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	// Unregister the component.
	hr = UnregisterInprocServer(CLSID_doCloudExt);
	if (SUCCEEDED(hr))
	{
		// Unregister the context menu handler.
		for (int i = 0; installContextHandlers[i] != NULL; i++)
			hr = UnregisterShellExtContextMenuHandler(installContextHandlers[i], CLSID_doCloudExt);
	}

	hr = UnregisterShellOverlayIconIdentifier("doCloud");
	return hr;
}

extern "C" {

STDAPI_EXPORTED
DllRegisterServer(void)
{
	return DllRegisterServerCPP();
}
STDAPI_EXPORTED
DllUnregisterServer(void)
{
	return DllUnregisterServerCPP();
}

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Hold the instance of this DLL module, we will use it to get the 
		// path of the DLL to register the component.
		g_hInst = hModule;
		DisableThreadLibraryCalls(hModule);
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


STDAPI_EXPORTED
DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	if (IsEqualCLSID(CLSID_doCloudExt, rclsid))
	{
		ClassFactory *pClassFactory = new ClassFactory();
		if (!pClassFactory) return E_OUTOFMEMORY;
		return pClassFactory->QueryInterface(riid, ppv);
	}

	return CLASS_E_CLASSNOTAVAILABLE;
}


STDAPI_EXPORTED
DllCanUnloadNow(void)
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}

}


/* 
ClassFactory - pass back a COM object to caller 
{{{
*/
ClassFactory::ClassFactory() :
	m_cRef(0)
{
	InterlockedIncrement((LPLONG)&m_cRef);
}

ClassFactory::~ClassFactory()
{
	InterlockedDecrement((LPLONG)&m_cRef);
}

STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    *ppv = NULL;

    // Any interface on this object is the object pointer
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef()
{
	return InterlockedIncrement((LPLONG)&m_cRef);

}

STDMETHODIMP_(ULONG) ClassFactory::Release()
{
	if(InterlockedDecrement((LPLONG)&m_cRef))
		return m_cRef;
	delete this;
	return 0L;
}

STDMETHODIMP ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
	if (pUnkOuter) return CLASS_E_NOAGGREGATION;
	
	// Create the COM component.
	ShellExt* pExt = new ShellExt();
	if (pExt == NULL) return E_OUTOFMEMORY;

	
	// Query the specified interface.
    return pExt->QueryInterface(riid, ppv);
}
    
STDMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
	return NOERROR;
}

/* }}} END CLASSFACTORY */
