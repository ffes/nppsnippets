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

#include "Library.h"
#include "Database.h"

/////////////////////////////////////////////////////////////////////////////
//

Library::Library()
{
	_LibraryID = 0;
	_Name = NULL;
	_CreatedBy = NULL;
	_Comments = NULL;
	_SortAlphabetic = true;
}

Library::Library(sqlite3_stmt* stmt)
{
	_Name = NULL;
	_CreatedBy = NULL;
	_Comments = NULL;
	Set(stmt);
}

Library::~Library()
{
	if (_Name != NULL)
		free(_Name);
	if (_CreatedBy != NULL)
		free(_CreatedBy);
	if (_Comments != NULL)
		free(_Comments);
}

/////////////////////////////////////////////////////////////////////////////
//

void Library::WSetCreatedBy(LPCWCH txt)
{
	if (_CreatedBy != NULL)
		free(_CreatedBy);

	_CreatedBy = (txt == NULL ? NULL : wcsdup(txt));
}

void Library::WSetComments(LPCWCH txt)
{
	if (_Comments != NULL)
		free(_Comments);

	_Comments = (txt == NULL ? NULL : wcsdup(txt));
}

/////////////////////////////////////////////////////////////////////////////
//

void Library::Set(sqlite3_stmt* stmt)
{
	// Map the column names to column numbers
	std::map<string,int> cNames = ResolveColumnNames(stmt);

	_LibraryID = sqlite3_column_int(stmt, cNames["LibraryID"]);
	WSetName((LPCWSTR) sqlite3_column_text16(stmt, cNames["Name"]));
	WSetCreatedBy((LPCWSTR) sqlite3_column_text16(stmt, cNames["CreatedBy"]));
	WSetComments((LPCWSTR) sqlite3_column_text16(stmt, cNames["Comments"]));
	SetSortAlphabetic(sqlite3_column_int(stmt, cNames["SortBy"]));
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::SaveToDB(bool autoOpen)
{
	// Try to open the database
	if (autoOpen)
	{
		if (!OpenDB())
		{
			MsgBox("Could not open the database");
			return false;
		}
	}

	// Saving a new record or updating an existing?
	sqlite3_stmt *stmt = NULL;
	if (_LibraryID == 0)
	{
		// Determine the last used SnippetID
		long maxID;
		GetLongResult("SELECT MAX(LibraryID) FROM Library", maxID);
		_LibraryID = maxID + 1;

		// Prepare the statement
		sqlite3_prepare_v2(g_db, "INSERT INTO Library(LibraryID, Name, CreatedBy, Comments, SortBy) VALUES (@id, @name, @createdby, @comments, @sortby)", -1, &stmt, NULL);
	}
	else
	{
		// Prepare the statement
		sqlite3_prepare_v2(g_db, "UPDATE Library SET Name = @name, CreatedBy = @createdby, Comments = @comments, SortBy = @sortby WHERE LibraryID = @id", -1, &stmt, NULL);
	}

	// Bind the values to the parameters
	BindInt(stmt, "@id", _LibraryID);
	BindText(stmt, "@name", _Name);
	BindText(stmt, "@createdby", _CreatedBy);
	BindText(stmt, "@comments", _Comments);
	BindInt(stmt, "@sortby", GetSortBy());

	bool ret = (sqlite3_step(stmt) == SQLITE_DONE);
	sqlite3_finalize(stmt);

	if (!ret)
		MsgBox(sqlite3_errmsg(g_db));

	if (autoOpen)
		CloseDB();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::DeleteFromDB()
{
	// Try to open the database
	if (!OpenDB())
	{
		MsgBox("Could not open the database");
		return false;
	}

	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare_v2(g_db, "DELETE FROM Library WHERE LibraryID = @id", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, sqlite3_bind_parameter_index(stmt, "@id"), _LibraryID);
	bool ret = (sqlite3_step(stmt) == SQLITE_DONE);
	sqlite3_finalize(stmt);

	CloseDB();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::AddLanguageToDB(int lang)
{
	return LanguageDBHelper("INSERT INTO LibraryLang(LibraryID, Lang) VALUES (@id, @lang)", lang);
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::DeleteLanguageFromDB(int lang)
{
	return LanguageDBHelper("DELETE FROM LibraryLang WHERE LibraryID = @id AND Lang = @lang)", lang);
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::LanguageDBHelper(LPCSTR sql, int lang)
{
	// Try to open the database
	if (!OpenDB())
	{
		MsgBox("Could not open the database");
		return false;
	}

	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
	BindInt(stmt, "@id", _LibraryID);
	BindInt(stmt, "@lang", lang);
	bool ret = (sqlite3_step(stmt) == SQLITE_DONE);
	sqlite3_finalize(stmt);

	CloseDB();
	return ret;
}
