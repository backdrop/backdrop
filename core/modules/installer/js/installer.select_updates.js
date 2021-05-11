(function ($) {

"use strict";

/**
 * Disables the "Download updates" button if no project was selected.
 */
Backdrop.behaviors.installerDownloadUpdatesToggle = {
  attach: function (context) {
    var $context = $(context);
    var $downloadButton = $('#installer-download-updates');
    var buttonText = $downloadButton.val();

    // Check if at least one update checkbox has been ticked. Then update the
    // disabled state and the label of the "Download updates" button
    // accordingly.
    function toggleExecuteButton (event) {
      var $rowChecked = $context.find('.form-checkbox:checked').length;
      var $allSelected = $context.find('.select-all .form-checkbox:checked').length;
      var newButtonText = $allSelected ? Backdrop.t('Download all updates') : buttonText;
      if ($rowChecked) {
        $downloadButton.attr('disabled', false).removeClass('no-js-hide form-button-disabled').prop('value', newButtonText);
      }
      else {
        $downloadButton.attr('disabled', 'disabled').addClass('no-js-hide form-button-disabled').prop('value', Backdrop.t('No update selected'));
      }
    }

    // Initialize the "Download updates" button once when the page loads.
    toggleExecuteButton();
    // Repeat each time any checkbox is ticked/unticked.
    $(context).find('.form-checkbox').on('change', function () {
      toggleExecuteButton();
    });
  }
};

/**
 * Toggles the more/less links that show/hide details in the "System updates"
 * page.
 */
Backdrop.behaviors.systemUpdates = {
  attach: function() {
    var $table = $('table.table-select-processed');

    // Hide the manual core update info text.
    $table.find('tr.core-manual-update').find('.core-manual-update-info').hide();

    // Toggle the info text.
    $('a.core-manual-update-info-toggle').click(function(e) {
      var $description = $(this).closest('td').find('.core-manual-update-info').toggle();
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
