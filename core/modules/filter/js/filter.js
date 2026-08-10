/**
 * @file
 * Attaches behavior for the Filter module.
 */

(function ($) {

/**
 * Initialize an empty object where editors can place their attachment code.
 */
Backdrop.editors = {};

/**
 * Horizontal offset while the image browser window is open.
 */
Backdrop.filterModalLeft = undefined;

/**
 * In the image editor, keep track of which "screen" is being
 * displayed (e.g., the Upload screen or the Select from library screen).
 */
Backdrop.filterImageEditorDisplay = undefined;


/**
 * Keeps track of the original dimensions of the image we are manipulating
 * in the image editor.
 */
Backdrop.filterImageOriginalDimensions = {width: null, height: null};


/**
 * Displays the guidelines of the selected text format automatically.
 */
Backdrop.behaviors.filterGuidelines = {
  attach: function (context) {
    $('.filter-guidelines', context).once('filter-guidelines')
      .find(':header').hide()
      .closest('.filter-wrapper').find('select.filter-list')
      .on('change', function () {
        $(this).closest('.filter-wrapper')
          .find('.filter-guidelines-item').hide()
          .siblings('.filter-guidelines-' + this.value).show();
      })
      .trigger('change');
  }
};


/**
 * Enables an editor (if any) when the matching format is selected.
 */
Backdrop.behaviors.filterEditors = {
  attach: function (context, settings) {
    // If there are no filter settings, there are no editors to enable.
    if (!settings.filter) {
      return;
    }

    var $context = $(context);
    $context.find('.filter-list:input').once('filterEditors', function () {
      var $this = $(this);
      var activeEditor = $this.val();
      var field = $this.closest('.text-format-wrapper').find('textarea').get(-1);

      // No textarea found. This may happen on long text elements that use a
      // single-line text field widget.
      if (!field) {
        return;
      }

      // Directly attach this editor, if the input format is enabled or there is
      // only one input format at all.
      if ($this.is(':input')) {
        if (Backdrop.settings.filter.formats[activeEditor]) {
          Backdrop.filterEditorAttach(field, Backdrop.settings.filter.formats[activeEditor]);
        }
      }
      // Attach onChange handlers to input format selector elements.
      if ($this.is('select')) {
        $this.on('change', function() {
          // Detach the current editor (if any) and attach a new editor.
          if (Backdrop.settings.filter.formats[activeEditor]) {
            Backdrop.filterEditorDetach(field, Backdrop.settings.filter.formats[activeEditor]);
          }
          activeEditor = $this.val();
          if (Backdrop.settings.filter.formats[activeEditor]) {
            Backdrop.filterEditorAttach(field, Backdrop.settings.filter.formats[activeEditor]);
          }
        });
      }
      // Detach any editor when the containing form is submitted.
      $this.parents('form').on('submit', function (event) {
        // Do not detach if the event was canceled.
        if (event.isDefaultPrevented()) {
          return;
        }
        // Detach the editor with "submit" when doing a non-AJAX submit.
        Backdrop.filterEditorDetach(field, Backdrop.settings.filter.formats[activeEditor], 'submit');
      });
    });
  },
  detach: function (context, settings, trigger) {
    var $context = $(context);
    $context.find('.filter-list:input').each(function () {
      var $this = $(this);
      var activeEditor = $this.val();
      var field = $this.closest('.text-format-wrapper').find('textarea').get(-1);
      if (trigger !== 'serialize') {
        $this.removeOnce('filterEditors');
      }
      if (field && Backdrop.settings.filter.formats[activeEditor]) {
        Backdrop.filterEditorDetach(field, Backdrop.settings.filter.formats[activeEditor], trigger);
      }
    });
  }
};

/**
 * Attach an editor to a textarea field.
 *
 * @param {Element} field
 *   The original textarea DOM element.
 * @param {Object} format
 *   The text format information.
 */
Backdrop.filterEditorAttach = function(field, format) {
  if (format.editor && Backdrop.editors[format.editor]) {
    Backdrop.editors[format.editor].attach(field, format);
  }
};

  /**
   * Detach an editor from the page.
   *
   * @param {Element} field
   *   The original textarea DOM element.
   * @param {Object} format
   *   The text format information.
   * @param {string} trigger
   *   A string with the value "unload", "move", "serialize" (see
   *   Backdrop.detachBehaviors() for more information on these), or "submit".
   *   Submit is used to detach editors when the complete form is submitted
   *   with non-AJAX behavior, which may be useful to let the browser clean up
   *   the editor's events and memory.
   *
   * @see Backdrop.detachBehaviors()
   */
  Backdrop.filterEditorDetach = function(field, format, trigger) {
  trigger = trigger || 'unload';
  if (format.editor && Backdrop.editors[format.editor]) {
    Backdrop.editors[format.editor].detach(field, format, trigger);
  }
};

/**
 * Provides summary text for the "Formatting options" fieldset, under each
 * textarea field with a text editor.
 */
Backdrop.behaviors.filterFieldsetSummaries = {
  attach: function (context) {
    $(context).find('fieldset.filter-wrapper').backdropSetSummary(function (element) {
      var summary = '';
      // Look for a select list of text formats.
      var $select_list = $(element).find('select.filterEditors-processed :selected');
      // Otherwise look for a hidden input element (when the current user has
      // access to only a single text format).
      var $input_element = $(element).find('input.filterEditors-processed');

      if ($select_list.length) {
        summary = $select_list.text();
      }
      else if ($input_element.length) {
        summary = $input_element.attr('data-text-format-name');
      }

      return summary;
    });
  }
};

/**
 * Provides toggles for uploading an image, whether by URL or upload.
 */
Backdrop.behaviors.editorImageDialog = {
  attach: function (context, settings) {
    var $newToggles = $('[data-editor-image-toggle]', context).once('editor-image-toggle');
    $newToggles.each(function() {
      var $toggleItems = $('[data-editor-image-toggle]');

      // Remove any previous toggles next to all labels.
      $toggleItems.find('label').siblings('.editor-image-toggle').remove();

      // Add toggles next to all labels.
      var $toggleLink, toggleLabel;
      $toggleItems.each(function(n) {
        $toggleItems.eq(n).find('label:first').addClass('editor-image-toggle');
        $toggleItems.each(function(m) {
          toggleLabel = $toggleItems.eq(m).attr('data-editor-image-toggle');
          $toggleLink = $('<a class="editor-image-toggle" href="#"></a>').text(toggleLabel);
          if (n > m) {
            $toggleItems.eq(n).find('label:first').before($toggleLink);
          }
          else if (n < m) {
            $toggleItems.eq(n).find('label:first').after($toggleLink);
          }
        });

        // Because these elements are not the first and last elements of their
        // parent (the form-element wrapper), we need specific classes to target
        // them instead of using :first-child and :last-child in CSS.
        $toggleItems.eq(n).find('.editor-image-toggle').removeClass('first last')
          .filter(':first').addClass('first').end()
          .filter(':last').addClass('last').end();
      });
    });

    // Initialize styles of Dialog.
    if ($newToggles.length) {
      // Hide the library image browser on load.
      $(".editor-dialog").removeClass("editor-dialog-with-library");
      // Set the class for the left-hand part.
      $(".editor-image-fields").addClass("editor-image-fields-full");
      // When we first open, we're always on the "upload" part of the dialog.
      Backdrop.filterImageEditorDisplay = 'upload';
    }

    $newToggles.on('click', function(e) {
      var $link = $(e.target);
      if ($link.is('.editor-image-toggle') === false) {
        return;
      }

      // Find the first ancestor of link.
      var $currentItem = $link.closest('[data-editor-image-toggle]');
      var $allItems = $('[data-editor-image-toggle]');
      var offset = $currentItem.find('.editor-image-toggle').index($link);
      var $shownItem = $allItems.eq(offset);
      $allItems.not($shownItem).filter(':visible').hide().trigger('editor-image-hide');
      var $newItem = $allItems.eq(offset).show();
      // Focus the first shown new element. This keeps focus on the dialog and
      // allows it to be closed with the escape key.
      $newItem.find('input, textarea, select').filter(':focusable').first().trigger('focus');
      $newItem.trigger('editor-image-show');

      // Clear any existing width and height, as well as
      // previously recorded width and height for the "reset"
      // button.
      $('.filter-format-editor-image-form [name="attributes[width]"]').val('');
      $('.filter-format-editor-image-form [name="attributes[height]"]').val('');
      $('.filter-format-editor-image-form .editor-image-size #reset-orig').data('data-dimensions', {width: null, height: null});
      $('.filter-format-editor-image-form .editor-image-size #reset-orig').addClass('reset-orig-inactive');

      if ($newItem.hasClass('form-item-fid')) {
        // The user is now viewing the "Upload an image" screen.
        Backdrop.filterImageEditorDisplay = 'upload';
        // Check if we had previously uploaded an image. If so, populate
        // our width and height fields with its width and height.
        var existingFile = $('.filter-format-editor-image-form .form-managed-file a').attr('href');
        if (typeof existingFile !== 'undefined') {
          Backdrop.filterImageOriginalDimensions = {
            width: null,
            height: null
          };

          var img = new Image();
          img.onload = function() {
            Backdrop.filterImageOriginalDimensions.width = this.width;
            Backdrop.filterImageOriginalDimensions.height = this.height;
            Backdrop.behaviors.editorImageLibrary.resetDataAttr(Backdrop.filterImageOriginalDimensions);

            if (!$('.filter-format-editor-image-form [name="attributes[width]"]').val().length) {
              Backdrop.behaviors.editorImageLibrary.imageDimensionsSet(Backdrop.filterImageOriginalDimensions);
            }

            Backdrop.behaviors.editorImageLibrary.syncAspectRatio(Backdrop.filterImageOriginalDimensions);
            Backdrop.behaviors.editorImageLibrary.updateResetButtonState();
          }
          // Actually perform the loading of the image last
          // just in case the listener functions are set yet.
          img.src = existingFile;
        }
      }
      else if ($newItem.hasClass('form-item-attributes-src')) {
        // The user is now viewing the "Select from library" screen.
        Backdrop.filterImageEditorDisplay = 'library';
      }

      return false;
    });

    $newToggles.on('editor-image-hide', function() {
      var $input;
      $(this).find('input[type="url"], input[type="text"], textarea').each(function() {
        $input = $(this);
        $input.data('editor-previous-value', $input.val());
        $input.val('');
      });
    });

    $newToggles.on('editor-image-show', function() {
      var $input, previousValue;
      $(this).find('input[type="url"], input[type="text"], textarea').each(function() {
        $input = $(this);
        previousValue = $input.data('editor-previous-value');
        if (previousValue && previousValue.length) {
          $input.val(previousValue);
        }
      });

      var libraryShown = $('.editor-image-fields').find('[name="attributes[src]"]').is(':visible');
      if (libraryShown) {
        // Image library already open.
        if ($('.library-view').length) {
          return;
        }
        // Toggle state is set to show 'select an image'
        // so add library view to dialog display.
        // But only for filter-format-edit-image-form.
        if ($('form').hasClass('filter-format-editor-image-form')) {
          // Remove the dialog position, let the filter.css CSS for a
          // percentage-based width take precedence.
          Backdrop.filterModalLeft = $('.editor-dialog').position().left;
          $('.editor-dialog').css('left', '');
          // Re-center the dialog by triggering a window resize.
          window.setTimeout(function() {
            Backdrop.optimizedResize.trigger();
          }, 500);
          // Increase width of dialog form.
          $('.editor-dialog').addClass('editor-dialog-with-library');

          // Display the library view.
          $('.editor-image-fields').removeClass('editor-image-fields-full');
          $('form.filter-format-editor-image-form').append('<div class="editor-image-library"></div>');
          $('[name=library_open]').trigger('click');
        }
      }
      else {
        // Remove the library part of the dialog form.
        $('.editor-image-library').each(function() {
          Backdrop.detachBehaviors(this);
          $(this).remove();
        });

        // Restore the previous dialog position.
        if (Backdrop.filterModalLeft) {
          $(".editor-dialog").css('left', Backdrop.filterModalLeft + 'px');
          // Re-center the dialog by triggering a window resize.
          window.setTimeout(function() {
            Backdrop.optimizedResize.trigger();
          }, 500);
        }
        $('.editor-dialog').removeClass('editor-dialog-with-library');
        // Set the class for the dialog part.
        $('.editor-image-fields').addClass('editor-image-fields-full');
      }
    });
  }
};

/**
 * Provides behavior for clicking on images within the library browser.
 */
Backdrop.behaviors.editorImageLibrary = {
  attach: function (context, settings) {
    // The context may be the image library div itself, so include the context
    // element in the selector.
    $('[data-editor-library-view]')
      .once('editor-library-view')
      .on('click', '.image-library-choose-file', function() {
        var $libraryFile = $(this);
        var $selectedImg = $libraryFile.find('img');
        var absoluteImgSrc = $selectedImg.data('file-url');
        var relativeImgSrc = Backdrop.relativeUrl(absoluteImgSrc);

        var $form = $('.filter-format-editor-image-form');
        $form.find('[name="attributes[src]"]').val(relativeImgSrc).trigger('change');
        $form.find('[name="fid[fid]"]').val($selectedImg.data('fid'));

        // Remove style from previous selection.
        $('.image-library-image-selected').removeClass('image-library-image-selected');
        // Add style to this selection.
        $libraryFile.addClass('image-library-image-selected');
      })
      .on('dblclick', '.image-library-choose-file', function() {
        var $libraryFile = $(this);
        $libraryFile.trigger('click');
        var $form = $libraryFile.closest('.ui-dialog-content').find('form');
        var $submit = $form.find('.form-actions input[type=submit]:first');
        $submit.trigger('mousedown').trigger('click').trigger('mouseup');
      });

    // Lock image aspect ratio if the user manually changes width or height.
    var $sizeFormItems = $('.filter-format-editor-image-form .editor-image-size');
    // But first make sure, the form items exist.
    if (!$sizeFormItems.length) {
      return;
    }

    Backdrop.filterImageOriginalDimensions = {
      width: null,
      height: null
    };

    $('.filter-format-editor-image-form .editor-image-size #reset-orig').off().on('click', function(e) {
      e.preventDefault();
      e.stopPropagation();
      Backdrop.behaviors.editorImageLibrary.imageDimensionsSet($(this).data('data-dimensions'));
      Backdrop.behaviors.editorImageLibrary.updateResetButtonState();
    });

    Backdrop.behaviors.editorImageLibrary.imageLoadExistingFile();

  },
  imageLoadExistingFile: function () {
    // When editing a previously added/selected image, or upload a new one.
    var existingFile = $('.filter-format-editor-image-form .form-managed-file .file a').attr('href');
    if (Backdrop.filterImageEditorDisplay === 'library') {
      existingFile = $('.filter-format-editor-image-form #edit-attributes-src').val();
    }

    if (typeof existingFile !== 'undefined' && existingFile !== '') {
      var img = new Image();
      // First, define functions for img.
      img.onload = function() {


        Backdrop.filterImageOriginalDimensions.width = this.width;
        Backdrop.filterImageOriginalDimensions.height = this.height;
        Backdrop.behaviors.editorImageLibrary.resetDataAttr(Backdrop.filterImageOriginalDimensions);
        if (!$('.filter-format-editor-image-form [name="attributes[width]"]').val().length) {
          Backdrop.behaviors.editorImageLibrary.imageDimensionsSet(Backdrop.filterImageOriginalDimensions);
        }
        Backdrop.behaviors.editorImageLibrary.syncAspectRatio(Backdrop.filterImageOriginalDimensions);
        Backdrop.behaviors.editorImageLibrary.updateResetButtonState();
      }
      img.onerror = function() {
        Backdrop.filterImageOriginalDimensions.width = null;
        Backdrop.filterImageOriginalDimensions.height = null;
      }
      // Perform the loading of the image last, just in case
      // the listener functions aren't set yet.
      img.src = existingFile;
    }
    else if ($('.filter-format-editor-image-form [name="attributes[width]"]').length) {
      // After an image has been removed via button, and the managed form item
      // reloads, reset width and height.
      if ($('.filter-format-editor-image-form [name="attributes[width]"]').val().length) {
        Backdrop.filterImageOriginalDimensions.width = null;
        Backdrop.filterImageOriginalDimensions.height = null;
        Backdrop.behaviors.editorImageLibrary.resetDataAttr(Backdrop.filterImageOriginalDimensions);
        Backdrop.behaviors.editorImageLibrary.imageDimensionsEmpty();
      }
    }

    // Selecting an image from library updates width and height values.
    $('.filter-format-editor-image-form [name="attributes[src]"]').once('filter-editor-img-src').on('change', function() {
      var img = new Image();
      img.onload = function() {
        Backdrop.filterImageOriginalDimensions.width = this.width;
        Backdrop.filterImageOriginalDimensions.height = this.height;
        Backdrop.behaviors.editorImageLibrary.resetDataAttr(Backdrop.filterImageOriginalDimensions);
        Backdrop.behaviors.editorImageLibrary.imageDimensionsSet(Backdrop.filterImageOriginalDimensions);
        Backdrop.behaviors.editorImageLibrary.syncAspectRatio(Backdrop.filterImageOriginalDimensions);
        Backdrop.behaviors.editorImageLibrary.updateResetButtonState();
      }
      img.onerror = function() {
        Backdrop.filterImageOriginalDimensions.width = null;
        Backdrop.filterImageOriginalDimensions.height = null;
        Backdrop.behaviors.editorImageLibrary.imageDimensionsEmpty();
      }
      img.src = this.value;
    });

  },
  /**
   * Helper function to empty the width and height form items.
   */
  imageDimensionsEmpty: function() {
    $('.filter-format-editor-image-form [name="attributes[width]"]').val('');
    $('.filter-format-editor-image-form [name="attributes[height]"]').val('');
    $('.filter-format-editor-image-form .editor-image-size #reset-orig').addClass('reset-orig-inactive');
  },
  /**
   * Helper function to set width and height values.
   */
  imageDimensionsSet: function(imgDimensions) {
    $('.filter-format-editor-image-form [name="attributes[width]"]').val(imgDimensions.width);
    $('.filter-format-editor-image-form [name="attributes[height]"]').val(imgDimensions.height);
  },
  /**
   * Remove previous event listeners, add new ones with current dimensions.
   *
   * Keep width and height input values in sync based on the supplied image
   * dimensions.
   */
  syncAspectRatio: function(imgDimensions) {
    $('.filter-format-editor-image-form [name="attributes[width]"]').off('input').on('input', function() {
      var newHeight = Math.round(this.value / imgDimensions.width * imgDimensions.height);
      $('.filter-format-editor-image-form [name="attributes[height]"]').val(newHeight);
      Backdrop.behaviors.editorImageLibrary.updateResetButtonState();
    });
    $('.filter-format-editor-image-form [name="attributes[height]"]').off('input').on('input', function() {
      var newWidth = Math.round(this.value / imgDimensions.height * imgDimensions.width);
      $('.filter-format-editor-image-form [name="attributes[width]"]').val(newWidth);
      Backdrop.behaviors.editorImageLibrary.updateResetButtonState();
    });
  },
  /**
   * Update the data-dimensions attribute value.
   */
  resetDataAttr: function(imgDimensions) {
    $('.filter-format-editor-image-form .editor-image-size #reset-orig').data('data-dimensions', imgDimensions);
  },
  /**
   * Determine and update the disabled state of the reset icon
   */
  updateResetButtonState: function() {
    var $icon = $('.filter-format-editor-image-form .image-ratio-reset-original');
    var currentWidth = $('.filter-format-editor-image-form [name="attributes[width]"]').val();
    var currentHeight = $('.filter-format-editor-image-form [name="attributes[height]"]').val();
    if ((currentWidth.length && $icon.data('data-dimensions').width != currentWidth) || (currentHeight.length && $icon.data('data-dimensions').height != currentHeight))  {
      $('.filter-format-editor-image-form .editor-image-size #reset-orig').removeClass('reset-orig-inactive');
    }
    else {
      $('.filter-format-editor-image-form .editor-image-size #reset-orig').addClass('reset-orig-inactive');
    }
  }
};

/**
 * Command to save the contents of an editor-provided dialog.
 *
 * This command does not close the open dialog. It should be followed by a call
 * to Drupal.AjaxCommands.prototype.closeDialog. Editors that are integrated
 * with dialogs must independently listen for an editor:dialogsave event to save
 * the changes into the contents of their interface.
 */
Backdrop.ajax.prototype.commands.editorDialogSave = function (ajax, response, status) {
  $(window).trigger('editor:dialogsave', [response.values]);
};

$(window).on('dialog:aftercreate', function () {
  // Determine which tab should be shown.
  var $visibleItems = $('[data-editor-image-toggle]').filter(':visible');
  if ($visibleItems.length > 1) {
    var $fidField = $visibleItems.find('[name="fid[fid]"]');
    var $srcField = $visibleItems.find('[name="attributes[src]"]');
    var $srcItem = $visibleItems.find($srcField).closest('[data-editor-image-toggle]');
    var $errorItem = $visibleItems.find('.error').closest('[data-editor-image-toggle]');

    // If any errors are present in the form, pre-select that tab.
    if ($errorItem.length) {
      $visibleItems.not($errorItem).hide().trigger('editor-image-hide');
      $errorItem.find('input, textarea, select').filter(':focusable').first().trigger('focus');
      $errorItem.trigger('editor-image-show');
    }
    // If an FID is not provided but a src attribute is, highlight the tab
    // that contains the src attribute field.
    if (($fidField.val() === '0' || !$fidField.val()) && $srcField.length > 0 && $srcField.val().length > 0) {
      $visibleItems.not($srcItem).hide().trigger('editor-image-hide');
      $srcItem.find('input, textarea, select').filter(':focusable').first().trigger('focus');
      $srcItem.trigger('editor-image-show');
    }
    // Otherwise, show the first tab and hide all the others.
    else {
      $visibleItems.not(':first').hide().trigger('editor-image-hide');
      $visibleItems.first().find('input, textarea, select').filter(':focusable').first().trigger('focus');
      $visibleItems.first().trigger('editor-image-show');
    }
  }
  // If no element is visible show the first tab.
  else {
    $('[data-editor-image-toggle]').not(':first').hide().trigger('editor-image-hide');
    $('[data-editor-image-toggle]').first().show().find('input, textarea, select').filter(':focusable').first().trigger('focus');
    $('[data-editor-image-toggle]').first().trigger('editor-image-show');
  }
});

})(jQuery);
