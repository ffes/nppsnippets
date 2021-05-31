/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010-2021 Frank Fesevur                                  //
//                                                                         //
//  This program is free software; you can redistribute it and/or modify   //
//  it under the terms of the GNU General Public License as published by   //
//  the Free Software Foundation; either version 2 of the License, or      //
//  (at your option) any later version.                                    //
//                                                                         //
//  This program is distributed in the hope that it will be useful,        //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           //
//  GNU General Public License for more details.                           //
//                                                                         //
//  You should have received a copy of the GNU General Public License      //
//  along with this program; if not, write to the Free Software            //
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"
#include "Resource.h"
#include "Options.h"

static bool s_toolbarIcon = false;

/////////////////////////////////////////////////////////////////////////////
// This function is not actually used. Keep it here so when it *is* needed
// it gets called already.

static void CleanItems(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);
}

/////////////////////////////////////////////////////////////////////////////
//

static bool Validate(HWND hDlg)
{
	// Do we need to show a warning that N++ may need to be restarted
	const bool toolbarIcon = IsDlgButtonChecked(hDlg, IDC_INDENT_SNIPPET) == BST_CHECKED;
	if (s_toolbarIcon != toolbarIcon)
		MsgBox("Note that changing the toolbar icon only takes effect after restarting Notepad++");

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	// Are all the values in the dialog valid?
	if (!Validate(hDlg))
		return TRUE;

	// Put the items back in the options
	g_Options->SetToolbarIcon(IsDlgButtonChecked(hDlg, IDC_TOOLBAR_ICON) == BST_CHECKED);
	g_Options->SetIndentSnippet(IsDlgButtonChecked(hDlg, IDC_INDENT_SNIPPET) == BST_CHECKED);
	g_Options->SetDBFile(GetDlgText(hDlg, IDC_DBFILE));

	// We're done
	CleanItems(hDlg);
	EndDialog(hDlg, IDOK);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnCancel(HWND hDlg)
{
	CleanItems(hDlg);
	EndDialog(hDlg, IDCANCEL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnBrowse(HWND hDlg)
{
	// Init for GetOpenFileName()
	WCHAR szFile[_MAX_PATH];       // buffer for file name
	szFile[0] = 0;

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner =  hDlg;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFilter = L"NppSnippets.sqlite (NppSnippets.sqlite)\0NppSnippets.sqlite\0All SQLite databases (*.sqlite)\0*.sqlite\0\0";
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFile = szFile;

	// Ask for the filename
	if (GetOpenFileName(&ofn))
	{
		// Put the filename in the dialog item
		SetDlgItemText(hDlg, IDC_DBFILE, ofn.lpstrFile);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	// Center the window
	CenterWindow(hDlg);

	// Fill the items of the dialog
	CheckDlgButton(hDlg, IDC_TOOLBAR_ICON, g_Options->GetToolbarIcon() ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_INDENT_SNIPPET, g_Options->GetIndentSnippet() ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(hDlg, IDC_DBFILE, g_Options->GetDBFile().c_str());

	// Store the current value of `ToolbarIcon`
	s_toolbarIcon = g_Options->GetToolbarIcon();

	// Let windows set focus
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch(message)
	{
		case WM_INITDIALOG:
		{
			return OnInitDialog(hDlg);
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
					return OnOK(hDlg);

				case IDCANCEL:
					return OnCancel(hDlg);

				case IDC_BROWSE:
					return OnBrowse(hDlg);
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Show the Dialog

void ShowOptionsDlg()
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_OPTIONS), g_nppData._nppHandle, (DLGPROC) DlgProc);
}
