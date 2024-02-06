(function ($) {

Backdrop.behaviors.menuAdminFieldsetSummaries = {
  attach: function (context) {
    var $context = $(context);
    $context.find('#edit-menu').backdropSetSummary(function () {
      var $enabledMenus = $context.find('.form-item-menu-options input:checked');
      if ($enabledMenus.length) {
        var vals = [];
        $enabledMenus.each(function(n, checkbox) {
          vals.push($(checkbox).siblings('label').text());
        });
        return vals.join(', ');
      }
      else {
        return Backdrop.t('Disabled');
      }
    });
  }
};

})(jQuery);
