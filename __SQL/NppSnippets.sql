-----------------------------------------------------------------------------
-- At least SQLite 3.6.19 is needed because of foreign key support         --
-- See ReadMe.pdf for more details about the schema                        --
-----------------------------------------------------------------------------

PRAGMA foreign_keys = ON;

-----------------------------------------------------------------------------
-- TABLE Library                                                           --
-----------------------------------------------------------------------------
DROP TABLE IF EXISTS Library;
CREATE TABLE Library (
	LibraryID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	Name TEXT NOT NULL,
	CreatedBy TEXT,
	Comments TEXT,
	SortBy INTEGER NOT NULL DEFAULT 0
);

INSERT INTO Library(LibraryID, Name) VALUES(1,'General');

-----------------------------------------------------------------------------
-- TABLE LibraryLang                                                       --
-----------------------------------------------------------------------------
DROP TABLE IF EXISTS LibraryLang;
CREATE TABLE LibraryLang (
	LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,
	Lang INTEGER NOT NULL,
	PRIMARY KEY (LibraryID, Lang)
);

INSERT INTO LibraryLang(LibraryID, Lang) VALUES(1, -1);

-----------------------------------------------------------------------------
-- TABLE Snippets                                                          --
-----------------------------------------------------------------------------
DROP TABLE IF EXISTS Snippets;
CREATE TABLE Snippets (
	SnippetID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE,
	Name TEXT NOT NULL,
	BeforeSelection TEXT NOT NULL,
	AfterSelection TEXT,
	ReplaceSelection BOOL NOT NULL DEFAULT 0,
	NewDocument BOOL NOT NULL DEFAULT 0,
	NewDocumentLang INTEGER,
	Sort INTEGER
);

CREATE INDEX SnipName ON Snippets(LibraryID, Name, Sort);
CREATE INDEX SnipSort ON Snippets(LibraryID, Sort, Name);

INSERT INTO Snippets(LibraryID, Name, BeforeSelection) VALUES (1, 'Test Item 1', '11');

-----------------------------------------------------------------------------
-- TABLE LangLastUsed                                                          --
-----------------------------------------------------------------------------
DROP TABLE IF EXISTS LangLastUsed;
CREATE TABLE LangLastUsed (
	Lang INTEGER PRIMARY KEY NOT NULL,
	LibraryID INTEGER NOT NULL REFERENCES Library(LibraryID) ON DELETE CASCADE ON UPDATE CASCADE
);

-----------------------------------------------------------------------------
-- Set the user version of this database                                   --
-----------------------------------------------------------------------------
PRAGMA user_version = 3;
