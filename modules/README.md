Modules can add to the set of available features on your site, or change the way
existing features work. You can use modules contributed by others or create your
own.

WHAT TO PLACE IN THIS DIRECTORY
-------------------------------

Placing downloaded and custom modules in this directory separates them from
Backdrop's core modules. This allows Backdrop core to be updated without
overwriting these files.

DOWNLOAD ADDITIONAL MODULES
---------------------------

Contributed modules from the Backdrop community may be downloaded at http://backdropcms.org/modules.

ORGANIZING MODULES IN THIS DIRECTORY
------------------------------------

You may create subdirectories within this directory to organize your added modules. Some common subdirectories include "contrib" for contributed modules,
and "custom" for custom modules. Note that if you change the location of a
module after it has been enabled, you may need to clear the Backdrop cache so it
can be found. (Alternatively, you can disable the module before moving it and then re-enable it after the move.)

MULTISITE CONFIGURATION
-----------------------

In multisite configuration, modules found in this directory are available to
all sites. Shared common modules may also be kept in the sites/all/modules directory and modules there will take precedence over modules in this directory.
To restrict modules to a specific site instance, place modules in a directory following the pattern sites/your_site_name/modules.

MORE INFORMATION
----------------

Refer to the "Developing Modules" section of the developer documentation on the
Backdrop API website for further information on extending Backdrop with custom
modules: https://api.backdropcms.org/developing-modules.
