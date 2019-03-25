/**
 * Provides summary text for the "Administration theme" fieldset, in the "List
 * themes" page (/admin/appearance).
 */
 (function ($) {

"use strict";

Backdrop.behaviors.systemFieldsetSummaries = {
  attach: function (context) {
    $(context).find('fieldset.admin-theme-form').backdropSetSummary(function (element) {
      var $element = $(element);
      var admin_theme  = $element.find('[name="admin_theme"]').val();
      var node_admin_theme = $element.find('[name="node_admin_theme"]').prop('checked');

      if (node_admin_theme) {
        return Backdrop.t('@admin_theme - also used when editing or creating content', { '@admin_theme': admin_theme });
      }
      if (admin_theme == 0) {
        return Backdrop.t('Same theme as the rest of the site');
      }
      else {
        return admin_theme;
      }
    });
  }
};

})(jQuery);
