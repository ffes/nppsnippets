/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2010 Frank Fesevur                                       //
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
#include "Version.h"

/////////////////////////////////////////////////////////////////////////////
// Strings used in the ini file

static WCHAR s_szOptions[]			= L"Options";
static WCHAR s_szShow[]				= L"Show";
static WCHAR s_szToolbarIcon[]		= L"ToolbarIcon";
static WCHAR s_szVersion[]			= L"Version";
static WCHAR s_szDBPath[]			= L"DBPath";
static WCHAR s_szDBFile[]			= L"DBFile";

/////////////////////////////////////////////////////////////////////////////
// Constructor: read the settings

Options::Options()
{
	// First make sure the paths are empty
	_szIniPath[0] = 0;
	_szPrevVersion[0] = 0;
	_szDBFile[0] = 0;

	// Get the directory from NP++ and add the filename of the settings file
	SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM) &_szIniPath);
	wcsncat(_szIniPath, L"\\NppSnippets.ini", MAX_PATH);

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
	WritePrivateProfileString(s_szOptions, s_szShow, (showConsoleDlg ? L"1" : L"0"), _szIniPath);
	WritePrivateProfileString(s_szOptions, s_szToolbarIcon, (toolbarIcon ? L"1" : L"0"), _szIniPath);
	WritePrivateProfileString(s_szOptions, s_szVersion, VERSION_NUMBER_WSTR, _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Read the options from the ini-file

void Options::Read()
{
	showConsoleDlg = (GetPrivateProfileInt(s_szOptions, s_szShow, 1, _szIniPath) > 0);
	toolbarIcon = (GetPrivateProfileInt(s_szOptions, s_szToolbarIcon, 1, _szIniPath) > 0);
	GetPrivateProfileString(s_szOptions, s_szVersion, L"", _szPrevVersion, MAX_PATH,  _szIniPath);

	// Did the user specify a special path for the database?
	GetPrivateProfileString(s_szOptions, s_szDBFile, L"", _szDBFile, MAX_PATH,  _szIniPath);	
	if (wcslen(_szDBFile) == 0)
	{
		// Try the old name of this entry
		GetPrivateProfileString(s_szOptions, s_szDBPath, L"", _szDBFile, MAX_PATH,  _szIniPath);
	}
}
