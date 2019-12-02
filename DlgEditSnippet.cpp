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
#include "Snippets.h"
#include "Language.h"
#include "Resource.h"
#include "WaitCursor.h"

static Snippet* s_pSnippet = NULL;

/////////////////////////////////////////////////////////////////////////////
//

static void CleanItems(HWND hDlg)
{
	// Clean up the ItemData
	int count = (int) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
	for (int item = 0; item < count; item++)
	{
		int* langid = (int*) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_GETITEMDATA, (WPARAM) item, (LPARAM) 0);
		delete langid;
	}

	// Now delete the items from the combobox
	SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
}

/////////////////////////////////////////////////////////////////////////////
//

static bool Validate(HWND hDlg)
{
	// Check if "Name" is entered
	if (GetDlgTextLength(hDlg, IDC_NAME) == 0)
	{
		MsgBox("Name needs to be entered!");
		return false;
	}

	// Check if "Before Selection" is entered
	if (GetDlgTextLength(hDlg, IDC_BEFORE_SEL) == 0)
	{
		MsgBox("Before Selection needs to be entered!");
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

	// Get the new data from the dialog
	using namespace std;
	wstring name = GetDlgText(hDlg, IDC_NAME);
	wstring before = GetDlgText(hDlg, IDC_BEFORE_SEL);
	wstring after = GetDlgText(hDlg, IDC_AFTER_SEL);

	s_pSnippet->WSetName(name.c_str());
	s_pSnippet->WSetBeforeSelection(before.c_str());
	s_pSnippet->WSetAfterSelection(after.c_str());
	s_pSnippet->SetReplaceSelection(IsDlgButtonChecked(hDlg, IDC_REPLACE_SEL) == BST_CHECKED);
	s_pSnippet->SetNewDocument(IsDlgButtonChecked(hDlg, IDC_NEW_DOC) == BST_CHECKED);

	// Get the selected language from the combo
	if (g_HasLangMsgs)
	{
		long item = (long) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
		if (item != CB_ERR)
		{
			int* langid = (int*) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_GETITEMDATA, (WPARAM) item, (LPARAM) 0);
			s_pSnippet->SetNewDocumentLang(*langid);
		}
	}

	// Save the snippet to the database
	try
	{
		WaitCursor wait;
		s_pSnippet->SaveToDB();
	}
	catch (SqliteException ex)
	{
		std::string msg = "Failed to save the snippet to the database!\n\n";
		msg += ex.what();
		MsgBox(msg.c_str());
	}

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

static BOOL OnReplaceSelClick(HWND hDlg)
{
	if (IsDlgButtonChecked(hDlg, IDC_REPLACE_SEL) == BST_CHECKED)
	{
		SetWindowText(GetDlgItem(hDlg, IDC_BEFORE_SEL_TXT), L"&Replace with:");
		SetWindowText(GetDlgItem(hDlg, IDC_AFTER_SEL_TXT), L"&After selection:");
		EnableWindow(GetDlgItem(hDlg, IDC_AFTER_SEL), FALSE);
	}
	else
	{
		SetWindowText(GetDlgItem(hDlg, IDC_BEFORE_SEL_TXT), L"Befo&re cursor:");
		SetWindowText(GetDlgItem(hDlg, IDC_AFTER_SEL_TXT), L"&After cursor:");
		EnableWindow(GetDlgItem(hDlg, IDC_AFTER_SEL), TRUE);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnNewDocumentClick(HWND hDlg)
{
	// Only enable the combo if we have a version of Notepad++ that supports NPPM_GETLANGUAGENAME
	if (g_HasLangMsgs)
		EnableWindow(GetDlgItem(hDlg, IDC_NEW_DOC_LANG), IsDlgButtonChecked(hDlg, IDC_NEW_DOC) == BST_CHECKED);
	else
		EnableWindow(GetDlgItem(hDlg, IDC_NEW_DOC_LANG), FALSE);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

static void FillLanguageCombo(HWND hDlg)
{
	// Check if we have a version of Notepad++ that supports NPPM_GETLANGUAGENAME
	if (!g_HasLangMsgs)
		return;

	WCHAR LangName[MAX_PATH];
	long item = 0;

	// Go through all the available languages and put them in the combo
	for (int langid = L_TEXT; langid <= L_EXTERNAL; langid++)
	{
		if (langid != L_EXTERNAL)
		{
			// Get the language name
			SendMessage(g_nppData._nppHandle, NPPM_GETLANGUAGENAME, langid, (LPARAM) LangName);

			// Add the language to the combo
			item = (long) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_ADDSTRING, (WPARAM) 0, (LPARAM) LangName);
		}
		else
		{
			// Add the language to the combo
			item = (long) SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_ADDSTRING, (WPARAM) 0, (LPARAM) L"Use default");
		}

		int* tmpLangid = new int;
		*tmpLangid = langid;
		SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_SETITEMDATA, (WPARAM) item, (LPARAM) tmpLangid);

		// Need to select the current language?
		if (langid == s_pSnippet->GetNewDocumentLang())
			SendDlgItemMessage(hDlg, IDC_NEW_DOC_LANG, CB_SETCURSEL, (WPARAM) item, (LPARAM) 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
//

static BOOL OnInitDialog(HWND hDlg)
{
	if (s_pSnippet == NULL)
		return TRUE;

	CenterWindow(hDlg);

	SetDlgItemText(hDlg, IDC_NAME, s_pSnippet->WGetName());
	SetDlgItemText(hDlg, IDC_BEFORE_SEL, s_pSnippet->WGetBeforeSelection());
	SetDlgItemText(hDlg, IDC_AFTER_SEL, s_pSnippet->WGetAfterSelection());
	CheckDlgButton(hDlg, IDC_REPLACE_SEL, s_pSnippet->GetReplaceSelection() ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_NEW_DOC, s_pSnippet->GetNewDocument() ? BST_CHECKED : BST_UNCHECKED);

	FillLanguageCombo(hDlg);

	OnReplaceSelClick(hDlg);
	OnNewDocumentClick(hDlg);

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

				case IDC_REPLACE_SEL:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						return OnReplaceSelClick(hDlg);
					}
					break;
				}

				case IDC_NEW_DOC:
				{
					if (HIWORD(wParam) == BN_CLICKED)
					{
						return OnNewDocumentClick(hDlg);
					}
					break;
				}
			}
			return FALSE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Show the Dialog

bool ShowEditSnippetDlg(Snippet* pSnippet)
{
	s_pSnippet = pSnippet;
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_EDIT_SNIPPET), g_nppData._nppHandle, (DLGPROC) DlgProc) == IDOK;
}
