/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// SqliteDB Classes                                                        //
// Version 1.1.1, 16-Jun-2019                                              //
//                                                                         //
// Copyright (c) 2013-2019, Frank Fesevur <https://www.fesevur.com>        //
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
#include <stdio.h>
#include "SqliteDB.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#define snwprintf swprintf
#define wunlink _wunlink
#endif

/////////////////////////////////////////////////////////////////////////////
//

SqliteDatabase::SqliteDatabase()
{
	_db = nullptr;
}

SqliteDatabase::SqliteDatabase(LPCWSTR file)
{
	_db = nullptr;
	SetFilename(file);
	Open();
}

SqliteDatabase::~SqliteDatabase()
{
	if (_db != nullptr)
		sqlite3_close(_db);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::SetFilename(LPCWSTR file)
{
	_dbFile = file;
}

/**
 * Close the database
 */

void SqliteDatabase::Close()
{
	if (sqlite3_close(_db) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));

	_db = nullptr;
}

/**
 * Open the database
 *
 * @pre The filename is already set
 */

void SqliteDatabase::Open()
{
	if (_db != nullptr)
		throw SqliteException("Database already opened!");

	// Is the filename filled?
	if (_dbFile.length() == 0)
		throw SqliteException("Filename not set!");

	// Open the database
	if (sqlite3_open16(_dbFile.c_str(), &_db) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/**
 * Open the database
 *
 * @param file The filename of the database to be opened.
 */

void SqliteDatabase::Open(LPCWSTR file)
{
	SetFilename(file);
	Open();
}

/**
 * Delete the database file
 */

void SqliteDatabase::Delete()
{
	if (!DeleteFile(_dbFile.c_str()))
	{
		const DWORD err = GetLastError();
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

/**
 * Compress the database
 */

void SqliteDatabase::Vacuum()
{
	Execute("VACUUM;");
}

/**
 * Check if a table exists
 */

bool SqliteDatabase::TableExists(const char* table)
{
	SqliteStatement stmt(this, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = @name;");
	stmt.Bind("@name", table);
	stmt.GetNextRecord();
	const int count = stmt.GetIntColumn(0);
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

/**
 * Execute an SQL statement without results, like \c UPDATE, \c INSERT, \c DELETE, etc.
 */

void SqliteDatabase::Execute(LPCSTR szSQL)
{
	if (sqlite3_exec(_db, szSQL, nullptr, nullptr, nullptr) != SQLITE_OK)
		throw sqlite3_errmsg(_db);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::SetUserVersion(long version, const char* dbname)
{
	char sql[MAX_PATH];
	snprintf(sql, MAX_PATH, "PRAGMA %s.user_version = %ld;", dbname == nullptr ? "main" : dbname, version);
	Execute(sql);
}

/////////////////////////////////////////////////////////////////////////////
//

long SqliteDatabase::GetUserVersion(const char* dbname)
{
	std::string sql = "PRAGMA ";
	sql += (dbname == nullptr ? "main" : dbname);
	sql += ".user_version;";

	SqliteStatement stmt(this, sql.c_str());
	stmt.GetNextRecord();
	return stmt.GetIntColumn(0);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteDatabase::EnableForeignKeys(bool on)
{
	std::string sql = "PRAGMA foreign_keys = ";
	sql += (on ? "ON" : "OFF");
	Execute(sql.c_str());
}

/////////////////////////////////////////////////////////////////////////////
//

SqliteStatement::SqliteStatement(SqliteDatabase* db)
{
	_db = db->GetDB();
	_stmt = nullptr;
}

SqliteStatement::SqliteStatement(SqliteDatabase* db, const char* sql)
{
	_db = db->GetDB();
	_stmt = nullptr;
	Prepare(sql);
}

SqliteStatement::~SqliteStatement()
{
	if (_stmt != nullptr)
		sqlite3_finalize(_stmt);
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteStatement::Prepare(const char* sql)
{
	if (sqlite3_prepare_v2(_db, sql, -1, &_stmt, nullptr) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
	_colNames.clear();
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteStatement::Reset()
{
	if (sqlite3_reset(_stmt) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/////////////////////////////////////////////////////////////////////////////
//

void SqliteStatement::SaveRecord()
{
	if (sqlite3_step(_stmt) != SQLITE_DONE)
		throw SqliteException(sqlite3_errmsg(_db));

	Reset();
}

/////////////////////////////////////////////////////////////////////////////
//

bool SqliteStatement::GetNextRecord()
{
	const int rc = sqlite3_step(_stmt);

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

	_stmt = nullptr;
	_colNames.clear();
}

/////////////////////////////////////////////////////////////////////////////
//

int SqliteStatement::GetBindParameterIndex(std::string col)
{
	return sqlite3_bind_parameter_index(_stmt, col.c_str());
}

/**
 * Binds a wchar string to the given parameter. Empty strings are bound as null.
 *
 * @param param The parameter to bind to
 * @param val The value to bind
 */

void SqliteStatement::Bind(const char* param, const WCHAR* val)
{
	Bind(GetBindParameterIndex(param), val);
}

void SqliteStatement::Bind(int col, const WCHAR* val)
{
	int res = SQLITE_OK;
	if (val == nullptr)
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

/**
 * Binds a char string to the given parameter. Empty strings are bound as null.
 *
 * @param param The parameter to bind to
 * @param val The value to bind
 */

void SqliteStatement::Bind(const char* param, const char *val)
{
	Bind(GetBindParameterIndex(param), val);
}

void SqliteStatement::Bind(int col, const char *val)
{
	int res = SQLITE_OK;
	if (val == nullptr)
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

/**
 * Bind an integer to the given parameter.
 *
 * If the optional "bool null" is true, \c NULL is bound.
 * It can be used like this: <tt>stmt.Bind("col", var, var == 0);</tt>
 */

void SqliteStatement::Bind(const char* param, int val, bool null)
{
	Bind(GetBindParameterIndex(param), val, null);
}

void SqliteStatement::Bind(int col, int val, bool null)
{
	const int res = (null ? sqlite3_bind_null(_stmt, col) : sqlite3_bind_int(_stmt, col, val));
	if (res != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/**
 * Binds \c 1 for \c true and \c 0 for \c false to the given parameter
 */

void SqliteStatement::Bind(const char* param, bool val)
{
	Bind(GetBindParameterIndex(param), val);
}

void SqliteStatement::Bind(int col, bool val)
{
	if (sqlite3_bind_int(_stmt, col, val ? 1 : 0) != SQLITE_OK)
		throw SqliteException(sqlite3_errmsg(_db));
}

/**
 * Bind \c NULL to the given parameter
 */

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
	const LPCSTR val = (LPCSTR) sqlite3_column_text(_stmt, col);
	return (val == nullptr ? "" : val);
}

std::string SqliteStatement::GetTextColumn(std::string col)
{
	return GetTextColumn(_colNames[col]);
}

std::wstring SqliteStatement::GetWTextColumn(int col)
{
	const LPCWSTR val = (LPCWSTR) sqlite3_column_text16(_stmt, col);
	return (val == nullptr ? L"" : val);
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
