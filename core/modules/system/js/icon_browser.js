(function($) {

  /**
   * Icon Browser behavior.
   *
   * Add indications on all activated icons (those that have been added to the
   * page and are available for CSS and JS use) in browser list and add info
   * to details as well.
   */
  Backdrop.behaviors.iconBrowser = {
    attach: function (context, settings) {
      const iconList = document.querySelectorAll(".icon-browser-list .icon-wrapper");
      // Indicate which icons are active.
      iconList.forEach((iconItem) => {
        var iconName = iconItem.getAttribute('data-icon-name');
        if (iconName && Backdrop.icons[iconName]) {
          iconItem.setAttribute('data-icon-active', true);
        }
      });
      const iconDetail = document.querySelector(".icon-browser-detail");
      if (iconDetail) {
        const iconStatus = document.querySelector(".icon-browser-detail .icon-status span");
        var iconName = iconDetail.getAttribute('data-icon-name');
        if (iconName && Backdrop.icons[iconName]) {
          iconDetail.setAttribute('data-icon-active', true);
          iconStatus.innerHTML = Backdrop.t("Active");
        }
      }
    }
  };

})(jQuery);
