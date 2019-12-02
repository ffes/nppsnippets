/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010 Frank Fesevur                                       //
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
#include "Library.h"
#include "Resource.h"
#include "WaitCursor.h"

static Library* s_pLibrary = NULL;

/////////////////////////////////////////////////////////////////////////////
//

static bool Validate(HWND hDlg)
{
	// Check required fields
	if (GetDlgTextLength(hDlg, IDC_NAME) == 0)
	{
		MsgBox("Name needs to be entered!");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	if (!Validate(hDlg))
		return TRUE;

	// Get the data from the dialog
	using namespace std;
	wstring name = GetDlgText(hDlg, IDC_NAME);
	wstring created = GetDlgText(hDlg, IDC_CREATED_BY);
	wstring comments = GetDlgText(hDlg, IDC_COMMENTS);

	s_pLibrary->WSetName(name.c_str());
	s_pLibrary->WSetCreatedBy(created.c_str());
	s_pLibrary->WSetComments(comments.c_str());
	s_pLibrary->SetSortAlphabetic(IsDlgButtonChecked(hDlg, IDC_SORT_ALPHABET) == BST_CHECKED);

	// Save the library to the database
	try
	{
		WaitCursor wait;
		s_pLibrary->SaveToDB();
	}
	catch (SqliteException ex)
	{
		std::string msg = "Failed to save the library to the database!\n\n";
		msg += ex.what();
		MsgBox(msg.c_str());
	}

	// We're done
	EndDialog(hDlg, IDOK);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnCancel(HWND hDlg)
{
	EndDialog(hDlg, IDCANCEL);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	if (s_pLibrary == NULL)
		return TRUE;

	CenterWindow(hDlg);

	SetDlgItemText(hDlg, IDC_NAME, s_pLibrary->WGetName());
	SetDlgItemText(hDlg, IDC_CREATED_BY, s_pLibrary->WGetCreatedBy());
	SetDlgItemText(hDlg, IDC_COMMENTS, s_pLibrary->WGetComments());
	CheckDlgButton(hDlg, IDC_SORT_ALPHABET, s_pLibrary->GetSortAlphabetic() ? BST_CHECKED : BST_UNCHECKED);

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
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Show the Dialog

bool ShowEditLibraryDlg(Library* pLibrary)
{
	s_pLibrary = pLibrary;
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_EDIT_LIBRARY), g_nppData._nppHandle, (DLGPROC) DlgProc) == IDOK;
}
