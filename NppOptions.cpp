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
#include "NppOptions.h"
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor

NppOptions::NppOptions() noexcept
{
	// Get the directory from NP++ and add the filename of the settings file
	WCHAR szPath[_MAX_PATH];
	szPath[0] = 0;
	SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)&szPath);

	_IniPath = szPath;
	_IniPath += L"\\";
	_IniPath += getName();
	_IniPath += L".ini";
}

//////////////////////////////////////////////////////////////////////////////
// Constructor with .ini filename

NppOptions::NppOptions(std::wstring filename)
{
	// Get the directory from NP++ and add the filename of the settings file
	WCHAR szPath[_MAX_PATH];
	szPath[0] = 0;
	SendMessage(g_nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)&szPath);

	_IniPath = szPath;
	_IniPath += L"\\";
	_IniPath += filename;
}

/////////////////////////////////////////////////////////////////////////////
// Read a boolean from the ini file

bool NppOptions::GetBool(const WCHAR* szAppName, const WCHAR* szKeyName, const bool def) noexcept
{
	return(GetPrivateProfileInt(szAppName, szKeyName, def ? 1 : 0, _IniPath.c_str()) > 0);
}

/////////////////////////////////////////////////////////////////////////////
// Read a int from the ini file

int NppOptions::GetInt(const WCHAR* szAppName, const WCHAR* szKeyName, const int def) noexcept
{
	return GetPrivateProfileInt(szAppName, szKeyName, def, _IniPath.c_str());
}

/////////////////////////////////////////////////////////////////////////////
// Read a string from the ini file

std::wstring NppOptions::GetString(const WCHAR* szAppName, const WCHAR* szKeyName, const WCHAR* def) noexcept
{
	WCHAR szRet[_MAX_PATH];
	GetPrivateProfileString(szAppName, szKeyName, def, szRet, _MAX_PATH, _IniPath.c_str());
	return szRet;
}

/////////////////////////////////////////////////////////////////////////////
// Write a boolean to the ini file

void NppOptions::WriteBool(const WCHAR* szAppName, const WCHAR* szKeyName, const bool val) noexcept
{
	WritePrivateProfileString(szAppName, szKeyName, val ? L"1" : L"0", _IniPath.c_str());
}

/////////////////////////////////////////////////////////////////////////////
// Write an integer to the ini file

void NppOptions::WriteInt(const WCHAR* szAppName, const WCHAR* szKeyName, const int val) noexcept
{
	WCHAR temp[256];
	swprintf(temp, _countof(temp), L"%d", val);
	WritePrivateProfileString(szAppName, szKeyName, temp, _IniPath.c_str());
}

/////////////////////////////////////////////////////////////////////////////
// Write a string to the ini file

void NppOptions::WriteString(const WCHAR* szAppName, const WCHAR* szKeyName, const WCHAR* val) noexcept
{
	WritePrivateProfileString(szAppName, szKeyName, val, _IniPath.c_str());
}
