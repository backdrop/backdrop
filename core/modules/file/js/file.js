
/**
 * @file
 * Provides JavaScript additions to the managed file field type.
 *
 * This file provides progress bar support (if available), popup windows for
 * file previews, and disabling of other file fields during Ajax uploads (which
 * prevents separate file fields from accidentally uploading files).
 */

(function ($) {

  // Placeholder for image field name.
  var $thisImageFieldName = 'placeholder';

/**
 * Attach behaviors to managed file element upload fields.
 */
Backdrop.behaviors.fileUploadChange = {
  attach: function (context, settings) {
    $(context).find('input[data-file-extensions]').once('validate-extension').on('change', Backdrop.file.validateExtension);
    $(context).find('input[data-file-auto-upload]').once('auto-upload').on('change', Backdrop.file.autoUpload).each(function() {
      $(this).closest('.form-item').find('.file-upload-button').hide();
    });
  },
  detach: function (context, settings) {
    $(context).find('input[data-file-extensions]').off('change', Backdrop.file.validateExtension);
    $(context).find('input[data-file-auto-upload]').off('change', Backdrop.file.autoUpload);
  }
};

/**
 * Attach behaviors to the file upload and remove buttons.
 */
Backdrop.behaviors.fileButtons = {
  attach: function (context) {
    $('input.form-submit', context).once('file-disable-fields').bind('mousedown', Backdrop.file.disableFields);
    $('div.form-managed-file input.form-submit', context).once('file-progress-bar').bind('mousedown', Backdrop.file.progressBar);
  },
  detach: function (context) {
    $('input.form-submit', context).unbind('mousedown', Backdrop.file.disableFields);
    $('div.form-managed-file input.form-submit', context).unbind('mousedown', Backdrop.file.progressBar);
  }
};

/**
 * Attach behaviors to links within managed file elements.
 */
Backdrop.behaviors.filePreviewLinks = {
  attach: function (context) {
    $('div.form-managed-file .file a, .file-widget .file a', context).once('file-preview-link').bind('click', Backdrop.file.openInNewWindow);
  },
  detach: function (context){
    $('div.form-managed-file .file a, .file-widget .file a', context).unbind('click', Backdrop.file.openInNewWindow);
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
    if ($(this).closest('div.form-managed-file').length > 0) {
      $enabledFields = $(this).closest('div.form-managed-file').find('input.form-file');
    }

    // Temporarily disable upload fields other than the one we're currently
    // working with. Filter out fields that are already disabled so that they
    // do not get enabled when we re-enable these fields at the end of behavior
    // processing. Re-enable in a setTimeout set to a relatively short amount
    // of time (1 second). All the other mousedown handlers (like Backdrop's
    // Ajax behaviors) are excuted before any timeout functions are called, so
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
  }
};

/**
 * Modify display of image upload fields in node add/edit form.
 */
Backdrop.behaviors.ImageLibraryOption = {
  attach: function (context, settings) {
    // select image fields (there may be more than one).
    // for each image field, if there is an existing image
    // hide the select image library option.
    var $imageFields = $(".field-type-image");
    // Check whether this has already been done.
    if (!$imageFields.find(".image-widget").hasClass("image-library-option-processed")) {
      // Add a 'processed' class to prevent unnecessary repetition.
      var $imageWidgets = $imageFields.find(".image-widget").addClass("image-library-option-processed");
      $imageWidgets.each(function (addLibrary) {
        // check each image field for existence of an image file.
        var $thisImage = $(this);
        var $thisData = $thisImage.find(".image-widget-data");
        var $thisSize = $thisData.find("span.file-size").length;
        if ($thisSize > 0) {
          // if there is an image file
          // hide 'select from image library' option for this field.
          $thisImage.find(".image-library-option").hide();
          // hide image selection option div.
          $thisImage.find(".image-selection-option").hide();
        }
        else {
          // Add a mouseover function to the 'Open Library' button's wrapper
          // for this image.
          $thisImage.find(".fieldset-wrapper").mouseover(function () {
            // Find the field name for this image and assign to
            // placeholder for image field name.
            $thisImageFieldName = $thisImage.find('[name*= "[field_name]"]').val();
            // Find the FID field and add image field name as a class
            // in order to select it later.
            $thisImage.find('input[name$= "[fid]"]').addClass("current-image");
          });
          var $thisItem = $thisImage.parent().find("Label");
          var $selection = $thisItem.parent();
          var $fieldLabelText = $thisItem.text();
          var $newLabelText = '<div class="image-label">';
          var $addText = '<div class="image-upload">Upload image | </div><div class="image-reference">Reference existing | </div><div class="image-select">Select from image library</div>';
          $newLabelText = $newLabelText.concat($fieldLabelText,'</div>',$addText);
          $thisItem.replaceWith($newLabelText);
          // Add on click functions to the selection options.
          $selection.find(".image-upload").click(function () {
            $(".image-selection-option").hide();
            $(".form-file").show();
            $(".image-library-option").hide();
            $(".image-upload").css({"color" : "mediumblue"});
            $(".image-reference").css({"color" : "black"});
            $(".image-select").css({"color" : "black"});
          });
          $selection.find(".image-reference").click(function () {
            $(".image-selection-option").show();
            $(".form-file").hide();
            $(".image-library-option").hide();
            $(".image-upload").css({"color" : "black"});
            $(".image-reference").css({"color" : "mediumblue"});
            $(".image-select").css({"color" : "black"});
          });
          $selection.find(".image-select").click(function () {
            $(".image-selection-option").hide();
            $(".form-file").hide();
            $(".image-library-option").show();
            $(".image-upload").css({"color" : "black"});
            $(".image-reference").css({"color" : "black"});
            $(".image-select").css({"color" : "mediumblue"});
          });
        }
      });
    }
  }
};

  /**
 * Provide mouseover and click event functions for images in library.
 */
Backdrop.behaviors.ImageFieldDialog = {
  attach: function (context, settings) {
    // Listen for a dialog creation event.
    $(window).on('dialog:aftercreate', function() {
      // Add events to images appearing in library.
      var $galleryContainer = $(".image-library")
        // first process mouseover (hover)
        .on('mouseover', '.image-library-choose-file', function () {
          // Get values from view.
          var $currentImg = $(this).find('img');
          var $currentImgFid = $currentImg.data('fid');
          var $currentImgURI = $currentImg.data('file-url');
          var $relativeImgSrc = Backdrop.relativeUrl($currentImgURI);
          var $currentImgName = $currentImg.data('filename');
          var $currentImgSize = $currentImg.data('filesize');

          // Enter values in placeholders.
          $galleryContainer.find(".image-fid").text($currentImgFid);
          $galleryContainer.find(".image-uri").text($relativeImgSrc);
          $galleryContainer.find(".image-name").text($currentImgName);
          $galleryContainer.find(".image-size").text($currentImgSize);
          $galleryContainer.find(".field-name").text($thisImageFieldName);
        })

        // now process click (select).
        .on('click', '.image-library-choose-file', function () {
          // Enter src value in image confirmation form.
          // Get the value for the current image.
          $thisSrc = $galleryContainer.find(".image-uri").text();

          var $widgetdata = $(".l-wrapper").find(".image-widget-data");
          var $relevantFields = $widgetdata.find("input[name$='[fid]']");
          var $specificField = $relevantFields.find(".current-image");
          $specificField.removeClass('current-image');
          $galleryContainer.find(".field-src").find("input").val($thisSrc);
        });
    });
  }
};

  /**
   * Set field values in image confirmation form.
   */
  Backdrop.behaviors.ImageConfirmation = {
    attach: function (context, settings) {
      // Listen for a dialog creation event.
      $(window).on('dialog:aftercreate', function() {
        var $imageDetails = $(".image-preview-container");
        var $imageFID = $imageDetails.find("span.img-fid").text();
        var $imageSRC = $imageDetails.find("span.img-src").text();
        $imageDetails.find(".field-id").find("input").val($imageFID);
        $imageDetails.find(".field-src").find("input").val($imageSRC);
      });
    }
  };
})(jQuery);
