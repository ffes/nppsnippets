/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010-2017 Frank Fesevur                                  //
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

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	CenterWindow(hDlg);
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
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), g_nppData._nppHandle, (DLGPROC) DlgProc);
}
