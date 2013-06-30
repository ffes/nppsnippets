/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010-2013 Frank Fesevur                                  //
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
#include <string.h>
#include <commctrl.h>
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"
#include "Resource.h"
#include "Version.h"

/////////////////////////////////////////////////////////////////////////////
//

struct VersionInfo
{
	BYTE	version[VERSION_DIGITS];
	int		date[3];
	WCHAR*	text;
};

#define MAX_VERSION_INFO 11

static VersionInfo s_info[MAX_VERSION_INFO] =
{
	{	{1,3,0,0},	{2013, 6,30},	L"- Fixed problem with inserting UTF snippets.\n- Fixed wrong title of Import Library dialog.\n- Fixed some potential bugs found when trying to fix GCC compilation.\n- Converted the documentation from ODT to DocBook.\n- Upgrade to SQLite version 3.7.17" },
	{	{1,2,0,0},	{2013, 1, 8},	L"- There was an inconsistency between the documentation and code about the name of the option to specify your custom path for the database. Use DBFile from now on. For backwards compatibility the DBPath entry will still be recognized.\n- When a snippets creates a new document and the current document is empty, it reuses the current one and does not start a new.\n- Added Duplicate snippet function to context menu.\n- New (simple) templates library.\n- Upgrade to SQLite version 3.7.15.1" },
	{	{1,1,0,0},	{2012, 1, 2},	L"- You can now add a new snippet to a library based upon the current selection or based upon the content of the clipboard.\n- Installation has been improved. A template database is provided and when the plugin tries to find the database and it can't find it, it copies this template database to the AppData plugin-config directory.\n- The About dialog now shows the changelog.\n- When you upgrade the very first time the changelog for the current version will be shown.\n- When you didn't select a specific library for a certain language, the automatic selection of the library is improved. The first language specific library is preferred over the first general library.\n- Resized the edit snippet dialog.\n- Upgrade to SQLite version 3.7.9" },
	{	{1,0,0,0},	{2011, 9, 6},	L"- The selection or cursor position are now restored after inserting a snippet." },
	{	{0,7,1,0},	{2011, 8,28},	L"- Fixed a bug in the dialog to edit the languages for a certain library. This bug could cause a problem that libraries turn invisible, since all the records in LibraryLang table for that library were deleted and no new records were added.\n- Added a JavaScript - Math library." },		
	{	{0,7,0,0},	{2011, 8, 1},	L"- A user interface for editing the language selection for libraries has been added. You need at least Notepad++ version 5.93 for this feature.\n- You can import a library from another NppSnippet database.\n- Start a new document for a certain snippets, and allow that snippet to set the language of that new document. There were already fields in the database for this. It can be very useful to start a new CSS-file or JavaScript-file from HTML, etc.\n- Added an option DBPath to the ini-file to override the default location of the database. Made this mainly for my own testing, but maybe it is useful for others as well (corporate database). You need to manually edit the ini-file to use this.\n- Added an icon to the tab of the docking interface.\n- Upgrade to SQLite version 3.7.7.1" },
	{	{0,6,0,0},	{2011, 6,15},	L"- It is now possible to add, edit or delete the snippets and the libraries from within Notepad++. It is not yet possible to edit the languages for a library.\n- Added a new ANSI-characters library for all languages.\n- Deleted the useless General library.\n- Upgrade to SQLite version 3.7.6.3" },
	{	{0,5,0,0},	{2010,12,21},	L"- Upgrade to SQLite version 3.7.4\n- The focus is returned to the Scintilla window at start-up and after inserting a snippet.\n- The plug-in remembers if it is shown.\n- Added another special language to the table LibraryLang: Lang = -2. Libraries with this language will always be shown for all languages.\n- The languages last used is now stored in a separate table. The database schema version is 3. With this the libraries with special languages (negative language ID's) can be remembered as last used as well.\n- Error message when the database can not be opened." },
	{	{0,4,0,0},	{2010, 4, 8},	L"- The database schema is updated and is now at version 2. The most important difference is that the \"language\" and \"last used\" field of the library are now in a separate table, allowing it to be one-to-many. Existing databases will be converted automatically to the new schema.\n- When there are multiple libraries for a language, changing to another library works and the last used library is remembered.\n- The plug-in now first tries to find the database in the user's plug-in config directory. On my Windows XP machine that is C:\\Documents and Settings\\Frank\\Application Data\\Notepad++\\plugins\\config. If the database can't be found there it looks in the plugin\\config directory in the Notepad++ installation directory, in my case C:\\Program Files\\Notepad++\\plugins\\Config.\n- Small improvements to the About dialog.\n- There are now 4 HTML libraries, 1 PHP libraries, 1 XML library and 1 (rather useless) General library" },
	{	{0,3,0,0},	{2011, 2,10},	L"- First alpha version, released under the GPL2 license.\n- The basics work, no User Interface yet to edit the snippets" },
	{	{0,1,0,0},	{2010, 1,22},	L"- Internal proof of concept." }
};

static int s_showTill = MAX_VERSION_INFO;

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	CenterWindow(hDlg);

	// Show the relevant part of the changelog
	wstring txt;
	WCHAR szTmp[_MAX_PATH];
	for (int i = 0; i < s_showTill; i++)
	{
		if (!txt.empty())
			txt += L"\n\n";

		// Add the version number
		txt += L"Version ";
		snwprintf(szTmp, _MAX_PATH, L"%d.%d.%d", s_info[i].version[0], s_info[i].version[1], s_info[i].version[2]);
		txt += szTmp;

		// Add the release date
		struct tm released;
		ZeroMemory(&released, sizeof(tm));
		released.tm_year = s_info[i].date[0] - 1900;
		released.tm_mon = s_info[i].date[1] - 1;
		released.tm_mday = s_info[i].date[2];

		txt += L", released on ";
		wcsftime(szTmp,_MAX_PATH, L"%d-%b-%Y", &released);
		txt += szTmp;
		txt += L"\n";

		// Add the changelog
		txt += s_info[i].text;
	}
	txt = ConvertNewLines(txt.c_str());
	SetDlgItemText(hDlg, IDC_CHANGELOG, txt.c_str());

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
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{
				case NM_CLICK:
				case NM_RETURN:
				{
					PNMLINK pNMLink = (PNMLINK) lParam;
					LITEM item = pNMLink->item;
					ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
				}
			}
			return FALSE;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
				{
					EndDialog(hDlg, 0);
					return TRUE;
				}
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Show the About Dialog, with all version information

void ShowAboutDlg()
{
	s_showTill = MAX_VERSION_INFO;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), g_nppData._nppHandle, (DLGPROC) DlgProc);
}

/////////////////////////////////////////////////////////////////////////////
// Show the About Dialog, with version information until 'prevVer'

void ShowAboutDlgVersion(Version prevVer)
{
	s_showTill = MAX_VERSION_INFO;
	for (int i = 0; i < MAX_VERSION_INFO; i++)
	{
		Version ver(s_info[i].version);
		if (ver == prevVer)
		{
			s_showTill = i;
			break;
		}
	}

	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), g_nppData._nppHandle, (DLGPROC) DlgProc);
}
