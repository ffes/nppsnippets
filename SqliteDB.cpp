/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// SqliteDB Classes                                                        //
// Version 1.1, 27-May-2015                                                //
//                                                                         //
// Copyright (c) 2013-2015, Frank Fesevur <http://www.fesevur.com>         //
// All rights reserved.                                                    //
//                                                                         //
// Redistribution and use in source and binary forms, with or without      //
// modification, are permitted provided that the following conditions      //
// are met:                                                                //
//                                                                         //
// 1. Redistributions of source code must retain the above copyright       //
//    notice, this list of conditions and the following disclaimer.        //
// 2. Redistributions in binary form must reproduce the above copyright    //
//    notice, this list of conditions and the following disclaimer in the  //
//    documentation and/or other materials provided with the distribution. //
//                                                                         //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS     //
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT       //
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR   //
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    //
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,   //
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT        //
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY   //
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT     //
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE   //
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include "SqliteDB.h"
using namespace std;

#ifdef _MSC_VER
#define snprintf _snprintf
#define snwprintf swprintf
#define wunlink _wunlink
#endif

/////////////////////////////////////////////////////////////////////////////
//

SqliteDatabase::SqliteDatabase()
{
	_dbFile[0] = 0;
	_db = NULL;
}

SqliteDatabase::SqliteDatabase(LPCWSTR file)
{
	_db = NULL;
	SetFilename(file);
	Open();
}

SqliteDatabase::~SqliteDatabase()
{
	if (_db != NULL)
		sqlite3_close(_db);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::SetFilename(LPCWSTR file)
{
	wcsncpy(_dbFile, file, MAX_PATH);
}

/////////////////////////////////////////////////////////////////////////////
// Close the database

void SqliteDatabase::Close()
{
	if (sqlite3_close(_db) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));

	_db = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// Open the database

void SqliteDatabase::Open()
{
	if (_db != NULL)
		throw SqliteException("Database already opened!");

	// Is the filename filled?
	if (wcslen(_dbFile) == 0)
		throw SqliteException("Filename not set!");

	// Open the database
	if (sqlite3_open16(_dbFile, &_db) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
// Open the database

void SqliteDatabase::Open(LPCWSTR file)
{
	SetFilename(file);
	Open();
}

/////////////////////////////////////////////////////////////////////////////
// Delete the database file

void SqliteDatabase::Delete()
{
	if (!DeleteFile(_dbFile))
	{
		DWORD err = GetLastError();
		if (err != ERROR_FILE_NOT_FOUND)
			throw SqliteException("Unable to delete SqliteDatabase");
	}
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::Attach(LPCWSTR file, LPCWSTR alias)
{
	SqliteStatement stmt(this, "ATTACH DATABASE @file AS @alias");
	stmt.Bind("@file", file);
	stmt.Bind("@alias", alias);
	stmt.SaveRecord();
	stmt.Finalize();
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::Detach(LPCWSTR alias)
{
	SqliteStatement stmt(this, "DETACH DATABASE @alias");
	stmt.Bind("@alias", alias);
	stmt.SaveRecord();
	stmt.Finalize();
}

/////////////////////////////////////////////////////////////////////////////
// Compress the database

void SqliteDatabase::Vacuum()
{
	Execute("VACUUM;");
}

/////////////////////////////////////////////////////////////////////////////
// Check if a table exists

bool SqliteDatabase::TableExists(const char* table)
{
	SqliteStatement stmt(this, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = @name;");
	stmt.Bind("@name", table);
	stmt.GetNextRecord();
	int count = stmt.GetIntColumn(0);
	return (count == 1);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::BeginTransaction()
{
	Execute("BEGIN TRANSACTION;");
}

void SqliteDatabase::CommitTransaction()
{
	Execute("COMMIT TRANSACTION;");
}

void SqliteDatabase::RollbackTransaction()
{
	Execute("ROLLBACK TRANSACTION;");
}

/////////////////////////////////////////////////////////////////////////////
// Execute an SQL statement without results (UPDATE, INSERT, DELETE, etc)

void SqliteDatabase::Execute(LPCSTR szSQL)
{
	if (sqlite3_exec(_db, szSQL, NULL, NULL, NULL) != SQLITE_OK)
		throw sqlite3_errmsg(_db);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::SetUserVersion(long version, const char* dbname)
{
	char sql[MAX_PATH];
	snprintf(sql, MAX_PATH, "PRAGMA %s.user_version = %ld;", dbname == NULL ? "main" : dbname, version);
	Execute(sql);
}

/////////////////////////////////////////////////////////////////////////////
//

long SqliteDatabase::GetUserVersion(const char* dbname)
{
	char sql[MAX_PATH];
	strncpy(sql, "PRAGMA ", MAX_PATH);
	strncat(sql, dbname == NULL ? "main" : dbname, MAX_PATH);
	strncat(sql, ".user_version;", MAX_PATH);

	SqliteStatement stmt(this, sql);
	stmt.GetNextRecord();
	return stmt.GetIntColumn(0);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::EnableForeignKeys(bool on)
{
	char sql[MAX_PATH];
	strncpy(sql, "PRAGMA foreign_keys = ", MAX_PATH);
	strncat(sql, on ? "ON" : "OFF", MAX_PATH);
	Execute(sql);
}

/////////////////////////////////////////////////////////////////////////////
//

SqliteStatement::SqliteStatement(SqliteDatabase* db)
{
	_db = db->GetDB();
	_stmt = NULL;
}

SqliteStatement::SqliteStatement(SqliteDatabase* db, const char* sql)
{
	_db = db->GetDB();
	_stmt = NULL;
	Prepare(sql);
}

SqliteStatement::~SqliteStatement()
{
	if (_stmt != NULL)
		sqlite3_finalize(_stmt);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteStatement::Prepare(const char* sql)
{
	if (sqlite3_prepare_v2(_db, sql, -1, &_stmt, NULL) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
	_colNames.clear();
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteStatement::SaveRecord()
{
	if (sqlite3_step(_stmt) != SQLITE_DONE)
		throw SqliteException(sqlite3_errmsg(_db));

	if (sqlite3_reset(_stmt) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::GetNextRecord()
{
	int rc = sqlite3_step(_stmt);

	if (rc == SQLITE_ROW)
	{
		if (_colNames.empty())
			ResolveColumnNames();
		return true;
	}

	if (rc != SQLITE_DONE)
		throw SqliteException(sqlite3_errmsg(_db));

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteStatement::Finalize()
{
	if (sqlite3_finalize(_stmt) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));

	_stmt = NULL;
	_colNames.clear();
}

/////////////////////////////////////////////////////////////////////////////
//

int SqliteStatement::GetBindParameterIndex(std::string col)
{
	return sqlite3_bind_parameter_index(_stmt, col.c_str());
}

/////////////////////////////////////////////////////////////////////////////
// Binds a wchar string to the given parameter. Empty strings are bound
// as null

void SqliteStatement::Bind(const char* param, const WCHAR* val)
{
	Bind(GetBindParameterIndex(param), val);
}

void SqliteStatement::Bind(int col, const WCHAR* val)
{
	int res = SQLITE_OK;
	if (val == NULL)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else if (wcslen(val) == 0)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else
	{
		res = sqlite3_bind_text16(_stmt, col, val, -1, SQLITE_STATIC);
	}

	if (res != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
// Binds a char string to the given parameter. Empty strings are bound
// as null

void SqliteStatement::Bind(const char* param, const char *val)
{
	Bind(GetBindParameterIndex(param), val);
}

void SqliteStatement::Bind(int col, const char *val)
{
	int res = SQLITE_OK;
	if (val == NULL)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else if (strlen(val) == 0)
	{
		res = sqlite3_bind_null(_stmt, col);
	}
	else
	{
		res = sqlite3_bind_text(_stmt, col, val, -1, SQLITE_STATIC);
	}

	if (res != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
// Bind an integer to the given parameter. If optional "bool null" is true,
// null is bound. Use like this: stmt.Bind("col", var, var == 0);

void SqliteStatement::Bind(const char* param, int val, bool null)
{
	Bind(GetBindParameterIndex(param), val, null);
}

void SqliteStatement::Bind(int col, int val, bool null)
{
	int res = (null ? sqlite3_bind_null(_stmt, col) : sqlite3_bind_int(_stmt, col, val));
	if (res != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
// Binds 1 for true and 0 for false to the given parameter

void SqliteStatement::Bind(const char* param, bool val)
{
	Bind(GetBindParameterIndex(param), val);
}

void SqliteStatement::Bind(int col, bool val)
{
	if (sqlite3_bind_int(_stmt, col, val ? 1 : 0) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
// Bind NULL to the given parameter

void SqliteStatement::Bind(const char* param)
{
	Bind(GetBindParameterIndex(param));
}

void SqliteStatement::Bind(int col)
{
	if (sqlite3_bind_null(_stmt, col) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
// Map the column numbers to column names

void SqliteStatement::ResolveColumnNames()
{
	for (int i = 0; i < GetColumnCount(); i++)
		_colNames[sqlite3_column_name(_stmt, i)] =  i;
}

/////////////////////////////////////////////////////////////////////////////
//

int SqliteStatement::GetColumnCount()
{
	return sqlite3_column_count(_stmt);
}

std::string SqliteStatement::GetTextColumn(int col)
{
	LPCSTR val = (LPCSTR) sqlite3_column_text(_stmt, col);
	return (val == NULL ? "" : val);
}

std::string SqliteStatement::GetTextColumn(std::string col)
{
	return GetTextColumn(_colNames[col]);
}

std::wstring SqliteStatement::GetWTextColumn(int col)
{
	LPCWSTR val = (LPCWSTR) sqlite3_column_text16(_stmt, col);
	return (val == NULL ? L"" : val);
}

std::wstring SqliteStatement::GetWTextColumn(std::string col)
{
	return GetWTextColumn(_colNames[col]);
}

/////////////////////////////////////////////////////////////////////////////
//

int SqliteStatement::GetIntColumn(int col)
{
	return sqlite3_column_int(_stmt, col);
}

int SqliteStatement::GetIntColumn(std::string col)
{
	return sqlite3_column_int(_stmt, _colNames[col]);
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::GetBoolColumn(int col)
{
	return (sqlite3_column_int(_stmt, col) > 0);
}

bool SqliteStatement::GetBoolColumn(std::string col)
{
	return (sqlite3_column_int(_stmt, _colNames[col]) > 0);
}
