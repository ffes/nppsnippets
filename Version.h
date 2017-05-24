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

#pragma once

#define VERSION_NUMBER        1,4,0,0
#define VERSION_NUMBER_STR   "1.4.0"
#define VERSION_NUMBER_WSTR L"1.4.0"
#define COPYRIGHT_STR        "Copyright 2010-2017 by Frank Fesevur"

#include "version_git.h"

#define VERSION_DIGITS 4

class Version
{
public:
	Version();
	Version(BYTE version[VERSION_DIGITS]);
	Version(LPCWSTR version);
	Version(const Version&);

	// Overload the basic compare operators
	bool operator==(const Version) const;
	bool operator!=(const Version chk) const  { return !(*this == chk); };
	bool operator<(const Version) const;
	bool operator>(const Version) const;

private:
	BYTE _version[VERSION_DIGITS];
};
