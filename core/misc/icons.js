(function ($) {

  /**
   * Makes icons available to JavaScript and CSS files.
   */
  Backdrop.behaviors.icons = {
    attach: function (context, settings) {
      if (!Backdrop.icons) {
        Backdrop.icons = [];
      }
      if (settings.icons) {
        for (const iconName in settings.icons) {
          const iconPath = settings.icons[iconName];
          Backdrop.addIcon(iconName, iconPath);
        }
      }
    }
  };

  /**
   * Add an individual icon to the Backdrop.icons namespace.
   *
   * This also makes the icon path available within CSS files via a CSS
   * variable by the name --icon-[icon-name].
   */
  Backdrop.addIcon = function (iconName, iconPath) {
    Backdrop.icons[iconName] = iconPath;
    document.documentElement.style.setProperty('--icon-' + iconName, 'url(' + iconPath + ')');
  };

})(jQuery);
