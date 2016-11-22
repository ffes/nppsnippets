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
#include <windowsx.h>
#include <stdio.h>
#include <shlwapi.h>

#include "NPP/PluginInterface.h"
#include "NPP/menuCmdID.h"
#include "NPP/Docking.h"
#include "NppSnippets.h"
#include "Library.h"
#include "Snippets.h"
#include "Resource.h"
#include "DlgEditSnippet.h"
#include "DlgEditLibrary.h"
#include "DlgEditLanguages.h"
#include "DlgImportLibrary.h"
#include "Options.h"
#include "WaitCursor.h"

#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// Various static variables

static HWND s_hDlg = NULL;						// The HWND to the dialog
static HWND s_hList = NULL;						// The HWND to the listbox
static HWND s_hCombo = NULL;					// The HWND to the combo
static HICON s_hTabIcon = NULL;					// The icon on the docking tab
static HBRUSH s_hbrBkgnd = NULL;				// The brush to paint the theme on the background of the listbox
static int s_iHeightCombo = 20;					// This info should come from Windows
static bool s_bConsoleInitialized = false;		// Is the console initialized?
static bool s_bConsoleVisible = false;			// Is the console visible?
static Library* s_curLibrary = NULL;			// The currently selected lib
static int s_curLang = -1;						// The "LangType" of the currently selected lib

/////////////////////////////////////////////////////////////////////////////
//

static bool AddLibsToCombo(int lang)
{
	// First get the last used library for this language
	int lastUsed = -1;
	SqliteStatement stmt(g_db, "SELECT LibraryID FROM LangLastUsed WHERE Lang = @langid");
	stmt.Bind("@langid", lang);
	if (stmt.GetNextRecord())
		lastUsed = stmt.GetIntColumn("LibraryID");
	stmt.Finalize();

	// Get all the libraries for this language
	stmt.Prepare("SELECT Library.*, LibraryLang.Lang FROM Library INNER JOIN LibraryLang ON Library.LibraryID = LibraryLang.LibraryID WHERE (LibraryLang.Lang = @langid OR LibraryLang.Lang = -2) ORDER BY Library.Name");
	stmt.Bind("@langid", lang);

	// Go through the rows
	int colLang = stmt.GetColumnCount() - 1;
	bool first = true;
	bool firstLanguage = false;
	bool selectedUsersChoice = false;		// Did the user select a library?
	while (stmt.GetNextRecord())
	{
		// Store the library data
		Library* lib = new Library(&stmt);

		// Add the item to the combo
		long item = (long) SendDlgItemMessage(s_hDlg, IDC_NAME, CB_ADDSTRING, (WPARAM) 0, (LPARAM) lib->WGetName());
		SendDlgItemMessage(s_hDlg, IDC_NAME, CB_SETITEMDATA, (WPARAM) item, (LPARAM) lib);

		// Determine if we need to select this library?
		bool selectThisLib = first;

		// Did we run into the user selected library already?
		if (!selectedUsersChoice)
		{
			if (lib->GetLibraryID() == lastUsed)
			{
				selectedUsersChoice = true;
				selectThisLib = true;
			}
			else
			{
				if (!firstLanguage)
				{
					int libLang = stmt.GetIntColumn(colLang);
					if (libLang >= 0)
					{
						firstLanguage = true;
						selectThisLib = true;
					}
				}
			}
		}

		// Need to select this library?
		if (selectThisLib)
		{
			SendDlgItemMessage(s_hDlg, IDC_NAME, CB_SETCURSEL, (WPARAM) item, (LPARAM) 0);
			s_curLibrary = lib;
		}
		first = false;
	}
	stmt.Finalize();

	return !first;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool AddComboItems()
{
	// Add the Libraries to the Combobox for the documents language
	if (AddLibsToCombo(g_currentLang))
	{
		s_curLang = g_currentLang;
		return true;
	}

	// If no library is found for this language, try for the generic language
	if (AddLibsToCombo(-1))
	{
		s_curLang = -1;
		return true;
	}

	// Not even a generic library found in the database
	s_curLang = -1;
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//

static bool AddListItems()
{
	if (s_curLibrary == NULL)
		return false;

	// Create the proper SQL-statement
	char szSQL[MAX_PATH];
	strncpy(szSQL, "SELECT * FROM Snippets WHERE LibraryID = @libid ORDER BY ", MAX_PATH);
	strncat(szSQL, s_curLibrary->GetSortAlphabetic() ? "Name,Sort" : "Sort,Name", MAX_PATH);

	// Prepare this statement
	SqliteStatement stmt(g_db, szSQL);

	// Bind the language-id to the statement
	stmt.Bind("@libid", s_curLibrary->GetLibraryID());

	// Go through the rows
	while (stmt.GetNextRecord())
	{
		// Get the snippet from the database
		Snippet* snip = new Snippet(&stmt);

		// Put in the the list
		long item = (long) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_ADDSTRING, (WPARAM) 0, (LPARAM) snip->WGetName());
		SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETITEMDATA, (WPARAM) item, (LPARAM) snip);
	}
	stmt.Finalize();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

static void ClearComboItems()
{
	// First need to clean up the ItemData!!!
	Library* lib = NULL;
	int count = (int) SendDlgItemMessage(s_hDlg, IDC_NAME, CB_GETCOUNT, 0, 0);
	for (int item = 0; item < count; item++)
	{
		lib = (Library*) SendDlgItemMessage(s_hDlg, IDC_NAME, CB_GETITEMDATA, (WPARAM) item, 0);
		delete lib;
	}
	s_curLibrary = NULL;

	// Now delete the items from the combobox
	SendDlgItemMessage(s_hDlg, IDC_NAME, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
}

/////////////////////////////////////////////////////////////////////////////
//

static void ClearListItems()
{
	// First need to clean up the ItemData!!!
	Snippet* snip = NULL;
	int count = (int) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETCOUNT, 0, 0);
	for (int item = 0; item < count; item++)
	{
		snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
		delete snip;
	}

	// Now delete the items from the listbox
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
}

/////////////////////////////////////////////////////////////////////////////
//

void UpdateSnippetsList()
{
	if (!s_bConsoleVisible)
		return;

	WaitCursor wait;

	ClearComboItems();
	ClearListItems();

	g_db->Open();
	AddComboItems();
	AddListItems();
	g_db->Close();
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL DbExist(UINT uMsg, LPWSTR dbFile, LPCWSTR szExtraDir, bool bMainDB)
{
	// Get the directory from NP++
	WCHAR tmp[MAX_PATH];
	SendMessage(g_nppData._nppHandle, uMsg, MAX_PATH, (LPARAM) &tmp);
	wcsncpy(dbFile, tmp, MAX_PATH);

	// Add the extra dictory (if any)
	if (szExtraDir != NULL)
		wcsncat(dbFile, szExtraDir, MAX_PATH);

	// Add the filename of the database
	if (bMainDB)
		wcsncat(dbFile, L"\\NppSnippets.sqlite", MAX_PATH);
	else
		wcsncat(dbFile, L"\\Template.sqlite", MAX_PATH);

	// Check if the file exists
	return PathFileExists(dbFile);
}

/////////////////////////////////////////////////////////////////////////////
// Get or construct the filename for the database

static bool GetDatabaseFile()
{

	// First see if there is a user-defined path for the database
	if (PathFileExists(g_Options->GetDBFile()))
	{
		g_db->SetFilename(g_Options->GetDBFile());
		return true;
	}

	// Then try in the Plugin config dir in AppData. Best place to put the database!!!
	WCHAR dbFile[MAX_PATH];
	if (DbExist(NPPM_GETPLUGINSCONFIGDIR, dbFile, NULL, true))
	{
		g_db->SetFilename(dbFile);
		return true;
	}

	// If the database is still not found, it could be in the installation directory.
	// Putting your database there could result in read-only problems
	if (DbExist(NPPM_GETNPPDIRECTORY, dbFile, L"\\plugins\\Config", true))
	{
		g_db->SetFilename(dbFile);
		return true;
	}

	// If it is still not found, we most likely have a fresh installation. Look for the template database
	if (DbExist(NPPM_GETNPPDIRECTORY, dbFile, L"\\plugins\\NppSnippets", false))
	{
		// Store the filename of the template database
		WCHAR szFrom[MAX_PATH];
		wcsncpy(szFrom, dbFile, MAX_PATH);

		// Get the user's plugin-config directory
		DbExist(NPPM_GETPLUGINSCONFIGDIR, dbFile, NULL, true);

		// Copy template to "NPPM_GETPLUGINSCONFIGDIR"
		if (CopyFile(szFrom, dbFile, TRUE))
		{
			g_db->SetFilename(dbFile);
			return true;
		}
	}

	// Still not found, clean the filename
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//

static void ShowContextMenu(HWND hwnd, UINT uResID, int xPos, int yPos)
{
	// Load the menu resource.
	HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(uResID));
	if (hMenu == NULL)
		return;

	// TrackPopupMenu cannot display the menu bar so get a handle to the first shortcut menu. 
	HMENU hPopup = GetSubMenu(hMenu, 0);

	// Disable various items of the snippet menu
	if (uResID == IDCM_SNIPPET)
	{
		// Disable the "move up" and "move down" items
		// if the library is alphabetticly ordered
		if (s_curLibrary->GetSortAlphabetic())
		{
			EnableMenuItem(hPopup, IDC_SNIPPET_MOVE_UP, MF_DISABLED);
			EnableMenuItem(hPopup, IDC_SNIPPET_MOVE_DOWN, MF_DISABLED);
		}

		// Need to disable the clipboard entry?
		bool enable = false;
		if (OpenClipboard(hwnd))
		{
			// Get the content of the clipboard
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);
			WCHAR* buffer = (WCHAR*) GlobalLock(hData);
			enable = (buffer != NULL);
			GlobalUnlock(hData);
			CloseClipboard();
		}
		if (!enable)
			EnableMenuItem(hPopup, IDC_SNIPPET_ADD_CLIPBOARD, MF_DISABLED);

		// Need to disable the selection entry?
		if (SendMsg(SCI_GETSELECTIONEND) == SendMsg(SCI_GETSELECTIONSTART))
			EnableMenuItem(hPopup, IDC_SNIPPET_ADD_SELECTION, MF_DISABLED);
	}

	// Display the shortcut menu. Track the right mouse button.
	TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON, xPos, yPos, 0, hwnd, NULL);

	// Destroy the menu.
	DestroyMenu(hMenu);
}

/////////////////////////////////////////////////////////////////////////////
// Set the focus back on the edit window

static void SetFocusOnEditor()
{
	int currentEdit;
	::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &currentEdit);
	SetFocus(getCurrentHScintilla(currentEdit));	
}

/////////////////////////////////////////////////////////////////////////////
// Start a new document. Does not work yet, therefore only in debug-builds

static void StartNewDocument(LangType lang)
{
	// Start a new document, if not empty
	if (SendMsg(SCI_GETLENGTH) > 0)
		SendMessage(g_nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

	// Set the language for the new document
	if (lang != L_EXTERNAL)
		SendMessage(g_nppData._nppHandle, NPPM_SETCURRENTLANGTYPE, 0, lang);

	// Put the focus on the editor
	SetFocusOnEditor();
}

/////////////////////////////////////////////////////////////////////////////
//

#define SPACER 4

static void OnSize(HWND hWnd, int iWidth, int iHeight)
{
	UNREFERENCED_PARAMETER(hWnd);
	SetWindowPos(s_hList, 0, 0, s_iHeightCombo + SPACER, iWidth, iHeight - s_iHeightCombo - SPACER, SWP_NOACTIVATE | SWP_NOZORDER);
	SetWindowPos(s_hCombo, 0, 0, 0, iWidth, s_iHeightCombo, SWP_NOACTIVATE | SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnSelChange_Combo(HWND hWnd)
{
	// Store the previously used LibraryID
	int prevLibID = s_curLibrary->GetLibraryID();

	// Get current LibraryID from the selected item
	int item = (int) SendDlgItemMessage(hWnd, IDC_NAME, CB_GETCURSEL, 0, 0L);
	s_curLibrary = (Library*) SendDlgItemMessage(hWnd, IDC_NAME, CB_GETITEMDATA, (WPARAM) item, 0);

	// Did the library ID really change?
	if (prevLibID == s_curLibrary->GetLibraryID())
		return;

	// Open the database
	g_db->Open();

	// Update the database
	char szSQL[MAX_PATH];
	_snprintf(szSQL, MAX_PATH, "DELETE FROM LangLastUsed WHERE Lang = %d; INSERT INTO LangLastUsed(Lang,LibraryID) VALUES(%d,%d);", s_curLang, s_curLang, s_curLibrary->GetLibraryID());
	g_db->Execute(szSQL);

	// Update the list with snippets
	ClearListItems();
	AddListItems();

	g_db->Close();
}

/////////////////////////////////////////////////////////////////////////////
// Display a context menu for the selected item, or a general one
// Note that point is in Screen coordinates!

static void OnContextMenu(HWND hWnd, int xPos, int yPos, HWND hChild)
{
	if (hChild == s_hCombo)
	{
		ShowContextMenu(hWnd, IDCM_LIBRARY, xPos, yPos);
		return;
	}

	if (hChild == s_hList)
	{
/*
		// Find out where the cursor was and select that item from the list
		POINT pt;
		pt.x = xPos;
		pt.y = yPos;
		ScreenToClient(&pt);
		DWORD dw = (DWORD) SendMessage(m_hList, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
*/

		ShowContextMenu(hWnd, IDCM_SNIPPET, xPos, yPos);
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hWnd)
{
	// Store the DlgItems
	s_hList = GetDlgItem(hWnd, IDC_LIST);
	s_hCombo = GetDlgItem(hWnd, IDC_NAME);

	// Get the height of the combobox
	RECT rc;
	GetWindowRect(s_hCombo, &rc);
	s_iHeightCombo = rc.bottom - rc.top;

	// Let windows set focus
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Cleanup the mess

static void OnClose(HWND hWnd)
{
	ClearComboItems();
	ClearListItems();

	if (s_hTabIcon != NULL)
	{
		DestroyIcon(s_hTabIcon);
		s_hTabIcon = NULL;
	}

	EndDialog(hWnd, 0);
}

/////////////////////////////////////////////////////////////////////////////
// Convert a wide Unicode string to an UTF8 string

static std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

/////////////////////////////////////////////////////////////////////////////
// Insert the snippet (selected from the context menu item) into the text
// in the Scintilla window. Double clicking get redirected to here as well.

static void OnSnippetInsert(HWND hWnd)
{
	// Get the current item and its snippet
	int item = (int) SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0L);

	// Was there an item selected?
	if (item == LB_ERR)
		return;

	// Get the data for this item and store it in the local snip-variable
	Snippet* pSnip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
	Snippet snip = *pSnip;

	// Do we need to start a new document?
	if (snip.GetNewDocument())
		StartNewDocument(snip.GetNewDocumentLang());

	// Start an undo action, to make sure this insert is one action to undo
	SendMsg(SCI_BEGINUNDOACTION);

	//SCI_GETSELECTIONS
	//SCI_GETSELECTIONNSTART(int selection)

	// Get the text in current selection
	int selsize = (int) SendMsg(SCI_GETSELECTIONEND) - (int) SendMsg(SCI_GETSELECTIONSTART);
	char* pszText = NULL;
	if (selsize != 0)
	{
		pszText = new char[selsize + 2];
		if (pszText == NULL)
			return;
		SendMsg(SCI_GETSELTEXT, 0, (LPARAM) pszText);
	}

	// Insert the first part of the snippet
	std::wstring wstr = ConvertLineEnding(snip.WGetBeforeSelection());
	std::string strTo = utf8_encode(wstr);
	SendMsg(SCI_REPLACESEL, 0, (LPARAM) strTo.c_str());

	int beforeSel = (int) SendMsg(SCI_GETCURRENTPOS);

	// Put back the selection (if any)
	if (pszText != NULL)
	{
		if (!snip.GetReplaceSelection())
			SendMsg(SCI_REPLACESEL, 0, (LPARAM) pszText);
		delete [] pszText;
	}

	int afterSel = (int) SendMsg(SCI_GETCURRENTPOS);

	// Is there a select part of the snippet to add?
	if (snip.WGetAfterSelection() != NULL)
	{
		size_t len = wcslen(snip.WGetAfterSelection());
		if (len > 0)
		{
			std::wstring wstr = ConvertLineEnding(snip.WGetAfterSelection());
			std::string strTo = utf8_encode(wstr);
			SendMsg(SCI_REPLACESEL, 0, (LPARAM) strTo.c_str());
		}
	}

	// Restore the cursor position and selection
	SendMsg(SCI_SETANCHOR, beforeSel);
	SendMsg(SCI_SETCURRENTPOS, afterSel);

	// End the undo action
	SendMsg(SCI_ENDUNDOACTION);

	// Put the focus back on the edit window
	SetFocusOnEditor();
}

/////////////////////////////////////////////////////////////////////////////
// Get the highest used sort number

static long GetMaxSort()
{
	long maxSort = 0;
	int count = (int) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETCOUNT, 0, 0);
	for (int item = 0; item < count; item++)
	{
		Snippet* snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
		long sort = snip->GetSort();
		if (sort > maxSort)
			maxSort = sort;
	}
	return maxSort;
}

/////////////////////////////////////////////////////////////////////////////
// Add a new snippet to the current library.

static void OnSnippetAdd(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	// Create the new snippet and init some required members
	Snippet* snip = new Snippet();
	snip->SetLibraryID(s_curLibrary->GetLibraryID());
	snip->SetSort(GetMaxSort() + 1);

	// Let the user edit the new snippet
	bool closedOK = ShowEditSnippetDlg(snip);

	// We don't need the object anymore, it was stored in the database if OK was pressed
	delete snip;

	// If the users pressed OK, refresh everything from the database
	if (closedOK)
		UpdateSnippetsList();
}

/////////////////////////////////////////////////////////////////////////////
// Edit the currently selected snippet

static void OnSnippetEdit(HWND hWnd)
{
	// Get the current item and its snippet
	int item = (int) SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0L);

	// Was there an item selected?
	if (item == LB_ERR)
		return;

	// Get the data for this item
	Snippet* snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
	if (snip == NULL)
		return;

	// Let the uses edit the snippet
	if (ShowEditSnippetDlg(snip))
	{
		// Put the (changed?) name back in the list
		SendDlgItemMessage(s_hDlg, IDC_LIST, LB_DELETESTRING, (WPARAM) item, (LPARAM) NULL);
		SendDlgItemMessage(s_hDlg, IDC_LIST, LB_INSERTSTRING, (WPARAM) item, (LPARAM) snip->WGetName());
		SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETITEMDATA, (WPARAM) item, (LPARAM) snip);
		SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETCURSEL, (WPARAM) item, (LPARAM) NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Delete the snippet from the list and database

static void OnSnippetDelete(HWND hWnd)
{
	// Get the current item and its snippet
	int item = (int) SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0L);

	// Was there an item selected?
	if (item == LB_ERR)
		return;

	// Get the data for this item
	Snippet* snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
	if (snip == NULL)
		return;

	WCHAR wszTmp[MAX_PATH];
	wcsncpy(wszTmp, L"Are you sure you want to delete the snippet:\r\n", MAX_PATH);
	wcsncat(wszTmp, snip->WGetName(), MAX_PATH);

	// Ask for confirmation
	if (!MsgBoxYesNo(wszTmp))
		return;

	// Delete from the database
	if (!snip->DeleteFromDB())
		return;

	// Clean up the item data
	delete snip;

	// Delete the entry from the list
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_DELETESTRING, (WPARAM) item, (LPARAM) NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Duplicate the currently selected snippet

static void OnSnippetDuplicate(HWND hWnd)
{
	// Get the current item and its snippet
	int item = (int) SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0L);

	// Was there an item selected?
	if (item == LB_ERR)
		return;

	// Get the data for this item
	Snippet* snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
	if (snip == NULL)
		return;

	WCHAR wszTmp[MAX_PATH];
	wcsncpy(wszTmp, L"Are you sure you want to duplicate the snippet:\r\n", MAX_PATH);
	wcsncat(wszTmp, snip->WGetName(), MAX_PATH);

	// Ask for confirmation
	if (!MsgBoxYesNo(wszTmp))
		return;

	// Make a copy of the selected snippet
	Snippet* pNew = new Snippet(*snip);

	// Adjust the name of the duplicate
	wcsncpy(wszTmp, snip->WGetName(), MAX_PATH);
	wcsncat(wszTmp, L" - Dup", MAX_PATH);
	pNew->WSetName(wszTmp);

	// Save to the database and refresh the list
	pNew->SaveToDB();
	UpdateSnippetsList();
}

/////////////////////////////////////////////////////////////////////////////
// Add a new snippet based upon the current content of the clipboard

static void OnSnippetAddClipboard(HWND hWnd)
{
	if (!OpenClipboard(hWnd))
		return;

	// Get the content of the clipboard
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	WCHAR* buffer = (WCHAR*) GlobalLock(hData);
	if (buffer != NULL)
	{
		// Create the new snippet and init some required members
		Snippet* snip = new Snippet();
		snip->SetLibraryID(s_curLibrary->GetLibraryID());
		snip->SetSort(GetMaxSort() + 1);
		snip->WSetBeforeSelection(buffer);
		snip->GuessName();

		// Let the user edit the new snippet
		bool closedOK = ShowEditSnippetDlg(snip);

		// We don't need the object anymore, it was stored in the database if OK was pressed
		delete snip;

		// If the users pressed OK, refresh everything from the database
		if (closedOK)
			UpdateSnippetsList();
	}
	GlobalUnlock(hData);
	CloseClipboard();
}

/////////////////////////////////////////////////////////////////////////////
// Add a new snippet based upon the current selection

static void OnSnippetAddSelection(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	// Get the text in current selection
	int selsize = (int) SendMsg(SCI_GETSELECTIONEND) - (int) SendMsg(SCI_GETSELECTIONSTART);
	if (selsize == 0)
	{
		MsgBox("No selection found");
		return;
	}
	char* pszText = new char[selsize + 2];
	SendMsg(SCI_GETSELTEXT, 0, (LPARAM) pszText);

	// Create the new snippet and init some required members
	Snippet* snip = new Snippet();
	snip->SetLibraryID(s_curLibrary->GetLibraryID());
	snip->SetSort(GetMaxSort() + 1);
	snip->SetBeforeSelection(pszText);
	snip->GuessName();
	delete [] pszText;

	// Let the user edit the new snippet
	bool closedOK = ShowEditSnippetDlg(snip);

	// We don't need the object anymore, it was stored in the database if OK was pressed
	delete snip;

	// If the users pressed OK, refresh everything from the database
	if (closedOK)
		UpdateSnippetsList();
}

/////////////////////////////////////////////////////////////////////////////
// Update all the items in the database as well, based upon the current list

static void UpdateSortInDB()
{
	g_db->Open();
	g_db->BeginTransaction();

	SqliteStatement stmt(g_db, "UPDATE Snippets SET Sort = @sort WHERE SnippetID = @id");

	int colID = stmt.GetBindParameterIndex("@id");
	int colSort = stmt.GetBindParameterIndex("@sort");

	Snippet* snip = NULL;
	int count = (int) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETCOUNT, 0, 0);
	for (int item = 0; item < count; item++)
	{
		snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
		snip->SetSort(item + 1);
		stmt.Bind(colID, snip->GetSnippetID());
		stmt.Bind(colSort, snip->GetSort());
		try
		{
			stmt.SaveRecord();
		}
		catch(SqliteException e)
		{
			//MsgBox(sqlite3_errmsg(g_db));
			g_db->RollbackTransaction();
			g_db->Close();
			return;
		}
	}
	stmt.Finalize();

	g_db->CommitTransaction();
	g_db->Close();
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnSnippetMoveUp(HWND hWnd)
{
	// Just in case
	if (s_curLibrary->GetSortAlphabetic())
		return;

	// Get the current item and its snippet
	int item = (int) SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0L);

	// Was there an item selected?
	if (item == LB_ERR)
		return;

	// Are we at the top of the list?
	if (item == 0)
		return;

	// Get the data for this item
	Snippet* snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
	if (snip == NULL)
		return;

	WaitCursor wait;

	// Put the selected item one lower
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_DELETESTRING, (WPARAM) item, (LPARAM) NULL);
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_INSERTSTRING, (WPARAM) item - 1, (LPARAM) snip->WGetName());
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETITEMDATA, (WPARAM) item - 1, (LPARAM) snip);
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETCURSEL, (WPARAM) item - 1, (LPARAM) NULL);

	UpdateSortInDB();
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnSnippetMoveDown(HWND hWnd)
{
	// Just in case
	if (s_curLibrary->GetSortAlphabetic())
		return;

	// Get the current item and its snippet
	int item = (int) SendDlgItemMessage(hWnd, IDC_LIST, LB_GETCURSEL, 0, 0L);

	// Was there an item selected?
	if (item == LB_ERR)
		return;

	// Are we at the end of the list?
	int count = (int) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETCOUNT, 0, 0);
	if (item == count - 1)
		return;

	// Get the data for this item
	Snippet* snip = (Snippet*) SendDlgItemMessage(s_hDlg, IDC_LIST, LB_GETITEMDATA, (WPARAM) item, 0);
	if (snip == NULL)
		return;

	WaitCursor wait;

	// Put the selected item one higher
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_DELETESTRING, (WPARAM) item, (LPARAM) NULL);
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_INSERTSTRING, (WPARAM) item + 1, (LPARAM) snip->WGetName());
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETITEMDATA, (WPARAM) item + 1, (LPARAM) snip);
	SendDlgItemMessage(s_hDlg, IDC_LIST, LB_SETCURSEL, (WPARAM) item + 1, (LPARAM) NULL);

	UpdateSortInDB();
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnLibraryNew(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	Library* lib = new Library();
	if (!ShowEditLibraryDlg(lib))
	{
		delete lib;
		return;
	}

	long item = (long) SendDlgItemMessage(s_hDlg, IDC_NAME, CB_ADDSTRING, (WPARAM) 0, (LPARAM) lib->WGetName());
	SendDlgItemMessage(s_hDlg, IDC_NAME, CB_SETITEMDATA, (WPARAM) item, (LPARAM) lib);

	lib->AddLanguageToDB(s_curLang);
}

/////////////////////////////////////////////////////////////////////////////
// Edit the currently selected library

static void OnLibraryEdit(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	bool oldSortAlphabetic = s_curLibrary->GetSortAlphabetic();
	if (ShowEditLibraryDlg(s_curLibrary))
	{
/*
		bool newSortAlphabetic = s_curLibrary->GetSortAlphabetic();
		if (oldSortAlphabetic != newSortAlphabetic)
		{
			if (OpenDB())
			{
				if (newSortAlphabetic)
				{
					//RunSQL("UPDATE Snippets SET Sort = NULL WHERE LibraryID = %d");
				}
				else
				{
					//"SELECT SnippetID, Sort FROM Snippets WHERE LibraryID = %d ORDER BY Name, Sort"
					i = 1;
					//UPDATE Snippets SET Sort = %d WHERE SnippetID = %d
				}
			}
		}
*/
		UpdateSnippetsList();
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnLibraryDelete(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	WCHAR wszTmp[MAX_PATH];
	wcsncpy(wszTmp, L"Are you sure you want to delete the library:\r\n", MAX_PATH);
	wcsncat(wszTmp, s_curLibrary->WGetName(), MAX_PATH);

	// Ask for confirmation
	if (!MsgBoxYesNo(wszTmp))
		return;

	// Delete from the database
	if (!s_curLibrary->DeleteFromDB())
		return;

	// Fill the snippets again
	UpdateSnippetsList();
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnLibraryLanguages(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	// Check if we have a version of Notepad++ that supports NPPM_GETLANGUAGENAME
	if (!g_HasLangMsgs)
	{
		MsgBox("Editing the languages requires at least version 5.93 of Notepad++");
		return;
	}

	if (ShowEditLanguagesDlg(s_curLibrary))
		UpdateSnippetsList();
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnLibraryImport(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	// Init for GetOpenFileName()
	WCHAR szFile[_MAX_PATH];       // buffer for file name
	szFile[0] = 0;

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner =  g_nppData._nppHandle;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFilter = L"SQLite databases (*.sqlite)\0*.sqlite\0All Files (*.*)\0*.*\0\0";
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFile = szFile;

	// Ask for the filename
	if (GetOpenFileName(&ofn))
	{
		if (ImportLibraryDlg(ofn.lpstrFile))
		{
			// Fill the snippets again
			UpdateSnippetsList();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnLibraryExport(HWND hWnd)
{
	UNREFERENCED_PARAMETER(hWnd);

	// We need space to store the filename
	WCHAR szFileName[MAX_PATH];
	ZeroMemory(szFileName, MAX_PATH);

	// Initialize the FileSave dialog
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner =  g_nppData._nppHandle;
	ofn.Flags = OFN_HIDEREADONLY;
	ofn.lpstrFilter = L"SQLite databases (*.sqlite)\0*.sqlite\0All Files (*.*)\0*.*\0\0";
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrFile = szFileName;
	ofn.lpstrDefExt = L"sqlite";

	// Did we get a filename from the user?
	if (GetSaveFileName(&ofn))
		s_curLibrary->ExportTo(ofn.lpstrFile);
}

/////////////////////////////////////////////////////////////////////////////
// Double clicking the item inserts it as well

static void OnDblClk_List(HWND hWnd)
{
	OnSnippetInsert(hWnd);
}

/////////////////////////////////////////////////////////////////////////////
//

static void OnCommand(HWND hWnd, int ResID, int msg)
{
	switch (ResID)
	{
		case IDC_LIST:
		{
			switch (msg)
			{
				case LBN_DBLCLK:
					OnDblClk_List(hWnd);
					break;
			}
			break;
		}
		case IDC_NAME:
		{
			switch (msg)
			{
				case CBN_SELCHANGE:
					OnSelChange_Combo(hWnd);
					break;
			}
			break;
		}
		case IDC_SNIPPET_INSERT:
		{
			OnSnippetInsert(hWnd);
			break;
		}
		case IDC_SNIPPET_ADD:
		{
			OnSnippetAdd(hWnd);
			break;
		}
		case IDC_SNIPPET_EDIT:
		{
			OnSnippetEdit(hWnd);
			break;
		}
		case IDC_SNIPPET_DELETE:
		{
			OnSnippetDelete(hWnd);
			break;
		}
		case IDC_SNIPPET_DUPLICATE:
		{
			OnSnippetDuplicate(hWnd);
			break;
		}
		case IDC_SNIPPET_MOVE_UP:
		{
			OnSnippetMoveUp(hWnd);
			break;
		}
		case IDC_SNIPPET_MOVE_DOWN:
		{
			OnSnippetMoveDown(hWnd);
			break;
		}
		case IDC_SNIPPET_ADD_CLIPBOARD:
		{
			OnSnippetAddClipboard(hWnd);
			break;
		}
		case IDC_SNIPPET_ADD_SELECTION:
		{
			OnSnippetAddSelection(hWnd);
			break;
		}
		case IDC_SNIPPET_COPY_TO_LIB:
		case IDC_SNIPPET_PASTE_FROM_LIB:
		{
			break;
		}
		case IDC_LIB_NEW:
		{
			OnLibraryNew(hWnd);
			break;
		}
		case IDC_LIB_EDIT:
		{
			OnLibraryEdit(hWnd);
			break;
		}
		case IDC_LIB_DELETE:
		{
			OnLibraryDelete(hWnd);
			break;
		}
		case IDC_LIB_LANGUAGES:
		{
			OnLibraryLanguages(hWnd);
			break;
		}
		case IDC_LIB_IMPORT:
		{
			OnLibraryImport(hWnd);
			break;
		}
		case IDC_LIB_EXPORT:
		{
			OnLibraryExport(hWnd);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// On a theme changes, redraw the listbox so it matches the theme

void InvalidateListbox()
{
	s_hbrBkgnd = NULL;
	InvalidateRect(s_hList, NULL, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
//

static HBRUSH OnCtlColorListbox(HWND hWnd, HWND hwndList, HDC hdc)
{
	UNREFERENCED_PARAMETER(hWnd);

	if (hwndList != s_hList)
		return NULL;

	// Get the colors from the N++ theme
	int fore = (int) SendMsg(SCI_STYLEGETFORE, (WPARAM) STYLE_DEFAULT);
	int back = (int) SendMsg(SCI_STYLEGETBACK, (WPARAM) STYLE_DEFAULT);

	// Set the colors for the items
	SetTextColor(hdc, fore);
	SetBkColor(hdc, back);

	// Set the brush to paint the background of the listbox
	if (s_hbrBkgnd == NULL)
		s_hbrBkgnd = CreateSolidBrush(back);
	return s_hbrBkgnd;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL CALLBACK DlgProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			return OnInitDialog(hWnd);
/*
#pragma warning (push)
#pragma warning (disable : 4311 4312 )
			DefaultMessageEditWindowProc = (WNDPROC) SetWindowLongPtr(hOutputEdit,GWLP_WNDPROC,(LONG)&MessageEditWindowProc);
#pragma warning (pop)
			bool threadSuccess = StartThread(outputProc, (LPVOID)lParam, "outputProc");
			if (!threadSuccess)
			{
				err(TEXT("Error: could not create outputProc thread!"));
			}
*/
		}

		case WM_COMMAND:
		{
			OnCommand(hWnd, (int) LOWORD(wParam), (int) HIWORD(wParam));
			break;
		}

		case WM_SIZE:
		{
			OnSize(hWnd, (int) LOWORD(lParam), (int) HIWORD(lParam));
			break;
		}

		case WM_CONTEXTMENU:
		{
			OnContextMenu(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (HWND) wParam);
			break;
		}

		case WM_CLOSE:
		{
			OnClose(hWnd);
			g_Options->showConsoleDlg = s_bConsoleVisible;
			break;
		}

		case WM_CTLCOLORLISTBOX:
			return (UINT_PTR) OnCtlColorListbox(hWnd, (HWND) lParam, (HDC) wParam);

	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Entry point of the "Snippets Console" menu entry

void SnippetsConsole()
{
	if (s_bConsoleVisible)
	{
		// Hide the window and uncheck the menu item
		SendMessage(g_nppData._nppHandle, NPPM_DMMHIDE, 0, (LPARAM) s_hDlg);
		SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM) g_funcItem[0]._cmdID, (LPARAM) FALSE);

		// The console is not visible anymore
		s_bConsoleVisible = false;
	}
	else
	{
		// The console window will become visible.
		// Set it now already so other routines work properly
		s_bConsoleVisible = true;

		if (!s_bConsoleInitialized)
		{
			// Get the right database filename
			if (!GetDatabaseFile())
			{
				MsgBox("Unable to find the database, check your installation!");
				return;
			}
			
			// Check if we have a version of Notepad++ that supports NPPM_GETLANGUAGENAME
			DWORD ver = (DWORD) SendMessage(g_nppData._nppHandle, NPPM_GETNPPVERSION, (WPARAM) 0, (LPARAM) 0);
			g_HasLangMsgs = (ver >= MAKELONG(93, 5));
			
			// Load the icon
			s_hTabIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SNIPPETS), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_COLOR | LR_LOADTRANSPARENT);

			// Initialize everything for the console window
			tTbData tbd;
			ZeroMemory(&tbd, sizeof(tTbData));
			tbd.dlgID = -1;									// Nr of menu item to assign (!= _cmdID, beware)
			tbd.pszModuleName = L"Snippets";				// name of the dll this dialog belongs to
			tbd.pszName = L"Snippets";						// Name for titlebar
			tbd.hClient = s_hDlg;							// HWND Handle of window this dock belongs to
			tbd.uMask = DWS_DF_CONT_RIGHT | DWS_ICONTAB;	// Put it on the right
			tbd.hIconTab = s_hTabIcon;						// Put the icon in
			SendMessage(g_nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM) &tbd);	// Register it

			// It should initialize now
			s_bConsoleInitialized = true;
		}

		// Put the items in the combo and the list
		UpdateSnippetsList();

		// Show the window and check the menu item
		SendMessage(g_nppData._nppHandle, NPPM_DMMSHOW, 0, (LPARAM) s_hDlg);
		SendMessage(g_nppData._nppHandle, NPPM_SETMENUITEMCHECK, (WPARAM) g_funcItem[0]._cmdID, (LPARAM) TRUE);

		// Set the focus back on the main window
		SetFocusOnEditor();
	}

	// Store the visiblity in the options
	g_Options->showConsoleDlg = s_bConsoleVisible;
}

/////////////////////////////////////////////////////////////////////////////
//

void CreateConsoleDlg()
{
	// Create the console window
	s_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_SNIPPETS), g_nppData._nppHandle, (DLGPROC) DlgProcedure);
}
