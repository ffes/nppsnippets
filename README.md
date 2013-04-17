NppSnippets
===========

NppSnippets is an easy to use snippet / template plug-in: Similarity with the TextPad Clip Library is no accident. To insert a snippet simply double click on the item in the list and the snippet is inserted at the current cursor position. To edit right-click on that item.

The project page, with downloads and issue tracker, is at https://code.google.com/p/nppsnippets/

Version 1.2.0 (8 January 2013)
* There was an inconsistency between the documentation and code about the name of the option to specify your custom path for the database. Use DBFile from now on. For backwards compatibility the DBPath entry will still be recognized.
* When a snippets creates a new document and the current document is empty, it reuses the current one and does not start a new.
* Added Duplicate snippet function to context menu.
* New (simple) templates library.
* Upgrade to SQLite version 3.7.15.1
