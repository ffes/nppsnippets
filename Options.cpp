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
#include "Options.h"
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"
#include "version_git.h"

/////////////////////////////////////////////////////////////////////////////
// Strings used in the ini file

static WCHAR s_szOptions[]			= L"Options";
static WCHAR s_szShow[]				= L"Show";
static WCHAR s_szToolbarIcon[]		= L"ToolbarIcon";
static WCHAR s_szIndent[]			= L"Indent";
static WCHAR s_szVersion[]			= L"Version";
static WCHAR s_szDBFile[]			= L"DBFile";

/////////////////////////////////////////////////////////////////////////////
// Constructor: read the settings

Options::Options() noexcept : NppOptions()
{
	// Read the settings from the file
	Read();
}

Options::Options(std::wstring filename) : NppOptions(filename)
{
	// Read the settings from the file
	Read();
}

/////////////////////////////////////////////////////////////////////////////
// Destructor: write the settings

Options::~Options()
{
	Write();
}

/////////////////////////////////////////////////////////////////////////////
// Write the options to the ini-file

void Options::Write()
{
	WriteBool(s_szOptions, s_szShow, _showConsoleDlg);
	WriteBool(s_szOptions, s_szToolbarIcon, _toolbarIcon);
	WriteBool(s_szOptions, s_szIndent, _indentSnippet);
	WriteString(s_szOptions, s_szDBFile, _DBFile.c_str());
	WriteString(s_szOptions, s_szVersion, VERSION_NUMBER_WSTR);
}

/////////////////////////////////////////////////////////////////////////////
// Read the options from the ini-file

void Options::Read()
{
	_showConsoleDlg = GetBool(s_szOptions, s_szShow, true);
	_toolbarIcon = GetBool(s_szOptions, s_szToolbarIcon, true);
	_indentSnippet = GetBool(s_szOptions, s_szIndent, true);
	_prevVersion = GetString(s_szOptions, s_szVersion, L"");
	_DBFile = GetString(s_szOptions, s_szDBFile, L"");
}
