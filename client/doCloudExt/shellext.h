/****************************** Module Header ******************************\
Module Name:  ShellExt.h
Project:      CppShellExtContextMenuHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates creating a Shell context menu handler with C++. 

A context menu handler is a shell extension handler that adds commands to an 
existing context menu. Context menu handlers are associated with a particular 
file class and are called any time a context menu is displayed for a member 
of the class. While you can add items to a file class context menu with the 
registry, the items will be the same for all members of the class. By 
implementing and registering such a handler, you can dynamically add items to 
an object's context menu, customized for the particular object.

The example context menu handler adds the menu item "Display File Name (C++)"
to the context menu when you right-click a .cpp file in the Windows Explorer. 
Clicking the menu item brings up a message box that displays the full path 
of the .cpp file.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

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

		// reference to dataobject
		LPDATAOBJECT dataObj;

		HANDLE m_hMenuBmp;

		void OnVerbDisplayFileName(HWND hWnd);
};
