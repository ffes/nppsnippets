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

#include "Snippets.h"
#include "Database.h"

/////////////////////////////////////////////////////////////////////////////
//

void SnippetBase::WSetName(LPCWCH txt)
{
	if (_Name != NULL)
		free(_Name);

	_Name = (txt == NULL ? NULL : wcsdup(txt));
}

/////////////////////////////////////////////////////////////////////////////
//

Snippet::Snippet()
{
	_SnippetID = 0;
	_LibraryID = 0;
	_Name = NULL;
	_BeforeSelection = NULL;
	_AfterSelection = NULL;
	_ReplaceSelection = false;
	_NewDocument = false;
	_NewDocumentLang = L_EXTERNAL;
	_Sort = 0;
}

Snippet::Snippet(sqlite3_stmt* stmt)
{
	_Name = NULL;
	_BeforeSelection = NULL;
	_AfterSelection = NULL;
	Set(stmt);
}

///////////////////////////////////////////////////////////////////////////
// Copy-Constructor.

Snippet::Snippet(const Snippet& copy)
{
	_SnippetID = 0;
	_Name = NULL;
	_BeforeSelection = NULL;
	_AfterSelection = NULL;
	CopyValues(copy);
}

Snippet::~Snippet()
{
	if (_Name != NULL)
		free(_Name);
	if (_BeforeSelection != NULL)
		free(_BeforeSelection);
	if (_AfterSelection != NULL)
		free(_AfterSelection);
}

///////////////////////////////////////////////////////////////////////////
// Overloads of the '=' operator

Snippet& Snippet::operator=(const Snippet& copy)
{
	CopyValues(copy);
	return(*this);
}

Snippet& Snippet::operator=(sqlite3_stmt* stmt)
{
	Set(stmt);
	return(*this);
}

/////////////////////////////////////////////////////////////////////////////
//

void Snippet::CopyValues(const Snippet& copy)
{
	//_SnippetID = copy._SnippetID;
	_LibraryID = copy._LibraryID;
	WSetName(copy._Name);
	WSetBeforeSelection(copy._BeforeSelection);
	WSetAfterSelection(copy._AfterSelection);
	_ReplaceSelection = copy._ReplaceSelection;
	_NewDocument = copy._NewDocument;
	_NewDocumentLang = copy._NewDocumentLang;
	_Sort = copy._Sort;
}

/////////////////////////////////////////////////////////////////////////////
//

void Snippet::WSetBeforeSelection(LPCWCH txt)
{
	if (_BeforeSelection != NULL)
		free(_BeforeSelection);

	_BeforeSelection = (txt == NULL ? NULL : wcsdup(txt));
}

void Snippet::WSetAfterSelection(LPCWCH txt)
{
	if (_AfterSelection != NULL)
		free(_AfterSelection);

	_AfterSelection = (txt == NULL ? NULL : wcsdup(txt));
}

void Snippet::SetBeforeSelection(LPCSTR txt)
{
	if (txt == NULL)
	{
		WSetBeforeSelection(NULL);
		return;
	}

	size_t len = strlen(txt);
	if (len == 0)
	{
		WSetBeforeSelection(NULL);
		return;
	}

	// Convert from UTF-8 to WCHAR and store
    size_t size = MultiByteToWideChar(CP_UTF8, 0, txt, (int) len, NULL, 0);
	WCHAR* wBuffer = (WCHAR*) malloc(size);
	ZeroMemory(wBuffer, size);
    MultiByteToWideChar(CP_UTF8, 0, txt, (int) len, wBuffer, size);
	WSetBeforeSelection(wBuffer);
	free(wBuffer);
}

///////////////////////////////////////////////////////////////////////////////
// Convert the Unicode-string to an Ansi-string

LPSTR Snippet::Unicode2Ansi(LPCWSTR wszStr)
{
	size_t len = wcslen(wszStr);
	size_t size = sizeof(char) * (len + 3);

	char* buffer = (char*) malloc(size);
	ZeroMemory(buffer, size);

	if (wszStr != NULL)
		WideCharToMultiByte(CP_ACP, 0, wszStr, -1, buffer, (int) len, NULL, NULL);

	return buffer;
}

/////////////////////////////////////////////////////////////////////////////
//

void Snippet::Set(sqlite3_stmt* stmt)
{
	// Map the column names to column numbers
	map<string,int> cNames = ResolveColumnNames(stmt);

	// First get the texts and convert their newlines to Windows standards
	wstring before = ConvertNewLines((LPCWSTR) sqlite3_column_text16(stmt, cNames["BeforeSelection"]));
	wstring after = ConvertNewLines((LPCWSTR) sqlite3_column_text16(stmt, cNames["AfterSelection"]));

	_SnippetID = sqlite3_column_int(stmt, cNames["SnippetID"]);
	_LibraryID = sqlite3_column_int(stmt, cNames["LibraryID"]);
	WSetName((LPCWSTR) sqlite3_column_text16(stmt, cNames["Name"]));
	WSetBeforeSelection(before.c_str());
	WSetAfterSelection(after.c_str());
	SetReplaceSelection(sqlite3_column_int(stmt, cNames["ReplaceSelection"]));
	SetNewDocument(sqlite3_column_int(stmt, cNames["NewDocument"]));
	SetSort(sqlite3_column_int(stmt, cNames["Sort"]));

	// Get the language for a new document
	_NewDocumentLang = (LangType) sqlite3_column_int(stmt, cNames["NewDocumentLang"]);
	if (_NewDocumentLang == 0)
	{
		if (sqlite3_column_type(stmt, cNames["NewDocumentLang"]) == SQLITE_NULL)
		{
			_NewDocumentLang = L_EXTERNAL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Save the snippet to the database

bool Snippet::SaveToDB(bool autoOpen)
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
	sqlite3_stmt* stmt = NULL;
	bool adding = false;
	if (_SnippetID == 0)
	{
		adding = true;

		// Determine the last used SnippetID
		long maxID;
		GetLongResult("SELECT MAX(SnippetID) FROM Snippets", maxID);
		_SnippetID = maxID + 1;

		// Prepare the statement
		sqlite3_prepare_v2(g_db, "INSERT INTO Snippets(SnippetID, LibraryID, Name, BeforeSelection, AfterSelection, ReplaceSelection, NewDocument, NewDocumentLang, Sort) VALUES (@id, @libid, @name, @before, @after, @replace, @newdoc, @newdoclang, @sort)", -1, &stmt, NULL);
	}
	else
	{
		// Prepare the statement
		sqlite3_prepare_v2(g_db, "UPDATE Snippets SET Name = @name, BeforeSelection = @before, AfterSelection = @after, ReplaceSelection = @replace, NewDocument = @newdoc, NewDocumentLang = @newdoclang, Sort = @sort WHERE SnippetID = @id", -1, &stmt, NULL);
	}

	// Bind the values to the parameters
	BindInt(stmt, "@id", _SnippetID);
	if (adding)
		BindInt(stmt, "@libid", _LibraryID);
	BindText(stmt, "@name", _Name);
	BindText(stmt, "@before", _BeforeSelection);
	BindText(stmt, "@after", _AfterSelection);
	BindInt(stmt, "@replace", GetReplaceSelectionInt());
	BindInt(stmt, "@newdoc", GetNewDocumentInt());
	BindInt(stmt, "@newdoclang", _NewDocumentLang, _NewDocumentLang == L_EXTERNAL);
	BindInt(stmt, "@sort", _Sort, _Sort == 0);

	bool ret = (sqlite3_step(stmt) == SQLITE_DONE);
	sqlite3_finalize(stmt);

	if (!ret)
		MsgBox(sqlite3_errmsg(g_db));

	if (autoOpen)
		CloseDB();

	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Delete the snippet from the database

bool Snippet::DeleteFromDB()
{
	// Try to open the database
	if (!OpenDB())
	{
		MsgBox("Could not open the database");
		return false;
	}

	sqlite3_stmt *stmt = NULL;
	sqlite3_prepare_v2(g_db, "DELETE FROM Snippets WHERE SnippetID = @id", -1, &stmt, NULL);
	BindInt(stmt, "@id", _SnippetID);
	bool ret = (sqlite3_step(stmt) == SQLITE_DONE);
	sqlite3_finalize(stmt);

	CloseDB();
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// Guess the name of the snippet based upon the context of _BeforeSelection 

void Snippet::GuessName()
{
	// If the name already set, do nothing
	if (_Name != NULL)
		return;

	// Is there a text to analyse?
	if (_BeforeSelection == NULL)
		return;

	// First, try to find the first line containing text and use that as Name
	bool foundChar = false;
	WCHAR* p = _BeforeSelection;
	WCHAR* start = _BeforeSelection;
	while (*p)
	{
		// Did we run into a space character?
		if (isspace(*p))
		{
			// Did we already find any normal character
			if (foundChar)
			{
				// Did we run into the end of the line
				if (*p == '\r' || *p == '\n')
					break;
			}
		}
		else
		{
			// Is the first normal character, store it's position
			if (!foundChar)
			{
				start = p;
				foundChar = true;
			}
		}
		p++;
	}
	// If we didn't find any normal character, we're done
	if (!foundChar)
		return;

	// Now see if this text is not too long and therefore becomes impractible
	size_t len = ((size_t) p - (size_t) start) / sizeof(WCHAR);
	if (len > 50)
		len = 50;

	// Finally we can set the Name
	size_t size = sizeof(WCHAR) * (len + 3);
	WCHAR* tmp = (WCHAR*) malloc(size);
	ZeroMemory(tmp, size);
	memcpy(tmp, start, sizeof(WCHAR) * len);
	WSetName(tmp);
	free(tmp);
}
