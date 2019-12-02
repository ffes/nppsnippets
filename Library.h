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

#pragma once

#include "Snippets.h"

/////////////////////////////////////////////////////////////////////////////
//

class Library : public SnippetBase
{
public:
	Library();
	Library(SqliteStatement* stmt);
	virtual ~Library();

	WCHAR* WGetCreatedBy() { return _CreatedBy; }
	WCHAR* WGetComments() { return _Comments; }
	bool GetSortAlphabetic() { return _SortAlphabetic; }

	virtual void Set(SqliteStatement* stmt);

	void WSetCreatedBy(LPCWCH txt);
	void WSetComments(LPCWCH txt);
	void SetSortAlphabetic(bool b) { _SortAlphabetic = b; }
	void SetSortAlphabetic(int i) { _SortAlphabetic = (i == 0); }

	void SaveToDB(bool autoOpen = true) override;
	void DeleteFromDB() override;
	void AddLanguageToDB(int lang);
	void DeleteLanguageFromDB(int lang);
	void ExportTo(LPCWCH filename);

private:
	WCHAR* _CreatedBy;
	WCHAR* _Comments;
	bool _SortAlphabetic;

	int GetSortBy() { return _SortAlphabetic ? 0 : 1; }
	void LanguageDBHelper(LPCSTR sql, int lang);
};
