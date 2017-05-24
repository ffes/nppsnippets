How to install or upgrade
=========================

The easiest way to install this plugin is by using the Plugin Manager:
from the "Plugins" menu, choose the "Plugin Manager" and "Show Plugin
Manager". Select the "Snippets" plugin and choose "install" and follow
the instructions on the screen.

To manually install the plugin, copy ``NppSnippets.dll`` to the
``plugins`` directory and copy ``Template.sqlite`` to the
``plugins/config`` directory in the user's "Application Data" directory
and name it ``NppSnippets.sqlite``. On my Windows 7 machine this
directory is ``C:\Users\Frank\AppData\Roaming\Notepad++\plugins\config``.
Then (re)start Notepad++.

You can place the database in the ``plugins/config`` directory in the
Notepad++ installation directory typically in the ``Program Files``
directory, but this could result in a read-only database due to file
permissions and/or User Account Control (UAC). Therefore it is advised
to put it in your ``AppData`` directory.

	From `issue #9`_:
	I noticed when I change files with different languages of my progect it takes 2-3 seconds delay when plugin load snippets for this language (for example HTML to PHP).


You can also specify a custom path (like a shared network path or your
Dropbox directory) for your database by manually editing the
``NppSnippets.ini`` file and adding a ``DBFile`` entry pointing to the
full path name of the database. The ``NppSnippets.ini`` is normally in
your ``AppData`` directory.

When you automatically upgrade through the Plugin Manager your database
is not touched. If you are manually upgrading at least replace the
existing dll. It is up to you if you override your own database with the
template database provided in the archive. If you have not changed
existing snippets or added your own, it is recommended to use the
database in the archive.


.. _issue #9: https://github.com/ffes/nppsnippets/issues/9

