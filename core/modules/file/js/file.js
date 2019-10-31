
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
    // Ajax behaviors) are executed before any timeout functions are called, so
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

})(jQuery);

/**
 * Provides toggles for uploading an image, whether by URL or upload.
 * Derived from behavior in filter.js
 */
Backdrop.behaviors.ImageUploadDialog = {

  attach: function (context, settings) {
    // select image fields (there may be more than one).
    $imageFields = $('.field-type-image');
    // derive code for replacing label with pair of toggled links.
    $imageFields.each(function(addLibrary) {
      var $this = $(this);
      var $fieldID = $(this).attr('id');
      var str = '<div class="image-toggles"><a class="upload-image-toggle">Upload an image</a><a href="#';
      var $replacementLabel = str.concat($fieldID , '" class = "image-library-toggle">Select from Library</a></div>');


      // we need the internal system name of each image field,
      // e.g. field_image[und][0] or field_second_image[und][0]
      // select label element with attribute 'for' starting edit-field-image
      var str = '.field-widget-image-image ';
      var $xLabel = str.concat("label[for='", $fieldID , "-und-0']");
      $uploadLabel = $($xLabel);
      var $uploadLabelText = $uploadLabel.text();
      $this.find(".image-field-name").replaceWith($uploadLabelText);
      // replace Upload an image label with pair of toggled links.
      $uploadLabel.replaceWith($replacementLabel);

      // check value of fid.
      var $thisFid = $this.find('[name*= "[fid]"]').val();

      if ($thisFid > 0){
        // image already uploaded so show image-current part of form.
        $this.find(".image-toggles").hide();
        $this.find(".image-select").hide();
        $this.find(".image-upload").hide();
        $this.find(".image-confirm").hide();
        $this.find(".image-current").show();
        $this.find('[ID*= "library-confirm"]').hide();
        // replace current image preview.
        var $currentImgURI = $this.find('[name*= "[imagesrc]"]').val();
        $currentImgURI = '/files' + $currentImgURI.substring(8);
        var $relativeImgSrc = Backdrop.absoluteUrl($currentImgURI);
        $this.find('div[class= "selected-image-preview"] img').replaceWith('<img src="' + $relativeImgSrc + '">');
        $this.find(".description").hide();
        $this.find('[class*= "library-imagesrc"]').hide();
        $this.find('div[class*= "-title"]').hide();
        $this.find('div[class*= "-alt"]').hide();
      }
      else {
        // fid is not set so initialise with 'Upload an image' part of form.
        $this.find(".image-select").hide();
        $this.find(".image-confirm").hide();
        $this.find(".image-current").hide();
        $this.find(".image-widget").show();
        $this.find(".description").show();
        $this.find("a.upload-image-toggle").addClass("upload-selected");
        $this.find('[class*= "library-imagesrc"]').hide();
        $this.find('[ID*= "library-confirm"]').hide();
        $this.find('div[class*= "-alt"]').hide();
        $this.find('div[class*= "-title"]').hide();
      }

      // When 'Select from library' is clicked change to image library part of form.
      $this.find("a.image-library-toggle").on("click", ChangeToLibrary);
      function ChangeToLibrary() {
        $this.find(".image-select").show();
        $this.find(".image-upload").hide();
        $this.find(".image-confirm").hide();
        $this.find(".image-current").hide();
        $this.find("a.image-library-toggle").addClass("library-selected");
        $this.find("a.upload-image-toggle").removeClass("upload-selected");
        $this.find(".description").hide();

        // Set link 'Upload an image' for return when clicked:
        $this.find("a.upload-image-toggle").on("click", ChangeBack);
        function ChangeBack() {
          $this.find(".image-select").hide();
          $this.find(".image-upload").show();
          $this.find(".description").show();
          $this.find("a.image-library-toggle").removeClass("library-selected");
          $this.find("a.upload-image-toggle").addClass("upload-selected");
        }
      }

      // Now add events to images
      $this.find('.image-library').once('image-library')
      // first process mousoever (hover)
      .on('mouseover', '.image-library-choose-file', function () {
        var $currentImg = $(this).find('img');
        var $currentImgFid = $currentImg.data('fid');
        var $currentImgURI = $currentImg.data('file-url');
        var relativeImgSrc = Backdrop.relativeUrl($currentImgURI);
        var $currentImgName = $currentImg.data('filename');
        var $currentImgSize = $currentImg.data('filesize');
        $this.find(".image-fid").text($currentImgFid);
        $this.find(".image-uri").text(relativeImgSrc);
        $this.find('[ID*= "library-imagesrc"]').val(relativeImgSrc);
        $this.find(".image-name").text($currentImgName);
        $this.find(".image-size").text($currentImgSize);
      })

      // now process click (select).
      .on('click', '.image-library-choose-file', function () {
        var $selectedImg = $(this).find('img');
        var absoluteImgSrc = $selectedImg.data('file-url');
        var relativeImgSrc = Backdrop.relativeUrl(absoluteImgSrc);
        var $selectedImgFid = $selectedImg.data('fid');

        // make image-confirm part of form visible.
        $this.find(".image-select").hide();
        $this.find(".image-confirm").show();
        $this.find(".image-current").hide();
        $this.find(".image-upload").hide();
        $this.find(".description").hide();
        $this.find(".image-toggles").hide();

        // hide imagesrc field.
        $this.find('div[class*= "library-imagesrc"]').hide();
        // show 'confirm selected' button.
        $this.find('[ID*= "library-confirm"]').show();
        // hide image upload file field.
        $this.find('[ID*= "upload"]').hide();
        // replace selected image preview.
        $this.find('div[class= "selected-image-preview"] img').replaceWith('<img src="' + relativeImgSrc + '">');
        // add value of fid to form.
        $this.find('input[name*= "[fid]"]').val($selectedImgFid);
        // make alt and title fields visible.
        $this.find('div[class*= "-alt"]').show();
        $this.find('div[class*= "-title"]').show();
        // make field descriptions visible.
        $this.find(".description").show();
      });
    });
  }
};
