/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  NppSnippets - Code Snippets plugin for Notepad++                       //
//  Copyright (C) 2013 Frank Fesevur                                       //
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
#include "SnippetsDB.h"
#include "WaitCursor.h"

SnippetsDB* g_db = NULL;

/////////////////////////////////////////////////////////////////////////////
//

SnippetsDB::SnippetsDB() : SqliteDatabase()
{
	SetValues();
}

SnippetsDB::SnippetsDB(LPCWSTR file) : SqliteDatabase(file)
{
	SetValues();
}

/////////////////////////////////////////////////////////////////////////////
// Nothing is set yet

void SnippetsDB::SetValues()
{
}

/////////////////////////////////////////////////////////////////////////////
//

void SnippetsDB::Open()
{
	if (_db == NULL)
		SqliteDatabase::Open();

	// Make sure the database has the right version
	if (!CheckDBVersion())
	{
		throw SqliteException("Database has wrong version, please regenerate!");		
		return;
	}

	EnableForeignKeys();
}

/////////////////////////////////////////////////////////////////////////////
// Create the export database

void SnippetsDB::CreateExportDB()
{
	BeginTransaction();
	Execute("CREATE TABLE Library(LibraryID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, Name TEXT NOT NULL, CreatedBy TEXT, Comments TEXT, SortBy INTEGER NOT NULL DEFAULT 0);");
	Execute("CREATE TABLE LibraryLang(LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE, Lang INTEGER NOT NULL, PRIMARY KEY (LibraryID, Lang));");
	Execute("CREATE TABLE Snippets(SnippetID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE, Name TEXT NOT NULL, BeforeSelection TEXT NOT NULL, AfterSelection TEXT, ReplaceSelection BOOL NOT NULL DEFAULT 0, NewDocument BOOL NOT NULL DEFAULT 0, NewDocumentLang INTEGER, Sort INTEGER);");
	Execute("CREATE INDEX SnipName ON Snippets(LibraryID, Name, Sort);");
	Execute("CREATE INDEX SnipSort ON Snippets(LibraryID, Sort, Name);");
	Execute("CREATE TABLE LangLastUsed(Lang INTEGER PRIMARY KEY NOT NULL, LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE);");
	CommitTransaction();

	// We are at schema version 3
	SetUserVersion(3);
}

/////////////////////////////////////////////////////////////////////////////
// Upgrade the database from schema version 1 to 2

void SnippetsDB::UpgradeDatabase_1_2()
{
	// Create the LibraryLang table and fill it
	BeginTransaction();
	Execute("CREATE TABLE LibraryLang(LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,Lang INTEGER NOT NULL,LastUsed BOOL NOT NULL DEFAULT 0,PRIMARY KEY (LibraryID, Lang));");
	Execute("INSERT INTO LibraryLang SELECT LibraryID, Lang, LastUsed FROM Library;");
	CommitTransaction();

	// Delete the "Lang" and "LastUsed" column from the Library table
	BeginTransaction();
	Execute("DROP INDEX LibLang;");	// Index not needed anymore
	Execute("CREATE TEMPORARY TABLE Library_backup(LibraryID INTEGER,Name TEXT,CreatedBy TEXT,Comments TEXT,SortBy INTEGER);");
	Execute("INSERT INTO Library_backup SELECT LibraryID,Name,CreatedBy,Comments,SortBy FROM Library;");
	Execute("DROP TABLE Library;");
	Execute("CREATE TABLE Library (LibraryID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,Name TEXT NOT NULL,CreatedBy TEXT,Comments TEXT,SortBy INTEGER NOT NULL DEFAULT 0);");
	Execute("INSERT INTO Library SELECT LibraryID,Name,CreatedBy,Comments,SortBy FROM Library_backup;");
	Execute("DROP TABLE Library_backup;");
	CommitTransaction();

	// Throw away these old indices. New ones will be created later
	Execute("DROP INDEX SnipName;");
	Execute("DROP INDEX SnipSort;");

	// Add a primary key table to Snippets
	BeginTransaction();
	Execute("CREATE TEMPORARY TABLE Snippets_backup(LibraryID INTEGER,Name TEXT,BeforeSelection TEXT,AfterSelection TEXT,ReplaceSelection BOOL,NewDocument BOOL,NewDocumentLang INTEGER,Sort INTEGER);");
	Execute("INSERT INTO Snippets_backup SELECT LibraryID,Name,BeforeSelection,AfterSelection,ReplaceSelection,NewDocument,NewDocumentLang,Sort FROM Snippets;");
	Execute("DROP TABLE Snippets;");
	Execute("CREATE TABLE Snippets(SnippetID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,Name TEXT NOT NULL,BeforeSelection TEXT NOT NULL,AfterSelection TEXT,ReplaceSelection BOOL NOT NULL DEFAULT 0,NewDocument BOOL NOT NULL DEFAULT 0,NewDocumentLang INTEGER,Sort INTEGER);");
	Execute("INSERT INTO Snippets(LibraryID,Name,BeforeSelection,AfterSelection,ReplaceSelection,NewDocument,NewDocumentLang,Sort) SELECT LibraryID,Name,BeforeSelection,AfterSelection,ReplaceSelection,NewDocument,NewDocumentLang,Sort FROM Snippets_backup;");
	Execute("DROP TABLE Snippets_backup;");
	CommitTransaction();

	// Create new indices
	Execute("CREATE INDEX SnipName ON Snippets(LibraryID, Name, Sort);");
	Execute("CREATE INDEX SnipSort ON Snippets(LibraryID, Sort, Name);");

	// We are at schema version 2
	SetUserVersion(2);
}

/////////////////////////////////////////////////////////////////////////////
// Upgrade the database from schema version 2 to 3

void SnippetsDB::UpgradeDatabase_2_3()
{
	// Add the new tabel to store which library is last used for with language
	BeginTransaction();
	Execute("CREATE TABLE LangLastUsed(Lang INTEGER PRIMARY KEY NOT NULL,LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE);");
	Execute("INSERT INTO LangLastUsed SELECT Lang,LibraryID FROM LibraryLang WHERE LastUsed = 1;");
	CommitTransaction();

	// Delete the LastUsed column from the LibraryLang table
	BeginTransaction();
	Execute("CREATE TEMPORARY TABLE LibraryLang_backup(LibraryID INTEGER,Lang INTEGER);");
	Execute("INSERT INTO LibraryLang_backup SELECT LibraryID, Lang FROM LibraryLang;");
	Execute("DROP TABLE LibraryLang;");
	Execute("CREATE TABLE LibraryLang(LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,Lang INTEGER NOT NULL,PRIMARY KEY (LibraryID, Lang));");
	Execute("INSERT INTO LibraryLang SELECT LibraryID, Lang FROM LibraryLang_backup;");
	Execute("DROP TABLE LibraryLang_backup;");
	CommitTransaction();

	// We are at schema version 3
	SetUserVersion(3);
}

/////////////////////////////////////////////////////////////////////////////
//

bool SnippetsDB::CheckDBVersion()
{
	WaitCursor wait(false);
	switch (GetUserVersion())
	{
		case 0:
			// Database probably not found
			return false;

		case 1:
			wait.Show();
			UpgradeDatabase_1_2();
			// fall through

		case 2:
			wait.Show();
			UpgradeDatabase_2_3();		
			Vacuum();
			// fall through

		case 3:
			// The right version!
			return true;
	}

	// Running with an newer database!
	return false;
}
