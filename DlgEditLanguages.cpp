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
#include <commctrl.h>
#include <stdio.h>
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"
#include "Library.h"
#include "Language.h"
#include "Resource.h"
#include "WaitCursor.h"
#include "SnippetsDB.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

static Library* s_pLibrary = NULL;
static HWND s_hWndLangList = NULL;

/////////////////////////////////////////////////////////////////////////////
//

static bool Validate(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);

	// Go through the listview items and collect the required data
	bool negative = false;
	int checked = 0, count = ListView_GetItemCount(s_hWndLangList);
	for (int i = 0; i < count; i++)
	{
		if (ListView_GetCheckState(s_hWndLangList, i))
		{
			checked++;

			// Check if the language-ID is negative
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(LVITEM));
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = i;
			ListView_GetItem(s_hWndLangList, &lvitem);
			Language* lang = (Language*) lvitem.lParam;
			if (lang != NULL)
				if (lang->GetLangID() < 0)
					negative = true;
		}
	}

	// Check if there is at least one language selected
	if (checked == 0)
	{
		MsgBox("No language selected");
		return false;
	}

	// If there is a negative language selected, make sure only one language is selected
	if (negative && checked > 1)
	{
		MsgBox("A general language has been choosen,\r\nbut there is more then one language selected.");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static void CleanItems(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);

	int count = ListView_GetItemCount(s_hWndLangList);
	for (int i = 0; i < count; i++)
	{
		LVITEM lvitem;
		ZeroMemory(&lvitem, sizeof(LVITEM));
		lvitem.mask = LVIF_PARAM;
		lvitem.iItem = i;
		ListView_GetItem(s_hWndLangList, &lvitem);
		Language* lang = (Language*) lvitem.lParam;
		if (lang != NULL)
			delete lang;
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static bool SaveLanguages()
{
	// First delete all the current items
	SqliteStatement stmt(g_db, "DELETE FROM LibraryLang WHERE LibraryID = @libid");

	// Bind the language to the statement
	int libid = s_pLibrary->GetLibraryID();
	stmt.Bind("@libid", libid);

	// And save the record
	stmt.SaveRecord();
	stmt.Finalize();

	// Now add records for all selected languages
	stmt.Prepare("INSERT INTO LibraryLang(LibraryID, Lang) VALUES (@libid, @lang)");

	// Go through the selected items
	int count = ListView_GetItemCount(s_hWndLangList);
	for (int i = 0; i < count; i++)
	{
		if (ListView_GetCheckState(s_hWndLangList, i))
		{
			// Get the Language-class from this checked item
			LV_ITEM lvitem;
			ZeroMemory(&lvitem, sizeof(LVITEM));
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = i;
			ListView_GetItem(s_hWndLangList, &lvitem);
			Language* lang = (Language*) lvitem.lParam;
			if (lang != NULL)
			{
				// Bind the language-id and library-id to the statement
				stmt.Bind("@libid", libid);
				stmt.Bind("@lang", lang->GetLangID());
				stmt.SaveRecord();
			}
		}
	}

	stmt.Finalize();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnOK(HWND hDlg)
{
	if (!Validate(hDlg))
		return TRUE;

	WaitCursor wait;

	// Save the changes to the database
	g_db->Open();
	g_db->BeginTransaction();

	if (SaveLanguages())
		g_db->CommitTransaction();
	else
		g_db->RollbackTransaction();

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

static bool AddItem(Language* lang)
{
	LVITEM lvitem;
	ZeroMemory(&lvitem, sizeof(LVITEM));
	lvitem.mask = LVIF_TEXT | LVIF_PARAM;
	lvitem.iItem = ListView_GetItemCount(s_hWndLangList);
	lvitem.lParam = (LPARAM) lang;
	lvitem.pszText = lang->GetLangName();
	int actualItem = ListView_InsertItem(s_hWndLangList, &lvitem);

	// Set the text for the first subitem
	ZeroMemory(&lvitem, sizeof(LVITEM));
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = actualItem;
	lvitem.iSubItem = 1;
	lvitem.pszText = lang->GetLangDescr();
	ListView_SetItem(s_hWndLangList, &lvitem);

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool AddColumn(int col, int width, LPWSTR name)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = col;
	lvc.cx = width;
	lvc.pszText = name;
	ListView_InsertColumn(s_hWndLangList, col, &lvc);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	// Just in case
	if (s_pLibrary == NULL)
		return TRUE;

	WaitCursor wait;

	CenterWindow(hDlg);
	SetWindowText(GetDlgItem(hDlg, IDC_NAME), s_pLibrary->WGetName());
	s_hWndLangList = GetDlgItem(hDlg, IDC_LANG_LIST);
	ListView_SetExtendedListViewStyle(s_hWndLangList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	//
	AddColumn(0, 95, L"Language");
	AddColumn(1, 188, L"Description");

	// First add the special languages to the list
	AddItem(new Language(-2, L"All", L"All languages"));
	AddItem(new Language(-1, L"No Lib for lang", L"No other library for this language"));

	// Then add all the normal supported languages
	int langid = 0;
	for (langid = L_TEXT; langid < L_EXTERNAL; langid++)
		AddItem(new Language(langid));

	// Set the checkboxes for the right languages
	g_db->Open();

	// Create a new statement
	SqliteStatement stmt(g_db, "SELECT Lang FROM LibraryLang WHERE LibraryID = @langid ORDER BY Lang");

	// Bind the library to the statement
	stmt.Bind("@langid", s_pLibrary->GetLibraryID());

	// Go through the records
	int item = 0, count = ListView_GetItemCount(s_hWndLangList);
	while (stmt.GetNextRecord())
	{
		langid = stmt.GetIntColumn("Lang");

		// Go through the listview items to set the checkbox
		for (item = 0; item < count; item++)
		{
			LVITEM lvitem;
			ZeroMemory(&lvitem, sizeof(LVITEM));
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = item;
			ListView_GetItem(s_hWndLangList, &lvitem);
			Language* lang = (Language*) lvitem.lParam;
			if (lang != NULL)
			{
				if (lang->GetLangID() == langid)
				{
					ListView_SetCheckState(s_hWndLangList, item, TRUE);
					break;
				}
			}
		}
	}
	stmt.Finalize();
	g_db->Close();

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

bool ShowEditLanguagesDlg(Library* pLibrary)
{
	s_pLibrary = pLibrary;
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_EDIT_LANGUAGES), g_nppData._nppHandle, (DLGPROC) DlgProc) == IDOK;
}
