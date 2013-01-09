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
#include "Database.h"

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

static bool AttachDatabase()
{
	sqlite3_stmt* stmt = NULL;
	int res = sqlite3_prepare_v2(g_db, "ATTACH DATABASE @file AS Import", -1, &stmt, 0);
	if (res != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	// Bind the library to the statement
	BindText(stmt, "@file", s_databaseFile);

	if (sqlite3_step(stmt) != SQLITE_DONE)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool AddLibraryToCombo(HWND hDlg)
{
	WaitCursor wait;

	if (!OpenDB())
		return false;

	AttachDatabase();

	// Add the libraries in the attached database to the combobox
	sqlite3_stmt* stmt = NULL;
	int res = sqlite3_prepare_v2(g_db, "SELECT * FROM Import.Library ORDER BY Name", -1, &stmt, 0);
	if (res != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	// Go through the records
	long item = 0;
	bool first = true;
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		// Store the library data
		Library* lib = new Library(stmt);

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

	if (sqlite3_finalize(stmt) != SQLITE_OK)
		MsgBox(sqlite3_errmsg(g_db));

	RunSQL("DETACH DATABASE Import");
	CloseDB();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool ImportSnippets(long orgLibID, long newLibID)
{
	// Get all the snippets from the attached database
	sqlite3_stmt* stmt = NULL;
	int res = sqlite3_prepare_v2(g_db, "SELECT * FROM Import.Snippets WHERE LibraryID = @libid", -1, &stmt, 0);
	if (res != SQLITE_OK)
	{
		//MsgBox("INSERT-prepare() FAILED");
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	BindInt(stmt, "@libid", orgLibID);

	// Go through the records and save them to the database
	Snippet snip;
	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		snip.Set(stmt);
		snip.SetSnippetID(0);
		snip.SetLibraryID(newLibID);
		snip.SaveToDB(false);
	}

	if (sqlite3_finalize(stmt) != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool ImportLanguages(long orgLibID, long newLibID)
{
	// Get all the languages for this library from the attached database
	sqlite3_stmt* smtmSelect = NULL;
	int res = sqlite3_prepare_v2(g_db, "SELECT Lang FROM Import.LibraryLang WHERE LibraryID = @libid", -1, &smtmSelect, 0);
	if (res != SQLITE_OK)
	{
		//MsgBox("INSERT-prepare() FAILED");
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}
	BindInt(smtmSelect, "@libid", orgLibID);

	// Open a select stmt to store the new data in the table
	sqlite3_stmt* stmtInsert = NULL;
	res = sqlite3_prepare_v2(g_db, "INSERT INTO LibraryLang(LibraryID, Lang) VALUES (@libid, @lang)", -1, &stmtInsert, 0);
	if (res != SQLITE_OK)
	{
		//MsgBox("INSERT-prepare() FAILED");
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	// Go through the attached records and save them to the database
	while (sqlite3_step(smtmSelect) == SQLITE_ROW)
	{
		sqlite3_reset(stmtInsert);
		BindInt(stmtInsert, "@libid", newLibID);
		BindInt(stmtInsert, "@lang", sqlite3_column_int(smtmSelect, 0));

		// Put the record in the database
		if (sqlite3_step(stmtInsert) != SQLITE_DONE)
		{
			//MsgBox("INSERT-step() FAILED");
			MsgBox(sqlite3_errmsg(g_db));
			return false;
		}
	}

	if (sqlite3_finalize(smtmSelect) != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	if (sqlite3_finalize(stmtInsert) != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool ImportLibrary(Library* lib)
{
	WaitCursor wait;

	if (!OpenDB())
	{
		MsgBox("Error opening database. Changes not saved");
		return false;
	}

	// First save the selected library as a new library
	long orgLibID = lib->GetLibraryID();
	lib->SetLibraryID(0);
	lib->SaveToDB(false);

	AttachDatabase();
	ImportSnippets(orgLibID, lib->GetLibraryID());
	ImportLanguages(orgLibID, lib->GetLibraryID());
	RunSQL("DETACH DATABASE Import");
	CloseDB();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	if (!Validate(hDlg))
		return TRUE;

	// Get current LibraryID from the selected item
	int item = (int) SendDlgItemMessage(hDlg, IDC_NAME, CB_GETCURSEL, 0, 0L);
	Library* lib = (Library*) SendDlgItemMessage(hDlg, IDC_NAME, CB_GETITEMDATA, (WPARAM) item, 0);
	ImportLibrary(lib);

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
