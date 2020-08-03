.. _database:

Database
========

NppSnippets uses a `SQLite`_ database named ``NppSnippets.sqlite`` to
store all the content.

In the archive is a file named ``NppSnippets.sql``. With this you can
generate a new (almost empty) SQLite database. At least SQLite 3.6.19 is
needed because a foreign key constrains are used and the plug-in will
need this when editing the data through the GUI. So be sure not to
create the database with an older SQLite management tool that breaks
this.

You should only update, insert or delete records and not modify the
structure of the database. That can cause the plug-in (and as a result
of that Notepad++ itself) to crash.

Why a SQLite database and not plain text files?
-----------------------------------------------

All the snippets are stored in one `SQLite`_ database. SQLite is fast and easy
to use. With SQLite most of the file and storage handling is taken care of.
This is much more efficient then designing my own (complex) file format and
implementing a parser and a writer, although I understand that editing a
SQLite database is not as easy to edit for some as editing plain text files.
But there is a :ref:`user interface <edit>` for editing the snippets.

.. _SQLite: https://www.sqlite.org/


Database structure
------------------

The database structure is quite simple. There are four tables
:ref:`table_library`, :ref:`table_library_lang`, :ref:`table_snippets`
and :ref:`table_lang_last_used`. There are one-to-many relations between
Library and LibraryLang, between Library and Snippets and between Library
and LangLastUsed. They are all linked to each other with the LibraryID field.
It is possible to have multiple libraries per language and one library can be
used for many languages. For every library at least one record in the
LibraryLang and Snippets tables is needed.

.. mermaid::

   erDiagram
      Library ||--o{ Snippets : has
      Library ||--o{ LibraryLang : has
      Library ||--|| LangLastUsed : has-one

The current schema version of the database is stored in  ``user_version`` and is 3.


Table definitions
-----------------

.. _table_library:

Library
*******

+-------------+------------------------------------------------------+
| Field       | Description                                          |
+=============+======================================================+
| LibraryID   | The unique identifier of this library                |
+-------------+------------------------------------------------------+
| Name        | Name of the library                                  |
+-------------+------------------------------------------------------+
| CreatedBy   | Who created this library                             |
+-------------+------------------------------------------------------+
| Comments    | Comments about this library                          |
+-------------+------------------------------------------------------+
| SortBy      | Which fields of Snippets are used to sort:           |
|             | 0. "Name, Sort"                                      |
|             | 1. "Sort, Name"                                      |
+-------------+------------------------------------------------------+

.. _table_library_lang:

LibraryLang
***********

+-------------+-------------------------------------------------------------------+
| Field       | Description                                                       |
+=============+===================================================================+
| LibraryID   | The library this item is part of                                  |
+-------------+-------------------------------------------------------------------+
| Lang        | LangType from Notepad\_plus\_msgs.h. There are two special cases: |
|             |                                                                   |
|             | -  Libs with Lang = -1 are shown when there is no library for     |
|             |    the current language                                           |
|             |                                                                   |
|             | -  Libs with Lang = -2 are shown for every language               |
+-------------+-------------------------------------------------------------------+

.. _table_snippets:

Snippets
********

+--------------------+--------------------------------------------------------------+
| Field              | Description                                                  |
+====================+==============================================================+
| SnippetID          | The unique identifier of this snippet                        |
+--------------------+--------------------------------------------------------------+
| LibraryID          | The library this item is part of                             |
+--------------------+--------------------------------------------------------------+
| Name               | The name of the snippet                                      |
+--------------------+--------------------------------------------------------------+
| BeforeSelection    | The text inserted before the current cursor / selection      |
+--------------------+--------------------------------------------------------------+
| AfterSelection     | The text inserted after the current cursor / selection       |
+--------------------+--------------------------------------------------------------+
| ReplaceSelection   | Replace a selection or ignore the selection and insert       |
+--------------------+--------------------------------------------------------------+
| NewDocument        | Create a new document before inserting this snippet?         |
+--------------------+--------------------------------------------------------------+
| NewDocumentLang    | Change the language of the new document to this language     |
+--------------------+--------------------------------------------------------------+
| Sort               | Can determine the order of the snipping in the list. Depends |
|                    | on Library(SortBy) how the list is sorted.                   |
+--------------------+--------------------------------------------------------------+

.. _table_lang_last_used:

LangLastUsed
************

+-------------+-------------------------------------------------------+
| Field       | Description                                           |
+=============+=======================================================+
| Lang        | LangType from Notepad\_plus\_msgs.h                   |
+-------------+-------------------------------------------------------+
| LibraryID   | This library is the last one used for this language   |
+-------------+-------------------------------------------------------+
