/**
 * @file
 * Disables the "Download updates" button if no project was selected.
 */
(function ($) {

"use strict";

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

})(jQuery);
