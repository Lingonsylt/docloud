#include <windows.h>
#include "docloudfile.h"
#include "debug.h"

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

#define NO_SHLWAPI_STRFCNS
#include <shlwapi.h>
#include "shellext.h"
#include "resource.h"
#include "common.h"
#include <stdio.h>

extern HINSTANCE g_hInst;
extern long g_cDllRef;
FORMATETC fmte = {CF_HDROP, (DVTARGETDEVICE FAR *)NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

#define IDCMD_ADD		0
#define IDCMD_REMOVE		1

/* Do not check files that do not have a complete path,
 * e.g. c:\a is ok, but not c:
 */
#define MIN_PATH_LEN		4

ShellExt::ShellExt(void) : m_cRef(1), 
	dataObj(NULL),
	moduleFilename(NULL)
{
	wchar_t wide_filename[MAX_PATH];
	InterlockedIncrement(&g_cDllRef);
	HRESULT ret;

	// Load the bitmap for the menu item. 
	// If you want the menu item bitmap to be transparent, the color depth of 
	// the bitmap must not be greater than 8bpp.
	//m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), 
	//   IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);

	ret = GetModuleFileName(g_hInst, wide_filename, ARRAYSIZE(wide_filename));

	/* Ignore errors and keep moduleFilename as NULL -
	 * we'll lose our overlayicons, but everything else works
	 */
	if (ret > 0 && ret < MAX_PATH)
	{
		moduleFilename = new wchar_t[ret+1];
		StringCchCopy(moduleFilename, ret+1, wide_filename);
	}
}

ShellExt::~ShellExt(void)
{
	if (m_hMenuBmp)
	{
		DeleteObject(m_hMenuBmp);
		m_hMenuBmp = NULL;
	}

	if (moduleFilename)
		delete moduleFilename;

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
	const char *text = "";
	int ret;

	// If uFlags include CMF_DEFAULTONLY then we should not do anything.
	if (CMF_DEFAULTONLY & uFlags)
	{
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
	}

	hres = dataObj->GetData(&fmte, &medium);

	nFiles = 0;
	if (medium.hGlobal)
		nFiles = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);

	idCmd = idCmdFirst;

	/* Build list of files, with info */
	int found = 0;
	int blacklisted = 0;
	int show_flag = 0;

	std::vector<doCloudFile *>::iterator it;
	for (it = v_files.begin(); it != v_files.end(); it ++)
		delete (*it);

	v_files.clear();
	for (int i = 0; i < nFiles; i++) {
		doCloudFile *dcfile;
		wchar_t wide_filename[300];
		std::string filename;

		ret = DragQueryFile((HDROP)medium.hGlobal, i, NULL, 0);
		log("strlen: %d\n", ret);
		ret = DragQueryFile((HDROP)medium.hGlobal, i, wide_filename, ARRAYSIZE(wide_filename));
		filename = narrow(wide_filename);

		if (!ret) {
			log("Could not get file for index %d\n", i);
			continue;
		}

		dcfile = new doCloudFile;
		if ((ret = dcfile->getFromPath(filename.c_str())) == -1) {
			log("doCloudFile.getFromPath(%s): %d\n", filename.c_str(), ret);

			delete dcfile;
			continue;
		}

		if (dcfile->id == -1) {
			/* We're not watching this, so check what filetype it is */
			if (docloud_is_correct_filetype(filename.c_str()) ||
			    PathIsDirectory(wide_filename) != FALSE) {
				show_flag = 1;
			}
			delete dcfile;
			continue;
		}

		/* We're watching this file, so add it */
		show_flag = 1;
		if (dcfile->filename.length()) {
			log("Found file [%ld] %s\n", dcfile->id, dcfile->filename.c_str());
		} else {
			log("Could not find file for path %s\n", filename.c_str());
		}
		log("Blacklisted: %ld\n", dcfile->blacklisted);

		if (dcfile->id != -1) found++;
		if (dcfile->blacklisted != 0) blacklisted ++;

		/* Add a pointer to the acutal file/folder the user clicked on */
		if (dcfile->matches_parent || dcfile->id == -1) {
			dcfile->clear();
			dcfile->filename = filename.c_str();
		}
		v_files.push_back(dcfile);
	}

	if (!show_flag)
		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(idCmd - idCmdFirst));

	if (found && !blacklisted) {
		if (nFiles > 1) text = "Remove files from doCloud";
		else text = "Remove file from doCloud";
		idCmd += IDCMD_REMOVE;
	} else {
		if (nFiles > 1) text = "Add files to doCloud";
		else text = "Add file to doCloud";
		idCmd += IDCMD_ADD;
	}

	InsertMenu(hMenu, indexMenu++, MF_STRING|MF_BYPOSITION, idCmd++, widen(text).c_str());
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
	int ret;

	/* We're only interested in user actions -
	 * and if HIWORD(pici->lpVerb) > 0, we've been called programmatically
	 */
	if (HIWORD(pici->lpVerb))
		return E_INVALIDARG;

	UINT cmd;
	cmd = LOWORD(pici->lpVerb);

	log("inovkeCommand()\n", 1);
	std::vector<doCloudFile*>::iterator it;
	for (it = v_files.begin(); it != v_files.end(); it ++) {
		doCloudFile *dcfile = (*it);

		if (dcfile->filename.length() == 0)
			continue;
		if (cmd == IDCMD_ADD) {
			dcfile->blacklisted = 0;
			log("add file %s\n", dcfile->filename.c_str());
		} else if (cmd == IDCMD_REMOVE) {
			dcfile->blacklisted = 1;
			log("blacklist file %s\n", (*it)->filename.c_str());
		}
		dcfile->save();
	}
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
	*pdwFlags = 0;
	if (moduleFilename) {
		StringCchCopy(reinterpret_cast<PWSTR>(pwszIconFile), cchMax, moduleFilename);
		*pIndex = 0;
		*pdwFlags = ISIOI_ICONFILE;
	}
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
	doCloudFile file;
	std::string filename = narrow(pwszPath);
	int ret;

	if (!docloud_is_correct_filetype(filename.c_str())) {
		return S_FALSE;
	}

	file.getFromPath(filename.c_str());

	if (file.id != -1) {
		if (file.blacklisted)
			return S_FALSE;
		return S_OK;
	}
	return S_FALSE;
}

/* }}} END IShellIconOverlayIdentifier Interface */
