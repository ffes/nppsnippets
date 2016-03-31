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

#include <windows.h>
#include <stdio.h>
#include "NppOptions.h"
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor: read the settings

NppOptions::NppOptions()
{
	// First make sure the path is empty
	_szIniPath[0] = 0;

	// Get the directory from NP++ and add the filename of the settings file
	SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM) &_szIniPath);
	wcsncat(_szIniPath, L"\\NppSnippets.ini", MAX_PATH);
}

/////////////////////////////////////////////////////////////////////////////
// Read a boolean from the ini file

bool NppOptions::GetBool(WCHAR* szAppName, WCHAR* szKeyName, bool def)
{
	return(GetPrivateProfileInt(szAppName, szKeyName, def ? 1 : 0, _szIniPath) > 0);
}

/////////////////////////////////////////////////////////////////////////////
// Read a int from the ini file

int NppOptions::GetInt(WCHAR* szAppName, WCHAR* szKeyName, int def)
{
	return GetPrivateProfileInt(szAppName, szKeyName, def, _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Read a string from the ini file

void NppOptions::GetString(WCHAR* szAppName, WCHAR* szKeyName, WCHAR* szReturnedString, DWORD nSize, WCHAR* def)
{
	GetPrivateProfileString(szAppName, szKeyName, def, szReturnedString, nSize, _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Write a boolean to the ini file

void NppOptions::WriteBool(WCHAR* szAppName, WCHAR* szKeyName, bool val)
{
	WritePrivateProfileString(szAppName, szKeyName, val ? L"1" : L"0", _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Write an integer to the ini file

void NppOptions::WriteInt(WCHAR* szAppName, WCHAR* szKeyName, int val)
{
	WCHAR temp[256];
	snwprintf(temp, 256, L"%d", val);
	WritePrivateProfileString(szAppName, szKeyName, temp, _szIniPath);
}

/////////////////////////////////////////////////////////////////////////////
// Write a string to the ini file

void NppOptions::WriteString(WCHAR* szAppName, WCHAR* szKeyName, WCHAR* val)
{
	WritePrivateProfileString(szAppName, szKeyName, val, _szIniPath);
}
