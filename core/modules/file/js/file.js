
/**
 * @file
 * Provides JavaScript additions to the managed file field type.
 *
 * This file provides progress bar support (if available), popup windows for
 * file previews, and disabling of other file fields during Ajax uploads (which
 * prevents separate file fields from accidentally uploading files).
 */

(function ($) {

/**
 * Attach behaviors to managed file element upload fields.
 */
Backdrop.behaviors.fileUploadChange = {
  attach: function (context, settings) {
    $(context).find('input[data-file-extensions]').once('validate-extension').on('change', Backdrop.file.validateExtension);
    $(context).find('input[data-file-auto-upload]').once('auto-upload').on('change', Backdrop.file.autoUpload).each(function() {
      $(this).closest('.form-item').find('.file-upload-button').hide();
    });
  }
};

/**
 * Attach behaviors to the file upload and remove buttons.
 */
Backdrop.behaviors.fileButtons = {
  attach: function (context) {
    $('input.form-submit', context).once('file-disable-fields').on('mousedown', Backdrop.file.disableFields);
    $('div.form-managed-file input.form-submit', context).once('file-progress-bar').on('mousedown', Backdrop.file.progressBar);
  }
};

/**
 * Attach behaviors to links within managed file elements.
 */
Backdrop.behaviors.filePreviewLinks = {
  attach: function (context) {
    $('.file-preview-link', context).once('file-preview-link').on('click', Backdrop.file.openInNewWindow);
  },
  detach: function (context){
    $('.file-preview-link', context).off('click', Backdrop.file.openInNewWindow);
  }
};

/**
 * Attach behaviors to Vertical tabs on file administration pages.
 */
Backdrop.behaviors.fileFieldsetSummaries = {
  attach: function (context) {
    $('fieldset.file-form-destination', context).backdropSetSummary(function (context) {
      var scheme = $('.form-item-scheme input:checked', context).parent().text();
      return Backdrop.t('Destination: @scheme', { '@scheme': scheme });
    });
    $('fieldset.file-form-user', context).backdropSetSummary(function (context) {
      var name = $('.form-item-name input', context).val() || Backdrop.settings.anonymous;
      return Backdrop.t('By @name', { '@name': name });
    });
  }
};

/**
 * File upload utility functions.
 */
Backdrop.file = Backdrop.file || {
  /**
   * Client-side file input validation of file extensions.
   */
  validateExtension: function (event) {
    // Add client side validation for the input[type=file].
    var extensionList = $(this).data('file-extensions');
    var extensionPattern = extensionList.replace(/,\s*/g, '|');
    if (extensionPattern.length > 1 && this.value.length > 0) {
      // Remove any previous errors.
      $('.file-upload-js-error').remove();

      var acceptableMatch = new RegExp('\\.(' + extensionPattern + ')$', 'gi');
      if (!acceptableMatch.test(this.value)) {
        var error = Backdrop.t("The selected file %filename cannot be uploaded. Only files with the following extensions are allowed: %extensions.", {
          // According to the specifications of HTML5, a file upload control
          // should not reveal the real local path to the file that a user
          // has selected. Some web browsers implement this restriction by
          // replacing the local path with "C:\fakepath\", which can cause
          // confusion by leaving the user thinking perhaps Backdrop could not
          // find the file because it messed up the file path. To avoid this
          // confusion, therefore, we strip out the bogus fakepath string.
          '%filename': this.value.replace('C:\\fakepath\\', ''),
          '%extensions': extensionPattern.replace(/\|/g, ', ')
        });
        $(this).closest('div.form-managed-file').prepend('<div class="messages error file-upload-js-error" aria-live="polite">' + error + '</div>');
        this.value = '';
        event.filePreValidation = false;
        return false;
      }
      else {
        event.filePreValidation = true;
      }
    }
  },
  /**
   * Automatically upload files by clicking the Upload button on file selection.
   */
  autoUpload: function (event) {
    // This value is set in Backdrop.file.validateExtension().
    if (event.filePreValidation === undefined || event.filePreValidation === true) {
      $(this).closest('.form-item').find('.file-upload-button').trigger('mousedown').trigger('mouseup').trigger('click');
    }
  },
  /**
   * Prevent file uploads when using buttons not intended to upload.
   */
  disableFields: function (event){
    var clickedButton = this;

    // Only disable upload fields for Ajax buttons.
    if (!$(clickedButton).hasClass('ajax-processed')) {
      return;
    }

    // Check if we're working with an "Upload" button.
    var $enabledFields = [];
    if ($(this).is('.file-upload-button')) {
      $enabledFields = $(this).closest('div.form-managed-file').find('input.form-file');
    }

    // Temporarily disable upload fields other than the one we're currently
    // working with. Filter out fields that are already disabled so that they
    // do not get enabled when we re-enable these fields at the end of behavior
    // processing. Re-enable in a setTimeout set to a relatively short amount
    // of time (1 second). All the other mousedown handlers (like Backdrop's
    // AJAX behaviors) are executed before any timeout functions are called, so
    // we don't have to worry about the fields being re-enabled too soon.
    // @todo If the previous sentence is true, why not set the timeout to 0?
    var $fieldsToTemporarilyDisable = $('div.form-managed-file input.form-file').not($enabledFields).not(':disabled');
    $fieldsToTemporarilyDisable.prop('disabled', true);
    setTimeout(function (){
      $fieldsToTemporarilyDisable.prop('disabled', false);
    }, 1000);
  },
  /**
   * Add progress bar support if possible.
   */
  progressBar: function (event) {
    var clickedButton = this;
    var $progressId = $(clickedButton).closest('div.form-managed-file').find('input.file-progress');
    if ($progressId.length) {
      var originalName = $progressId.attr('name');

      // Replace the name with the required identifier.
      $progressId.attr('name', originalName.match(/APC_UPLOAD_PROGRESS|UPLOAD_IDENTIFIER/)[0]);

      // Restore the original name after the upload begins.
      setTimeout(function () {
        $progressId.attr('name', originalName);
      }, 1000);
    }
    // Show the progress bar if the upload takes longer than half a second.
    setTimeout(function () {
      $(clickedButton).closest('div.form-managed-file').find('div.ajax-progress-bar').slideDown();
    }, 500);
  },
  /**
   * Open links to files within forms in a new window.
   */
  openInNewWindow: function (event) {
    $(this).attr('target', '_blank');
    window.open(this.href, 'filePreview', 'toolbar=0,scrollbars=1,location=1,statusbar=1,menubar=0,resizable=1,width=500,height=550');
    return false;
  },

  /**
   * Provide events for files in the file browser dialog.
   */
  dialogOpenEvent: function(e, dialog, $element, settings) {
    var $browserContainer = $element.find(".file-browser");
    $browserContainer.once('file-browser').on('click', '[data-fid]', function () {
      var $selectedElement = $(this);
      if ($selectedElement.is('img')) {
        $browserContainer.find('.image-library-image-selected').removeClass('image-library-image-selected');
        $selectedElement.parent('.image-library-choose-file').addClass('image-library-image-selected');
      }
      else {
        $browserContainer.find('.file-browser-selected').removeClass('file-browser-selected');
        $selectedElement.parent('.file-browser-file').addClass('file-browser-selected');
      }
      var selectedFid = $(this).data('fid');
      // Set the FID in the modal submit form.
      $('form.file-managed-file-browser-form [name="fid"]').val(selectedFid);
    }).on('dblclick', '.image-library-choose-file', function() {
      var $selectedElement = $(this);
      $selectedElement.click();
      var $form = $selectedElement.closest('.ui-dialog-content').find('form');
      var $submit = $form.find('.form-actions input[type=submit]:first');
      $submit.trigger('mousedown').trigger('click').trigger('mouseup');
    });
  },

  /**
   * After closing a dialog, check if the file ID needs to be updated..
   */
  dialogCloseEvent: function(e, dialog, $element) {
    var $browserContainer = $element.find(".file-browser");
    if ($browserContainer.length > 0) {
      // These two variables are set server-side when submitting the dialog, in
      // file_managed_file_browser_submit().
      var selectedFid = Backdrop.settings.file.browser.selectedFid;
      var $fidElement = $(Backdrop.settings.file.browser.currentFidElement);

      var $parentElement = $fidElement.closest('.form-type-managed-file');
      var $fileInputField = $parentElement.find('input[type="file"]');
      var $uploadButton = $parentElement.find('.file-upload-button');

      if ($fidElement.length && selectedFid) {
        // Clear any selected file (in the event it was selected before opening).
        $fileInputField.val('');

        // Set this hidden FID value to the selected file.
        $fidElement.val(selectedFid);

        // Then click the "Upload" button, which will utilize the given file.
        $uploadButton
          .trigger('mousedown')
          .trigger('click')
          .trigger('mouseup');
      }
    }
  }
};

/**
 * Attach dialog behaviors for the file browser.
 */
$(window).on('dialog:aftercreate.fileBrowser', Backdrop.file.dialogOpenEvent);
$(window).on('dialog:afterclose.fileBrowser', Backdrop.file.dialogCloseEvent);

})(jQuery);
