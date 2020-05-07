Known Issues
============

-  Closing the snippets window with the ``x`` does not update toolbar icon
   and menu item. And as a result of that the state is not remembered
   properly. I need to find the notification that is send to the window
   on clicking the ``x``  (`#26`_).

-  When a library is not alphabetic sorted, and you move an item up or
   down, this is slow. This is because the ``Sort``-field of all the
   snippets of that library need to be updated in the database.

-  The icon on the docking tab is inverted for some strange reason.

-  When there is no write-access to the database, this is not properly handled.

.. _#26: https://github.com/ffes/nppsnippets/issues/26
