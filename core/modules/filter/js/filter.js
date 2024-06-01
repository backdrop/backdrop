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
        Backdrop.filterEditorDetach(field, Backdrop.settings.filter.formats[activeEditor]);
      });
    });
  },
  detach: function (context, settings, trigger) {
    var $context = $(context);
    $context.find('.filter-list:input').each(function () {
      var $this = $(this);
      var activeEditor = $this.val();
      var field = $this.closest('.text-format-wrapper').find('textarea').get(-1);
      $this.removeOnce('filterEditors');
      if (field && Backdrop.settings.filter.formats[activeEditor]) {
        Backdrop.filterEditorDetach(field, Backdrop.settings.filter.formats[activeEditor], trigger);
      }
    });
  }
};

Backdrop.filterEditorAttach = function(field, format) {
  if (format.editor && Backdrop.editors[format.editor]) {
    Backdrop.editors[format.editor].attach(field, format);
  }
};

Backdrop.filterEditorDetach = function(field, format, trigger) {
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
        $form.find('[name="attributes[src]"]').val(relativeImgSrc);
        $form.find('[name="fid[fid]"]').val($selectedImg.data('fid'));

        // Reset width and height so image is not stretched to the any
        // previous image's dimensions.
        $form.find('[name="attributes[width]"]').val('');
        $form.find('[name="attributes[height]"]').val('');
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
