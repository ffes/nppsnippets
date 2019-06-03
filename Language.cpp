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
#include "Language.h"

/////////////////////////////////////////////////////////////////////////////
//

Language::Language(int langid)
{
	_LangID = langid;

	ZeroMemory(&_LangName, MAX_PATH);
	SendMessage(g_nppData._nppHandle, NPPM_GETLANGUAGENAME, langid, (LPARAM) _LangName);

	ZeroMemory(&_LangDescr, MAX_PATH);
	SendMessage(g_nppData._nppHandle, NPPM_GETLANGUAGEDESC, langid, (LPARAM) _LangDescr);
}

Language::Language(int langid, LPCWSTR langName, LPCWSTR langDescr)
{
	_LangID = langid;
	wcsncpy(_LangName, langName, MAX_PATH);
	wcsncpy(_LangDescr, langDescr, MAX_PATH);
}
