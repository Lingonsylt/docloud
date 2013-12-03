#ifndef SHELLEXT_H
#define SHELLEXT_H
#include <vector>
#include <windows.h>
#include <shlobj.h>     // For IShellExtInit and IContextMenu and IShellIconOverlayIdentifier

class ShellExt : public IShellExtInit, public IContextMenu, public IShellIconOverlayIdentifier
{
	public:
		// IUnknown
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();

		// IShellExtInit
		STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

		// IContextMenu
		STDMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
		STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
		STDMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);

		// IShellIconOverlayIdentifier
		STDMETHODIMP GetOverlayInfo(PWSTR pwszIconFile, int cchMax,int *pIndex,DWORD *pdwFlags);
		STDMETHODIMP GetPriority(int *priority);
		STDMETHODIMP IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib);

		ShellExt(void);

	protected:
		~ShellExt(void);
                
		STGMEDIUM medium;
	private:
		// Reference count of component.
		long m_cRef;
		wchar_t *moduleFilename;
		int nFiles;
		std::vector<struct file_info> v_files;

		// reference to dataobject
		LPDATAOBJECT dataObj;

		HANDLE m_hMenuBmp;

		void OnVerbDisplayFileName(HWND hWnd);
};
#endif /* end of include guard: SHELLEXT_H */
