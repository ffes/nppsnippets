Libraries and Snippets
======================

.. _edit:

How to add or edit libraries or snippets
----------------------------------------

To add or edit a snippet, right-click the snippet in the list and edit.
To add or edit a library right-click the combo-box where you select the
current library.

A library can be useful for multiple programming languages. You can specify
for which language the library is visible.

To add a lot of snippets, I suggest that you look at the section
:ref:`database` that describes the database structure and import your
data with your favorite SQLite management tool.


Importing and exporting libraries
---------------------------------

NppSnippets has an option to import and export libraries. When you right-click
the combo-box where the current library is selected, you have two options.
When you import a library, select the database you want to import from.
After that a dialog appears that lets you select the library to import.
Note that the imported library could a for a different programming language
then you are currently using. Therefore the library may not appear at first.

When you export the current library, you can create a new database or you can
export to an existing database. This way you can combine various libraries
for one programming language in one database.

Note that when you want to share libraries with other users with your
organization there is also :ref:`an option <options>` to specify the full
path to the database. Maybe you want to put one database on a network share.


Provided Snippets Libraries
---------------------------

At this moment the template database is filled with these libraries. If
you upgrade from a previous version your databases is not changed. If
you want to try any of the new libraries, you must manually import those
libraries from the template database.

+----------------------+--------------------+------------+
| Library Name         | Languages          | By         |
+======================+====================+============+
| ANSI Characters      | All                | FFes       |
+----------------------+--------------------+------------+
| Templates            | All                | FFes       |
+----------------------+--------------------+------------+
| HTML Tags            | HTML, PHP, ASP     | FFes       |
+----------------------+--------------------+------------+
| HTML Characters      | HTML, PHP, ASP     | FFes       |
+----------------------+--------------------+------------+
| Greek Characters     | HTML, PHP, ASP     | FFes       |
+----------------------+--------------------+------------+
| W3C Doctypes         | HTML               | FFes       |
+----------------------+--------------------+------------+
| CSS2 Tags            | CSS, HTML          | FFes       |
+----------------------+--------------------+------------+
| CSS2 Tags & Values   | CSS, HTML          | FFes       |
+----------------------+--------------------+------------+
| JavaScript - Basic   | JavaScript, HTML   | FFes       |
+----------------------+--------------------+------------+
| JavaScript - Date    | JavaScript, HTML   | FFes       |
+----------------------+--------------------+------------+
| JavaScript - Math    | JavaScript, HTML   | FFes       |
+----------------------+--------------------+------------+
| PHP Language         | PHP                | jvdanilo   |
+----------------------+--------------------+------------+
| XML Tags             | XML                | jvdanilo   |
+----------------------+--------------------+------------+

User generated libraries
------------------------

From version 0.7 it is possible to import libraries from another
NppSnippets database. This way redistributing user generated libraries
becomes easy. If you have created your own library and think it can be
useful for others, export that library and :ref:`send it to me <contact>`.
I will put them on `my website`_.

.. _my website: http://www.fesevur.com/nppsnippets
