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

Library::Library(SqliteStatement* stmt)
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

void Library::Set(SqliteStatement* stmt)
{
	_LibraryID = stmt->GetIntColumn("LibraryID");
	WSetName(stmt->GetWTextColumn("Name").c_str());
	WSetCreatedBy(stmt->GetWTextColumn("CreatedBy").c_str());
	WSetComments(stmt->GetWTextColumn("Comments").c_str());
	SetSortAlphabetic(stmt->GetIntColumn("SortBy"));
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::SaveToDB(bool autoOpen)
{
	// Try to open the database
	g_db->Open();

	// Saving a new record or updating an existing?
	SqliteStatement stmt(g_db);
	if (_LibraryID == 0)
	{
		// Determine the last used LibraryID
		SqliteStatement stmt2(g_db, "SELECT MAX(LibraryID) AS MaxID FROM Library");
		stmt2.GetNextRecord();
		_LibraryID = stmt2.GetIntColumn("MaxID") + 1;
		stmt2.Finalize();

		// Prepare the statement
		stmt.Prepare("INSERT INTO Library(LibraryID, Name, CreatedBy, Comments, SortBy) VALUES (@id, @name, @createdby, @comments, @sortby)");
	}
	else
	{
		// Prepare the statement
		stmt.Prepare("UPDATE Library SET Name = @name, CreatedBy = @createdby, Comments = @comments, SortBy = @sortby WHERE LibraryID = @id");
	}

	// Bind the values to the parameters
	stmt.Bind("@id", _LibraryID);
	stmt.Bind("@name", _Name);
	stmt.Bind("@createdby", _CreatedBy);
	stmt.Bind("@comments", _Comments);
	stmt.Bind("@sortby", GetSortBy());

	// Save the record
	stmt.SaveRecord();
	stmt.Finalize();

	if (autoOpen)
		g_db->Close();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::DeleteFromDB()
{
	g_db->Open();
	SqliteStatement stmt(g_db, "DELETE FROM Library WHERE LibraryID = @id");
	stmt.Bind("@id", _LibraryID);
	stmt.SaveRecord();
	stmt.Finalize();
	g_db->Close();
	return true;
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

void Library::ExportTo(LPCWCH filename)
{
	// Open the db and create/attach the new db
	g_db->Open();
	g_db->Attach(filename, L"export");
	g_db->CreateExportDB();
	g_db->Detach(L"export");
	g_db->Close();

	// Now use that export database as temporary "base" database
	// and import the current lib from the temp database
	// When all done, restore the original database filename
	WCHAR orgFile[MAX_PATH];
	wcsncpy(orgFile, g_db->GetFilename(), MAX_PATH);
	g_db->SetFilename(filename);
	g_db->Open();
	g_db->ImportLibrary(orgFile, _LibraryID);
	g_db->Close();
	g_db->SetFilename(orgFile);
}

/////////////////////////////////////////////////////////////////////////////
//

bool Library::LanguageDBHelper(LPCSTR sql, int lang)
{
	g_db->Open();
	SqliteStatement stmt(g_db, sql);
	stmt.Bind("@id", _LibraryID);
	stmt.Bind("@lang", lang);
	stmt.SaveRecord();
	stmt.Finalize();
	g_db->Close();
	return true;
}
