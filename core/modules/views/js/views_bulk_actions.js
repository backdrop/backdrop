/**
 * @file
 * This behavior is required when the Views UI module is disabled.
 *
 * @see views_form()
 */
(function ($) {

"use strict";

Backdrop.behaviors.viewsBulkFormApplyToggle = {
  attach: function (context) {
    var $context = $(context);
    var $applyButton = $('#views-bulk-form-apply');
    var buttonText = $applyButton.val();

    // Check if at least one checkbox has been ticked, and also if an action has
    // been selected. Update the disabled state of the "Apply" button, and its
    // text accordingly.
    function toggleExecuteButton (event) {
      var $rowChecked = $context.find('.form-checkbox:checked').length;
      var $actionSelected = $('#edit-action').val();
      if ($rowChecked && $actionSelected) {
        $applyButton.attr('disabled', false).removeClass('no-js-hide form-button-disabled').prop('value', buttonText);
      }
      else {
        var newButtonText = $rowChecked ? Backdrop.t('No action selected') : Backdrop.t('No item selected');
        $applyButton.attr('disabled', 'disabled').addClass('no-js-hide form-button-disabled').prop('value', newButtonText);
      }
    }

    // Initialize the "Apply" button once when the page loads.
    toggleExecuteButton();
    // Repeat each time any checkbox is ticked/unticked.
    $(context).find('.form-checkbox').on('change', function () {
      toggleExecuteButton();
    });
    // Repeat each time the value of the "Action" select changes.
    $(context).find('#edit-action').on('change', function () {
      toggleExecuteButton();
    });
  }
};

})(jQuery);
