.. _options:

Options
=======

To edit the settings of the plug-in, select ``Options...`` from the ``Plugin`` menu.

You can also edit the ini-file manually to change the settings.
This ini-file is normally found in your "Application Data" directory and is named ``NppSnippets.ini``.
On my Windows 10 machine this directory is ``C:\Users\Frank\AppData\Roaming\Notepad++\plugins\config``.

These are the default settings:

.. code:: ini

    [Options]
    Show=1
    ToolbarIcon=1
    Indent=1
    DBFile=

When ``Show`` is set to ``0`` the tree will not be shown.

When ``ToolbarIcon`` is set to ``0`` no icon will be shown on the toolbar.

When ``Indent`` is set to ``0`` the snippet will not be indented when it is inserted.

``DBFile`` can be used to override the default location of the database ``NppSnippets.sqlite`` by specifying the full path name of the database.
When it is not set the plug-in will look in the same directory as where ``NppSnippets.ini`` is located.
