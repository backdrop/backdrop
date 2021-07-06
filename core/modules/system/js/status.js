(function ($) {

/**
 * Toggles the more/less links that show/hide details in the "Status report"
 * page.
 */
Backdrop.behaviors.statusReport = {
  attach: function() {
    var $table = $('table.system-status-report');

    // Hide the description for non-urgent items.
    $table.find('tr.report-info').find('.status-description').hide();
    $table.find('tr.report-status').find('.status-description').hide();

    // Change the text on more-urgent items.
    $table.find('tr.report-error').find('a.description-toggle').text(Backdrop.t('less'));
    $table.find('tr.report-warning').find('a.description-toggle').text(Backdrop.t('less'));

    // Toggle the requirements info.
    $('a.description-toggle').click(function(e) {
      var $description = $(this).closest('td').find('.status-description').toggle();
      if ($description.is(':visible')) {
        $(this).text(Backdrop.t('less'));
      }
      else {
        $(this).text(Backdrop.t('more'));
      }
      e.preventDefault();
      e.stopPropagation();
    });
  }
};

})(jQuery);
