Layouts divide pages on your site into different regions where content can be placed. You can use layouts contributed by others or create your own.

WHAT TO PLACE IN THIS DIRECTORY
-------------------------------

Placing downloaded and custom layouts in this directory separates them from
Backdrop core's layouts. This allows Backdrop core to be updated without overwriting these files.

DOWNLOAD ADDITIONAL LAYOUTS
---------------------------

Contributed layouts from the Backdrop community may be downloaded at
https://backdropcms.org/layouts.

ORGANIZING LAYOUTS IN THIS DIRECTORY
------------------------------------

It is safe to organize layouts into subdirectories to ensure easy maintenance
and upgrades.

MULTISITE CONFIGURATION
-----------------------

In multisite configuration, layouts found in this directory are available to all
sites. In addition to this directory, shared common layouts may also be kept in
the sites/all/layouts directory and layouts there will take precedence over layouts in this directory. Alternatively, the sites/your_site_name/layouts
directory pattern may be used to restrict layouts to a specific site instance.

MORE INFORMATION
-----------------

Refer to the "Developing Layouts" section of the developer documentation on the
Backdrop API website for further information on creating custom layouts: https://api.backdropcms.org/developing-layouts.
