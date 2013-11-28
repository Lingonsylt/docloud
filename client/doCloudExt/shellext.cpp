#include <windows.h>
#include <strsafe.h>
#define NO_SHLWAPI_STRFCNS
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "shellext.h"
#include "resource.h"

extern HINSTANCE g_hInst;
extern long g_cDllRef;
FORMATETC fmte = {CF_HDROP, (DVTARGETDEVICE FAR *)NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

#define IDM_DISPLAY             0  // The command's identifier offset

ShellExt::ShellExt(void) : m_cRef(1), 
	dataObj(NULL)
{
	InterlockedIncrement(&g_cDllRef);

	// Load the bitmap for the menu item. 
	// If you want the menu item bitmap to be transparent, the color depth of 
	// the bitmap must not be greater than 8bpp.
	m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), 
	    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
}

ShellExt::~ShellExt(void)
{
	if (m_hMenuBmp)
	{
		DeleteObject(m_hMenuBmp);
		m_hMenuBmp = NULL;
	}

	InterlockedDecrement(&g_cDllRef);
}



/* IUnknown Interface {{{ */

// Query to the interface the component supported.
STDMETHODIMP ShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

	if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
		*ppv = (LPSHELLEXTINIT)this;
	else if (IsEqualIID(riid, IID_IContextMenu))
		*ppv = (LPCONTEXTMENU)this;
	else if (IsEqualIID(riid, IID_IShellIconOverlayIdentifier))
		*ppv = (IShellIconOverlayIdentifier*)this;

	if (*ppv) {
		AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}

// Increase the reference count for an interface on an object.
STDMETHODIMP_(ULONG) ShellExt::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
STDMETHODIMP_(ULONG) ShellExt::Release()
{
	if (InterlockedDecrement(&m_cRef))
		return m_cRef;

	delete this;
	return 0L;
}

/* }}} END IUnknown Interface */
/* IShellExtInit Interface {{{ */

// Initialize the context menu handler.
STDMETHODIMP
ShellExt::Initialize(LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{

	if (dataObj != NULL)
		dataObj->Release();

	if (pDataObj) {
		dataObj = pDataObj;
		pDataObj->AddRef();
	}

	return S_OK;
}

/* }}} END IShellExtInit Interface */
/* IContextMenu Interface {{{ */

/*
 * QueryContextMenu()
 * Add items to contextmenu for matching files
 */
STDMETHODIMP
ShellExt::QueryContextMenu(HMENU hMenu, UINT indexMenu,
    UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	HRESULT hres;
	UINT idCmd;
	int cbFiles;
	wchar_t *text;

	// If uFlags include CMF_DEFAULTONLY then we should not do anything.
	if (CMF_DEFAULTONLY & uFlags)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	hres = dataObj->GetData(&fmte, &medium);
	if (medium.hGlobal)
		cbFiles = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);

	idCmd = idCmdFirst;

	if (cbFiles > 1) text = L"Add files to doCloud";
	else text = L"Add file to doCloud";

	InsertMenu(hMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, text);
	InsertMenu(hMenu, indexMenu++, MF_SEPARATOR|MF_BYPOSITION, 0, NULL);

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(idCmd - idCmdFirst));
	/*
	 * Menuitem with bitmap:
	 *
	 
	   MENUITEMINFO mii = { sizeof(mii) };
	   mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
	   mii.wID = idCmdFirst + IDM_DISPLAY;
	   mii.fType = MFT_STRING;

	   if (nFiles == 1)
	   mii.dwTypeData = m_pszMenuText;
	   else
	   mii.dwTypeData = L"&Multiple files!!!";
	   mii.fState = MFS_ENABLED;
	   mii.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
	   if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
	   {
	   return HRESULT_FROM_WIN32(GetLastError());
	   }

*/
}


STDMETHODIMP ShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
	BOOL fUnicode = FALSE;

	/* We're only interested in user actions -
	 * and if HIWORD(pici->lpVerb) > 0, we've been called programmatically
	 */
	if (HIWORD(pici->lpVerb))
		return E_INVALIDARG;

	UINT cmd;
	cmd = LOWORD(pici->lpVerb);

	OnVerbDisplayFileName(pici->hwnd);
	return S_OK;

}

STDMETHODIMP ShellExt::GetCommandString(UINT_PTR idCommand, 
    UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
	HRESULT hr = S_OK;
	if (uFlags == GCS_HELPTEXTW && cchMax > 35)
		hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, L"Add file to doCloud");

	return S_OK;
}

/* }}} END IContextMenu Interface */
/* IShellIconOverlayIdentifier Interface {{{ */

STDMETHODIMP
ShellExt::GetOverlayInfo(PWSTR pwszIconFile, int cchMax,int *pIndex,DWORD *pdwFlags)
{
	StringCchCopy(reinterpret_cast<PWSTR>(pwszIconFile), cchMax, L"c:\\devel\\out.ico");
	*pIndex = 0;
	*pdwFlags = ISIOI_ICONFILE;
	return S_OK;
}

STDMETHODIMP
ShellExt::GetPriority(int *priority)
{
	*priority = 50;
	return S_OK;
}

STDMETHODIMP
ShellExt::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
	return S_OK;
}

/* }}} END IShellIconOverlayIdentifier Interface */

void ShellExt::OnVerbDisplayFileName(HWND hWnd)
{
	wchar_t szMessage[300];
	int nFiles;

	if (medium.hGlobal)
		nFiles = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);

	StringCchPrintf(szMessage, ARRAYSIZE(szMessage), 
		    L"Your selected file(s):\r\n\r\n");

	for (int i = 0; i < nFiles; i++) {
		wchar_t filename[300];
		DragQueryFile((HDROP)medium.hGlobal, i, filename, ARRAYSIZE(filename));
		StringCchCat(szMessage, ARRAYSIZE(szMessage), filename);
		StringCchCat(szMessage, ARRAYSIZE(szMessage), L"\r\n");

	}
	MessageBox(hWnd, szMessage, L"doCloud", MB_OK);
}
