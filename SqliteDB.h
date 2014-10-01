/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// SqliteDB Classes                                                        //
// Version 1.0.1, 30-Oct-2013                                              //
//                                                                         //
// Copyright (c) 2013, Frank Fesevur <http://www.fesevur.com>              //
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

#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include "sqlite3.h"

typedef std::map<std::string, int> StrIntMap;

class SqliteException : public std::runtime_error
{
public:
	SqliteException(const std::string& errorMessage) : std::runtime_error(errorMessage)
	{
	}
};

class SqliteDatabase
{
public:
	SqliteDatabase();
	SqliteDatabase(LPCWSTR file);
	virtual ~SqliteDatabase();

	virtual void Open();
	virtual void Open(LPCWSTR file);
	void Close();
	void Delete();
	void Vacuum();

	void Attach(LPCWSTR file, LPCWSTR alias);
	void Detach(LPCWSTR alias);

	void SetFilename(LPCWSTR file);
	void SetUserVersion(long version);

	LPCWSTR GetFilename() { return _dbFile; };
	long GetUserVersion();
	bool TableExists(const char* table);
	sqlite3* GetDB() { return _db; };

	void EnableForeignKeys(bool on = true);

	void Execute(LPCSTR szSQL);
	void BeginTransaction();
	void CommitTransaction();
	void RollbackTransaction();

protected:
	WCHAR _dbFile[MAX_PATH];
	sqlite3* _db;
};

class SqliteStatement
{
public:
	SqliteStatement(SqliteDatabase* db);
	SqliteStatement(SqliteDatabase* db, const char* sql);
	virtual ~SqliteStatement();

	void Prepare(const char* sql);
	void SaveRecord();
	bool GetNextRecord();
	void Finalize();

	int GetColumnCount();
	std::string GetTextColumn(int col);
	std::string GetTextColumn(std::string col);
	std::wstring GetWTextColumn(int col);
	std::wstring GetWTextColumn(std::string col);
	int GetIntColumn(int col);
	int GetIntColumn(std::string col);
	bool GetBoolColumn(int col);
	bool GetBoolColumn(std::string col);
	int GetBindParameterIndex(std::string col);

	void Bind(const char* param, const WCHAR* val);
	void Bind(const char* param, const char *val);
	void Bind(const char* param, int val, bool null = false);
	void Bind(const char* param, bool val);
	void Bind(const char* param);

	void Bind(int col, const WCHAR* val);
	void Bind(int col, const char *val);
	void Bind(int col, int val, bool null = false);
	void Bind(int col, bool val);
	void Bind(int col);

protected:
	void ResolveColumnNames();

	sqlite3* _db;
	sqlite3_stmt* _stmt;
	StrIntMap _colNames;
};
