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

Snippet::Snippet(SqliteStatement* stmt)
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

Snippet& Snippet::operator=(SqliteStatement* stmt)
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

void Snippet::Set(SqliteStatement* stmt)
{
	_SnippetID = stmt->GetIntColumn("SnippetID");
	_LibraryID = stmt->GetIntColumn("LibraryID");
	WSetName(stmt->GetWTextColumn("Name").c_str());
	WSetBeforeSelection(stmt->GetWTextColumn("BeforeSelection").c_str());
	WSetAfterSelection(stmt->GetWTextColumn("AfterSelection").c_str());
	SetReplaceSelection(stmt->GetIntColumn("ReplaceSelection"));
	SetNewDocument(stmt->GetIntColumn("NewDocument"));
	SetSort(stmt->GetIntColumn("Sort"));

	// Get the language for a new document
	_NewDocumentLang = (LangType) stmt->GetIntColumn("NewDocumentLang");
	if (_NewDocumentLang == 0)
	{
		//if (sqlite3_column_type(stmt, cNames["NewDocumentLang"]) == SQLITE_NULL)
		//{
		//	_NewDocumentLang = L_EXTERNAL;
		//}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Save the snippet to the database

bool Snippet::SaveToDB(bool autoOpen)
{
	// Try to open the database
	g_db->Open();

	// Saving a new record or updating an existing?
	SqliteStatement stmt(g_db);

	bool adding = false;
	if (_SnippetID == 0)
	{
		adding = true;

		// Determine the last used SnippetID
		SqliteStatement stmt2(g_db, "SELECT MAX(SnippetID) AS MaxID FROM Snippets");
		stmt2.GetNextRecord();
		_SnippetID = stmt2.GetIntColumn("MaxID") + 1;
		stmt2.Finalize();

		// Prepare the statement
		stmt.Prepare("INSERT INTO Snippets(SnippetID, LibraryID, Name, BeforeSelection, AfterSelection, ReplaceSelection, NewDocument, NewDocumentLang, Sort) VALUES (@id, @libid, @name, @before, @after, @replace, @newdoc, @newdoclang, @sort)");
	}
	else
	{
		// Prepare the statement
		stmt.Prepare("UPDATE Snippets SET Name = @name, BeforeSelection = @before, AfterSelection = @after, ReplaceSelection = @replace, NewDocument = @newdoc, NewDocumentLang = @newdoclang, Sort = @sort WHERE SnippetID = @id");
	}

	// Bind the values to the parameters
	stmt.Bind("@id", _SnippetID);
	if (adding)
		stmt.Bind("@libid", _LibraryID);
	stmt.Bind("@name", _Name);
	stmt.Bind("@before", _BeforeSelection);
	stmt.Bind("@after", _AfterSelection);
	stmt.Bind("@replace", GetReplaceSelectionInt());
	stmt.Bind("@newdoc", GetNewDocumentInt());
	stmt.Bind("@newdoclang", _NewDocumentLang, _NewDocumentLang == L_EXTERNAL);
	stmt.Bind("@sort", _Sort, _Sort == 0);

	stmt.SaveRecord();
	stmt.Finalize();

	if (autoOpen)
		g_db->Close();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Delete the snippet from the database

bool Snippet::DeleteFromDB()
{
	// Try to open the database
	g_db->Open();

	SqliteStatement stmt(g_db, "DELETE FROM Snippets WHERE SnippetID = @id");
	stmt.Bind("@id", _SnippetID);
	stmt.SaveRecord();
	stmt.Finalize();

	g_db->Close();
	return true;
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
