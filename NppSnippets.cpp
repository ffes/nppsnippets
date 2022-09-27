/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010-2020 Frank Fesevur                                  //
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
#include <assert.h>
#include <commctrl.h>

#include "NPP/PluginInterface.h"
#include "Resource.h"
#include "NppSnippets.h"
#include "DlgAbout.h"
#include "DlgConsole.h"
#include "DlgOptions.h"
#include "Options.h"
#include "SnippetsDB.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

static const TCHAR PLUGIN_NAME[] = L"Snippets";
static const int nbFunc = 11;
static HBITMAP hbmpToolbar = NULL;

HINSTANCE g_hInst;
NppData g_nppData;
LangType g_currentLang = L_TEXT;		// Current language of the N++ text window
FuncItem g_funcItem[nbFunc];
Options *g_Options = NULL;
bool g_HasLangMsgs = false;

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	g_nppData = notpadPlusData;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) const TCHAR* getName()
{
	return PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return g_funcItem;
}

/////////////////////////////////////////////////////////////////////////////
//

HWND getCurrentScintilla()
{
	int current = -1;
	SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&current);
	return (current == 0) ? g_nppData._scintillaMainHandle : g_nppData._scintillaSecondHandle;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C" __declspec(dllexport) void beNotified(SCNotification* notifyCode)
{
	switch (notifyCode->nmhdr.code)
	{
		case NPPN_READY:
		{
			// Initialize the database
			g_db = new SnippetsDB();

			if (g_Options->GetShowConsoleDlg())
				SnippetsConsole();
			break;
		}

		case NPPN_SHUTDOWN:
		{
			break;
		}

		case NPPN_TBMODIFICATION:
		{
			// First initialize the options
			g_Options = new Options(L"NppSnippets.ini");

			// Do we need to load the toolbar icon?
			if (g_Options->GetToolbarIcon())
			{
				// Add the button to the toolbar
				toolbarIconsWithDarkMode tbiFolder;
				hbmpToolbar = CreateMappedBitmap(g_hInst, IDB_SNIPPETS, 0, 0, 0);
				tbiFolder.hToolbarBmp = hbmpToolbar;
				tbiFolder.hToolbarIcon = NULL;
				tbiFolder.hToolbarIconDarkMode = NULL;
				SendMessage((HWND) notifyCode->nmhdr.hwndFrom, NPPM_ADDTOOLBARICON_FORDARKMODE, (WPARAM) g_funcItem[0]._cmdID, (LPARAM) &tbiFolder);
			}
			break;
		}

		case NPPN_LANGCHANGED:
		case NPPN_BUFFERACTIVATED:
		{
			LangType lang;
			SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM) &lang);
			if (g_currentLang != lang)
			{
				g_currentLang = lang;
				UpdateSnippetsList();
			}
			break;
		}

		case NPPN_WORDSTYLESUPDATED:
		{
			InvalidateListbox();
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Here you can process the Npp Messages
// I will make the messages accessible little by little, according to the
// need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781

extern "C" __declspec(dllexport) LRESULT messageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

/*
	if (uMsg == WM_MOVE)
	{
		MsgBox("move");
	}
*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// VERY simple LF -> CRLF conversion

std::wstring ConvertLineEnding(LPCWSTR from, int toLineEnding)
{
	// Is there a string anyway?
	std::wstring to;
	if (from == NULL)
		return to;

	// Get the line ending of the current document
	if (toLineEnding < 0)
		toLineEnding = (int) SendMsg(SCI_GETEOLMODE);

	// Determine the new line ending
	std::wstring lineend = L"\r\n";
	switch (toLineEnding)
	{
		case SC_EOL_CRLF:
			lineend = L"\r\n";
			break;
		case SC_EOL_CR:
			lineend = L"\r";
			break;
		case SC_EOL_LF:
			lineend = L"\n";
			break;
	}

	// Iterate through the text we were given
	size_t len = wcslen(from);
	for (size_t i = 0; i < len; i++)
	{
		if (from[i] == '\r')
		{
			if (i + 1 < len)
			{
				// is the next a \n, skip it
				if (from[i + 1] == '\n')
					i++;

				// Add the new line ending
				to += lineend;
				continue;
			}
		}

		if (from[i] == '\n')
		{
			to += lineend;
			continue;
		}

		to += from[i];
	}

	return to;
}

/////////////////////////////////////////////////////////////////////////////
// Easy access to the MessageBox functions

void MsgBox(const WCHAR* msg)
{
	MessageBox(g_nppData._nppHandle, msg, PLUGIN_NAME, MB_OK);
}

void MsgBox(const char* msg)
{
	std::wstring wsTmp(msg, msg + strlen(msg));
	MessageBox(g_nppData._nppHandle, wsTmp.c_str(), PLUGIN_NAME, MB_OK);
}

bool MsgBoxYesNo(const WCHAR* msg)
{
	return (MessageBox(g_nppData._nppHandle, msg, PLUGIN_NAME, MB_YESNO) == IDYES);
}

/////////////////////////////////////////////////////////////////////////////
// Send a simple message to the Notepad++ window 'count' times and return
// the last result.

LRESULT SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam, int count)
{
	LRESULT res = 0;
	for (int i = 0; i < count; i++)
		res = SendMessage(getCurrentScintilla(), Msg, wParam, lParam);
	return res;
}

/////////////////////////////////////////////////////////////////////////////
// Make the window center, relative the NPP-window

void CenterWindow(HWND hDlg)
{
	RECT rc;
	GetClientRect(g_nppData._nppHandle, &rc);

	POINT center;
	center.x = rc.left + ((rc.right - rc.left) / 2);
	center.y = rc.top + ((rc.bottom - rc.top) / 2);
	ClientToScreen(g_nppData._nppHandle, &center);

	RECT dlgRect;
	GetClientRect(hDlg, &dlgRect);
	const int x = center.x - ((dlgRect.right - dlgRect.left) / 2);
	const int y = center.y - ((dlgRect.bottom - dlgRect.top) / 2);

	SetWindowPos(hDlg, HWND_TOP, x, y, -1, -1, SWP_NOSIZE | SWP_SHOWWINDOW);
}

/////////////////////////////////////////////////////////////////////////////
//

int GetDlgTextLength(HWND hDlg, UINT uID)
{
	return GetWindowTextLength(GetDlgItem(hDlg, uID));
}

/////////////////////////////////////////////////////////////////////////////
//

std::wstring GetDlgText(HWND hDlg, UINT uID)
{
	// Allocate the (temporary) memory needed to store the text
	int maxBufferSize = GetDlgTextLength(hDlg, uID) + 3;
	WCHAR* buffer = new WCHAR[maxBufferSize];
	ZeroMemory(buffer, maxBufferSize);

	// Get the text from the dialog item
	GetDlgItemText(hDlg, uID, buffer, maxBufferSize);
	std::wstring ret = buffer;
	delete buffer;

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Open the on-line manual

static void OpenOnlineManual()
{
	ShellExecute(NULL, L"open", L"https://nppsnippets.readthedocs.io", NULL, NULL, SW_SHOWNORMAL);
}

/////////////////////////////////////////////////////////////////////////////
// The entry point of the DLL

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (reasonForCall)
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hInst = (HINSTANCE) hModule;

			// The most important menu entry
			int index = 0;
			g_funcItem[index]._pFunc = SnippetsConsole;
			wcscpy(g_funcItem[index]._itemName, L"Snippets");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Seperator
			g_funcItem[index]._pFunc = NULL;
			wcscpy(g_funcItem[index]._itemName, L"-SEPARATOR-");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Focus the items in the snippets panel
			g_funcItem[index]._pFunc = FocusLibraryCombo;
			wcscpy(g_funcItem[index]._itemName, L"Focus List of Libraries");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			g_funcItem[index]._pFunc = FocusFilterSnippets;
			wcscpy(g_funcItem[index]._itemName, L"Focus Filter Snippets");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			g_funcItem[index]._pFunc = FocusSnippetsList;
			wcscpy(g_funcItem[index]._itemName, L"Focus List of Snippets");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Seperator
			g_funcItem[index]._pFunc = NULL;
			wcscpy(g_funcItem[index]._itemName, L"-SEPARATOR-");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Open Options dialog
			g_funcItem[index]._pFunc = ShowOptionsDlg;
			wcscpy(g_funcItem[index]._itemName, L"Options...");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Seperator
			g_funcItem[index]._pFunc = NULL;
			wcscpy(g_funcItem[index]._itemName, L"-SEPARATOR-");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Open Online Manual
			g_funcItem[index]._pFunc = OpenOnlineManual;
			wcscpy(g_funcItem[index]._itemName, L"Open Online Manual");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Seperator
			g_funcItem[index]._pFunc = NULL;
			wcscpy(g_funcItem[index]._itemName, L"-SEPARATOR-");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;

			// Show About Dialog
			g_funcItem[index]._pFunc = ShowAboutDlg;
			wcscpy(g_funcItem[index]._itemName, L"About...");
			g_funcItem[index]._init2Check = false;
			g_funcItem[index]._pShKey = NULL;
			index++;
			assert(index == nbFunc);

			// Create the console dialog
			CreateConsoleDlg();
		}
		break;

		case DLL_PROCESS_DETACH:
		{
			// Don't forget to deallocate your shortcut here
			delete g_funcItem[0]._pShKey;

			// Delete the toolbar bitmap
			if (hbmpToolbar != NULL)
				DeleteObject(hbmpToolbar);

			// Clean up the options
			delete g_Options;

			// Clean up the database
			delete g_db;
		}
		break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}
