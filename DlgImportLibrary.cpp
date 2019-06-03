/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010-2011 Frank Fesevur                                  //
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
#include "SnippetsDB.h"

static LPCWSTR s_databaseFile = NULL;

/////////////////////////////////////////////////////////////////////////////
//

static bool Validate(HWND hDlg)
{
	// Is there a library selected?
	if (SendDlgItemMessage(hDlg, IDC_NAME, CB_GETCURSEL, 0, 0L) == CB_ERR)
	{
		MsgBox("No library selected");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static void CleanItems(HWND hDlg)
{
	// First need to clean up the ItemData!!!
	Library* lib = NULL;
	int count = (int) SendDlgItemMessage(hDlg, IDC_NAME, CB_GETCOUNT, 0, 0);
	for (int item = 0; item < count; item++)
	{
		lib = (Library*) SendDlgItemMessage(hDlg, IDC_NAME, CB_GETITEMDATA, (WPARAM) item, 0);
		delete lib;
	}

	// Now delete the items from the combobox
	SendDlgItemMessage(hDlg, IDC_NAME, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
}

/////////////////////////////////////////////////////////////////////////////
//

static bool AddLibraryToCombo(HWND hDlg)
{
	WaitCursor wait;

	g_db->Open();
	g_db->Attach(s_databaseFile, L"Import");

	// Add the libraries in the attached database to the combobox
	SqliteStatement stmt(g_db, "SELECT * FROM Import.Library ORDER BY Name");

	// Go through the records
	long item = 0;
	bool first = true;
	while (stmt.GetNextRecord())
	{
		// Store the library data
		Library* lib = new Library(&stmt);

		// Add the item to the combo
		item = (long) SendDlgItemMessage(hDlg, IDC_NAME, CB_ADDSTRING, (WPARAM) 0, (LPARAM) lib->WGetName());
		SendDlgItemMessage(hDlg, IDC_NAME, CB_SETITEMDATA, (WPARAM) item, (LPARAM) lib);

		// Need to select the current library?
		if (first)
		{
			SendDlgItemMessage(hDlg, IDC_NAME, CB_SETCURSEL, (WPARAM) item, (LPARAM) 0);
			first = false;
		}
	}
	stmt.Finalize();

	g_db->Detach(L"Import");
	g_db->Close();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	if (!Validate(hDlg))
		return TRUE;

	// Get Library from the selected item
	WaitCursor wait;
	int item = (int) SendDlgItemMessage(hDlg, IDC_NAME, CB_GETCURSEL, 0, 0L);
	Library* lib = (Library*) SendDlgItemMessage(hDlg, IDC_NAME, CB_GETITEMDATA, (WPARAM) item, 0);

	// Import the data
	g_db->Open();
	g_db->ImportLibrary(s_databaseFile, lib->GetLibraryID());
	g_db->Close();

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

static BOOL OnInitDialog(HWND hDlg)
{
	CenterWindow(hDlg);
	AddLibraryToCombo(hDlg);
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

bool ImportLibraryDlg(LPCWSTR databaseFile)
{
	s_databaseFile = databaseFile;
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_IMPORT_LIBRARY), g_nppData._nppHandle, (DLGPROC) DlgProc) == IDOK;
}
