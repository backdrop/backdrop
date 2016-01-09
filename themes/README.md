Themes are skins for your site that allow you to change the look, feel, and general appearance. You can use themes contributed by others or create your own.

WHAT TO PLACE IN THIS DIRECTORY
-------------------------------

Placing downloaded and custom themes in this directory separates them from
Backdrop core's themes. This allows Backdrop core to be updated without
overwriting these files.

DOWNLOAD ADDITIONAL THEMES
--------------------------

Contributed themes from the Backdrop community may be downloaded at
https://www.backdropcms.org/themes

ORGANIZING MODULES IN THIS DIRECTORY
------------------------------------

It is safe to organize themes into subdirectories to ensure easy maintenance
and upgrades. Is highly recommended to use Backdrop's sub-theme functionality to keep the code for related themes separated.

MULTISITE CONFIGURATION
-----------------------

In multisite configuration, themes found in this directory are available to
all sites. In addition to this directory, shared common themes may also be kept
in the sites/all/themes directory and themes there will take precedence over
themes in this directory. Alternatively, the sites/your_site_name/themes
directory pattern may be used to restrict themes to a specific site instance.

MORE INFORMATION
-----------------

Refer to the "Developing Themes" section of the developer documentation on the
Backdrop API website for further information on customizing the appearance of
Backdrop with custom themes: https://api.backdropcms.org/developing-themes.
