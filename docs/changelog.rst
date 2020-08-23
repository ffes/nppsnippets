Changelog
=========


Version `1.6.0`_ (23 August 2020)
---------------------------------

-  Indent the snippet when it is inserted. Since this is a new feature and
   I can't test it for all possible programming languages indenting
   may not always works as indented, so you can disable it in the :ref:`options`.

-  Make the plugin more keyboard friendly (`issue #31`_).
   It adds two menu items to the ``Plugins`` menu to set the focus to the lists of libraries and the lists snippets.
   In ``Settings``, ``Shortcut Mapper...`` you can assign your own keyboard shortcuts to these actions.
   Apart from these menu items the plugin is generally more keyboard friendly.

-  Add more images to the page :ref:`usage`.

-  Upgrade to SQLite version 3.33.0

.. _1.6.0: https://github.com/ffes/nppsnippets/releases/tag/v1.6.0
.. _issue #31: https://github.com/ffes/nppsnippets/issues/31


Version `1.5.1`_ (13 January 2020)
----------------------------------

-  Fix crash when ``Before Selection`` was left empty and a snippet was saved (`issue #16`_)

-  Make the Edit Snippet dialog bigger and add scrollbars (`PR #25`_)

-  Make sure to use the proper line endings when creating a snippet from selection or clipboard (`issue #27`_)

-  Upgrade to SQLite version 3.30.1

.. _1.5.1: https://github.com/ffes/nppsnippets/releases/tag/v1.5.1
.. _PR #25: https://github.com/ffes/nppsnippets/pull/25
.. _issue #16: https://github.com/ffes/nppsnippets/issues/16
.. _issue #27: https://github.com/ffes/nppsnippets/issues/27


Version `1.5.0`_ (15 May 2019)
------------------------------

-  Adapt to Notepad++ 7.6.x (`issue #20`_)

-  Fixed "Creating Snippet from Selection crashes Notepad++" (`issue #14`_)

-  Add entry to the plugin menu to open the online manual

-  Upgrade to SQLite version 3.27.1

.. _1.5.0: https://github.com/ffes/nppsnippets/releases/tag/v1.5.0
.. _issue #14: https://github.com/ffes/nppsnippets/issues/14
.. _issue #20: https://github.com/ffes/nppsnippets/issues/20


Version `1.4.0`_ (24 May 2017)
------------------------------

-  Provide a 64-bit version of the plug-in.

-  Added the possibility to :ref:`export a library <import_export_libs>` for easier sharing.

-  The color of the plug-in match the current Notepad++ theme.

-  Converted the documentation from DocBook to reStructuredText. The
   documentation is now hosted at `Read The Docs`_.

-  Fixed `bug #6`_ at Google Code and its GitHub duplicate `issue #8`_.
   When a snippet had an empty first line it could not be saved.

-  Removed all references to Google Code because that `service has retired`_.
   All things that were still on Google Code have been moved to `GitHub`_.

-  Internally use my SqliteDB-class to communicate with the database.

-  Added :ref:`option <options>` ``ToolbarIcon`` to hide the icon from the toolbar.

-  Update icon on toolbar. It is now a puzzle piece.

-  Fixed issue that sometimes new libraries and/or new snippets could
   not be added.

-  Upgrade to SQLite version 3.19.0

.. _1.4.0: https://github.com/ffes/nppsnippets/releases/tag/v1.4.0
.. _Read The Docs: http://nppsnippets.readthedocs.io
.. _service has retired: http://google-opensource.blogspot.com/2015/03/farewell-to-google-code.html
.. _GitHub: https://github.com/ffes/nppsnippets
.. _bug #6: https://code.google.com/archive/p/nppsnippets/issues/6
.. _issue #8: https://github.com/ffes/nppsnippets/issues/8


Version `1.3.0`_ (June 2013)
----------------------------

-  Fixed problem with inserting UTF snippets (`issue #3`_).

-  Fixed wrong title of Import Library dialog.

-  Fixed some potential bugs found when trying to fix GCC compilation.

-  Converted the documentation from ODT to DocBook. Because of that an
   `on-line version`_ of the documentation is available as well.

-  Upgrade to SQLite version 3.8.0.2

.. _1.3.0: https://github.com/ffes/nppsnippets/releases/tag/v1.3.0
.. _issue #3: http://code.google.com/archive/p/nppsnippets/issues/3
.. _on-line version: http://nppsnippets.readthedocs.io


Version `1.2.0`_ (8 January 2013)
---------------------------------

-  There was an inconsistency between the documentation and code about
   the name of the option to specify your custom path for the database.
   Use ``DBFile`` from now on. For backwards compatibility the ``DBPath``
   entry will still be recognized.

-  When a snippets creates a new document and the current document is
   empty, it reuses the current one and does not start a new.

-  Added ``Duplicate`` snippet function to context menu.

-  New (simple) templates library.

-  Upgrade to SQLite version 3.7.15.1

.. _1.2.0: https://github.com/ffes/nppsnippets/releases/tag/v1.2.0


Version 1.1.0 (13 December 2011)
--------------------------------

-  You can now add a new snippet to a library based upon the current
   selection or based upon the content of the clipboard. Right-click the
   snippets list to use these items.

-  Installation has been improved. A template database is provided and
   when the plug-in tries to find an existing database and it can't find
   it, it copies this template database to the AppData plugin-config
   directory.

-  The About dialog now shows the change-log.

-  When you upgrade the very first time the change-log for the current
   version will be shown.

-  When you didn't select a specific library for a certain language, the
   automatic selection of the library is improved. The first language
   specific library is preferred over the first general library.

-  Resized the edit snippet dialog.

-  Upgrade to SQLite version 3.7.9

-  Moved the download to `Google code`_. This gives me
   statistics about downloads and an issue tracker. The project's `web page`_
   stays where it is.

.. _Google code: https://code.google.com/p/nppsnippets/
.. _web page: http://www.fesevur.com/nppsnippets


Version 1.0.0 (6 September 2011)
--------------------------------

-  The selection or cursor position are now restored after inserting a
   snippet.

Version 0.7.1 (28 August 2011)
------------------------------

-  Fixed a bug in the dialog to edit the languages for a certain
   library. This bug could cause a problem that libraries turn
   invisible, since all the records in LibraryLang table for that
   library were deleted and no new records were added.

-  Added a JavaScript - Math library.

Version 0.7.0 (1 August 2011)
-----------------------------

-  A user interface for editing the language selection for libraries has
   been added. You need at least Notepad++ version 5.93 for this
   feature.

-  You can import a library from another NppSnippet database.

-  Start a new document for a certain snippets, and allow that snippet
   to set the language of that new document. There were already fields
   in the database for this. It can be very useful to start a new
   CSS-file or JavaScript-file from HTML, etc.

-  Added an option DBPath to the ini-file to override the default
   location of the database. Made this mainly for my own testing, but
   maybe it is useful for others as well (corporate database). You need
   to manually edit the ini-file to use this.

-  Added an icon to the tab of the docking interface.

-  Upgrade to SQLite version 3.7.7.1

Version 0.6.0 (15 June 2011)
----------------------------

-  It is now possible to add, edit or delete the snippets and the
   libraries from within Notepad++. It is not yet possible to edit the
   languages for a library.

-  Added a new ANSI-characters library for all languages.

-  Deleted the useless General library.

-  Upgrade to SQLite version 3.7.6.3

Version 0.5.0 (21 December 2010)
--------------------------------

-  Upgrade to SQLite version 3.7.4

-  The focus is returned to the Scintilla window at start-up and after
   inserting a snippet.

-  The plug-in remembers if it is shown.

-  Added another special language to the table LibraryLang: ``Lang = -2``.
   Libraries with this language will always be shown for all languages.

-  The languages last used is now stored in a separate table. The
   database schema version is 3. With this the libraries with special
   languages (negative language ID's) can be remembered as last used as
   well.

-  Error message when the database can not be opened.

Version 0.4.0 (8 April 2010)
----------------------------

-  The database schema is updated and is now at version 2. The most
   important difference is that the "language" and "last used" field of
   the library are now in a separate table, allowing it to be
   one-to-many. Existing databases will be converted automatically to
   the new schema.

-  When there are multiple libraries for a language, changing to another
   library works and the last used library is remembered.

-  The plug-in now first tries to find the database in the user's
   plug-in config directory. On my Windows XP machine that is
   ``C:\\Documents and Settings\\Frank\\Application Data\\Notepad++\\plugins\\config``.
   If the database can't be found there it looks in the ``plugin\\config`` directory
   in the Notepad++ installation directory, in my case
   ``C:\\Program Files\\Notepad++\\plugins\\Config``.

-  Small improvements to the About dialog.

-  There are now 4 HTML libraries, 1 PHP libraries, 1 XML library and 1
   (rather useless) General library.

Version 0.3.0 (10 February 2010)
--------------------------------

-  First alpha version, released under the GPL2 license.

-  The basics work, no User Interface yet to edit the snippets.

Version 0.1.0 (22 January 2010)
-------------------------------

-  Internal proof of concept.
