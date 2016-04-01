
(function ($) {

Backdrop.behaviors.redirectAdmin = {
  attach: function (context) {
    $('table.redirect-list-tableselect tbody input:checkbox').bind('change', function(context) {
      var checked = $('table.redirect-list-tableselect input:checkbox:checked').length;
      if (checked) {
        $('fieldset.redirect-list-operations').slideDown();
      }
      else {
        $('fieldset.redirect-list-operations').slideUp();
      }
    });
    $('table.redirect-list-tableselect th.select-all input:checkbox').bind('change', function(context) {
      var checked = $(this, context).attr('checked');
      if (checked) {
        $('fieldset.redirect-list-operations').slideDown();
      }
      else {
        $('fieldset.redirect-list-operations').slideUp();
      }
    });
    $('fieldset.redirect-list-operations').hide();
  }
};

})(jQuery);
