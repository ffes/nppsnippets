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
#include <string.h>
#include "NPP/PluginInterface.h"
#include "NppSnippets.h"
#include "Version.h"
#include "Options.h"

/////////////////////////////////////////////////////////////////////////////
//

Version::Version()
{
	int curVer[VERSION_DIGITS] = { VERSION_NUMBER };
	for (int i = 0; i < VERSION_DIGITS; i++)
		_version[i] = curVer[i];
}

Version::Version(BYTE version[VERSION_DIGITS])
{
	for (int i = 0; i < VERSION_DIGITS; i++)
		_version[i] = version[i];
}

Version::Version(LPCWSTR version)
{
	// Split the version in the seperate numbers
	int v0, v1, v2, v3;
	int numRead = swscanf(version, L"%d.%d.%d.%d", &v0, &v1, &v2, &v3);

	// Copy these numbers to the corresponding member of _version
	_version[0] = (numRead > 0 ? v0 : 0);
	_version[1] = (numRead > 1 ? v1 : 0);
	_version[2] = (numRead > 2 ? v2 : 0);
	_version[3] = (numRead > 3 ? v3 : 0);
}

///////////////////////////////////////////////////////////////////////////
// Copy-Constructor

Version::Version(const Version& copy)
{
	for (int i = 0; i < VERSION_DIGITS; i++)
		_version[i] = copy._version[i];
}

///////////////////////////////////////////////////////////////////////////
// Overload of '==' operator

bool Version::operator==(const Version chkVersion) const
{
	for (int i = 0; i < VERSION_DIGITS; i++)
	{
		if (_version[i] != chkVersion._version[i])
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
// Overload of '>' operator

bool Version::operator>(const Version chkVersion) const
{
	for (int i = 0; i < VERSION_DIGITS; i++)
	{
		if (_version[i] > chkVersion._version[i])
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
// Overload of '<' operator

bool Version::operator<(const Version chkVersion) const
{
	for (int i = 0; i < VERSION_DIGITS; i++)
	{
		if (_version[i] < chkVersion._version[i])
			return true;
	}
	return false;
}
