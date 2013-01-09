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

#include "sqlite3.h"

#include "NPP/PluginInterface.h"
#include "NppSnippets.h"
#include "Database.h"
#include "WaitCursor.h"

WCHAR g_dbFile[MAX_PATH];
sqlite3* g_db = NULL;

/////////////////////////////////////////////////////////////////////////////
// Map the column names to column numbers

map<string,int> ResolveColumnNames(sqlite3_stmt* stmt)
{
	map<string,int> names;

	for (int i = 0; i < sqlite3_column_count(stmt); i++)
		names[sqlite3_column_name(stmt, i)] =  i;

	return names;
}

/////////////////////////////////////////////////////////////////////////////
//

bool RunSQL(LPCSTR szSQL)
{
	if (sqlite3_exec(g_db, szSQL, NULL, NULL, NULL) != SQLITE_OK)
	{
		MsgBox(szSQL);
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

bool GetLongResult(LPCSTR szStmt, long& result)
{
	bool ret = false;

	sqlite3_stmt *stmt;
	sqlite3_prepare(g_db, szStmt, -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		result = sqlite3_column_int(stmt, 0);
		ret = true;
	}

	sqlite3_finalize(stmt);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool BindText(sqlite3_stmt *stmt, const char* param, const WCHAR* var)
{
	int col = sqlite3_bind_parameter_index(stmt, param);
	
	int res = SQLITE_OK;
	if (var == NULL)
	{
		res = sqlite3_bind_null(stmt, col);
	}
	else if (wcslen(var) == 0)
	{
		res = sqlite3_bind_null(stmt, col);
	}
	else
	{
		res = sqlite3_bind_text16(stmt, col, var, -1, SQLITE_STATIC);
	}

	if (res != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

bool BindInt(sqlite3_stmt *stmt, const char* param, const int i, bool null)
{
	int col = sqlite3_bind_parameter_index(stmt, param);

	int res = (null ? sqlite3_bind_null(stmt, col) : sqlite3_bind_int(stmt, col, i));

	if (res != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
//

bool CloseDB()
{
	bool ret = sqlite3_close(g_db) == SQLITE_OK;
	g_db = NULL;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
//

bool OpenDB()
{
	// Is the filename filled?
	if (wcslen(g_dbFile) == 0)
		return false;

	// Open the database
	int rc = sqlite3_open16(g_dbFile, &g_db);
	if (rc != SQLITE_OK)
	{
		MsgBox(sqlite3_errmsg(g_db));
		sqlite3_close(g_db);
		return false;
	}

	// Make sure the database has the right version
	if (!CheckDBVersion())
	{
		MsgBox(sqlite3_errmsg(g_db));
		sqlite3_close(g_db);
		return false;
	}

	// Make use the foreign key constraints are turned on
	if (!RunSQL("PRAGMA foreign_keys = ON"))
	{
		MsgBox(sqlite3_errmsg(g_db));
		sqlite3_close(g_db);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Compress the database

bool VacuumDB()
{
	return RunSQL("VACUUM;");
}

/////////////////////////////////////////////////////////////////////////////
// Upgrade the database from schema version 1 to 2

static void UpgradeDatabase_1_2()
{
	// Create the LibraryLang table and fill it
	RunSQL("BEGIN TRANSACTION;");
	RunSQL("CREATE TABLE LibraryLang(LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,Lang INTEGER NOT NULL,LastUsed BOOL NOT NULL DEFAULT 0,PRIMARY KEY (LibraryID, Lang));");
	RunSQL("INSERT INTO LibraryLang SELECT LibraryID, Lang, LastUsed FROM Library;");
	RunSQL("COMMIT TRANSACTION;");

	// Delete the "Lang" and "LastUsed" column from the Library table
	RunSQL("BEGIN TRANSACTION;");
	RunSQL("DROP INDEX LibLang;");	// Index not needed anymore
	RunSQL("CREATE TEMPORARY TABLE Library_backup(LibraryID INTEGER,Name TEXT,CreatedBy TEXT,Comments TEXT,SortBy INTEGER);");
	RunSQL("INSERT INTO Library_backup SELECT LibraryID,Name,CreatedBy,Comments,SortBy FROM Library;");
	RunSQL("DROP TABLE Library;");
	RunSQL("CREATE TABLE Library (LibraryID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,Name TEXT NOT NULL,CreatedBy TEXT,Comments TEXT,SortBy INTEGER NOT NULL DEFAULT 0);");
	RunSQL("INSERT INTO Library SELECT LibraryID,Name,CreatedBy,Comments,SortBy FROM Library_backup;");
	RunSQL("DROP TABLE Library_backup;");
	RunSQL("COMMIT TRANSACTION;");

	// Throw away these old indices. New ones will be created later
	RunSQL("DROP INDEX SnipName;");
	RunSQL("DROP INDEX SnipSort;");

	// Add a primary key table to Snippets
	RunSQL("BEGIN TRANSACTION;");
	RunSQL("CREATE TEMPORARY TABLE Snippets_backup(LibraryID INTEGER,Name TEXT,BeforeSelection TEXT,AfterSelection TEXT,ReplaceSelection BOOL,NewDocument BOOL,NewDocumentLang INTEGER,Sort INTEGER);");
	RunSQL("INSERT INTO Snippets_backup SELECT LibraryID,Name,BeforeSelection,AfterSelection,ReplaceSelection,NewDocument,NewDocumentLang,Sort FROM Snippets;");
	RunSQL("DROP TABLE Snippets;");
	RunSQL("CREATE TABLE Snippets(SnippetID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,Name TEXT NOT NULL,BeforeSelection TEXT NOT NULL,AfterSelection TEXT,ReplaceSelection BOOL NOT NULL DEFAULT 0,NewDocument BOOL NOT NULL DEFAULT 0,NewDocumentLang INTEGER,Sort INTEGER);");
	RunSQL("INSERT INTO Snippets(LibraryID,Name,BeforeSelection,AfterSelection,ReplaceSelection,NewDocument,NewDocumentLang,Sort) SELECT LibraryID,Name,BeforeSelection,AfterSelection,ReplaceSelection,NewDocument,NewDocumentLang,Sort FROM Snippets_backup;");
	RunSQL("DROP TABLE Snippets_backup;");
	RunSQL("COMMIT TRANSACTION;");

	// Create new indices
	RunSQL("CREATE INDEX SnipName ON Snippets(LibraryID, Name, Sort);");
	RunSQL("CREATE INDEX SnipSort ON Snippets(LibraryID, Sort, Name);");

	// We are at schema version 2
	RunSQL("PRAGMA user_version = 2;");
}

/////////////////////////////////////////////////////////////////////////////
// Upgrade the database from schema version 2 to 3

static void UpgradeDatabase_2_3()
{
	// Add the new tabel to store which library is last used for with language
	RunSQL("BEGIN TRANSACTION;");
	RunSQL("CREATE TABLE LangLastUsed(Lang INTEGER PRIMARY KEY NOT NULL,LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE);");
	RunSQL("INSERT INTO LangLastUsed SELECT Lang,LibraryID FROM LibraryLang WHERE LastUsed = 1;");
	RunSQL("COMMIT TRANSACTION;");

	// Delete the LastUsed column from the LibraryLang table
	RunSQL("BEGIN TRANSACTION;");
	RunSQL("CREATE TEMPORARY TABLE LibraryLang_backup(LibraryID INTEGER,Lang INTEGER);");
	RunSQL("INSERT INTO LibraryLang_backup SELECT LibraryID, Lang FROM LibraryLang;");
	RunSQL("DROP TABLE LibraryLang;");
	RunSQL("CREATE TABLE LibraryLang(LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,Lang INTEGER NOT NULL,PRIMARY KEY (LibraryID, Lang));");
	RunSQL("INSERT INTO LibraryLang SELECT LibraryID, Lang FROM LibraryLang_backup;");
	RunSQL("DROP TABLE LibraryLang_backup;");
	RunSQL("COMMIT TRANSACTION;");

	// We are at schema version 3
	RunSQL("PRAGMA user_version = 3;");
}

/////////////////////////////////////////////////////////////////////////////
//

bool CheckDBVersion()
{
	long schema_version = 0;
	if (!GetLongResult("PRAGMA user_version;", schema_version))
		return false;

	WaitCursor wait(false);
	switch (schema_version)
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
			VacuumDB();
			// fall through

		case 3:
			// The right version!
			return true;
	}

	// Running with an newer database!
	return false;
}
