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

#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "sqlite3.h"

extern bool OpenDB();
extern bool CloseDB();
extern bool CheckDBVersion();
extern bool RunSQL(LPCSTR szSQL);
extern bool GetLongResult(LPCSTR szStmt, long& result);
extern bool VacuumDB();
extern bool BindText(sqlite3_stmt *stmt, const char* param, const WCHAR* var);
extern bool BindInt(sqlite3_stmt *stmt, const char* param, const int i, bool null = false);
extern WCHAR g_dbFile[MAX_PATH];
extern sqlite3* g_db;

extern map<string,int> ResolveColumnNames(sqlite3_stmt* stmt);

#endif // __DATABASE_H__
