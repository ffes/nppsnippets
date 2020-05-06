/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010-2016 Frank Fesevur                                  //
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

#pragma once

#include <string>

extern HWND getCurrentHScintilla(int which);
extern LRESULT SendMsg(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0, int count = 1);
extern void MsgBox(const WCHAR* msg);
extern void MsgBox(const char* msg);
extern bool MsgBoxYesNo(const WCHAR* msg);
extern void CenterWindow(HWND hDlg);
extern std::wstring GetDlgText(HWND hDlg, UINT uID);
extern int GetDlgTextLength(HWND hDlg, UINT uID);
extern std::wstring ConvertLineEnding(LPCWSTR from, int toLineEnding = -1);

struct NppData;
struct FuncItem;
enum LangType;

extern HINSTANCE g_hInst;
extern NppData g_nppData;
extern LangType g_currentLang;
extern FuncItem g_funcItem[];
extern bool g_HasLangMsgs;

class Options;
extern Options *g_Options;
