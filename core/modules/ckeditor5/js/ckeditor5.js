(function (Backdrop, CKEditor5, $) {

  "use strict";

  Backdrop.editors.ckeditor5 = {

    attach: function (element, format) {
      if (!$('#ckeditor5-modal').length) {
        $('<div id="ckeditor5-modal" />').hide().appendTo('body');
      }

      // Set a title on the CKEditor instance that includes the text field's
      // label so that screen readers say something that is understandable
      // for end users.
      var label = $('label[for=' + element.getAttribute('id') + ']').text();
      var editorSettings = format.editorSettings;
      editorSettings.title = Backdrop.t("Rich Text Editor, !label field", {'!label': label});

      // CKEditor initializes itself in a read-only state if the 'disabled'
      // attribute is set. It does not respect the 'readonly' attribute,
      // however, so we set the 'readOnly' configuration property manually in
      // that case, for the CKEditor instance that's about to be created.
      editorSettings.readOnly = element.hasAttribute('readonly');

      // Try to match the textarea height on which we're replacing. Note this
      // minHeight property is enforced by the BackdropBasicStyles plugin.
      editorSettings.minHeight = $(element).height() + 'px';

      editorSettings.licenseKey = '';

      // If filter_html is turned on, and the htmlSupport plugin is available,
      // we prevent on* attributes.
      if (editorSettings.pluginList.includes('htmlSupport.GeneralHtmlSupport')) {
        if (editorSettings.htmlSupport.allow.length) {
          let onEventsPattern = {
            'name': /.*/,
            'attributes': /^on.*/
          }
          editorSettings.htmlSupport.disallow.push(onEventsPattern);
        }
        // If filter_html if off, allow all elements and attributes to be used.
        else {
          let patternAllowAll = {
            name: /.*/,
            attributes: true,
            classes: true,
            styles: true
          }
          editorSettings.htmlSupport.allow.push(patternAllowAll);
        }
      }

      // Convert the plugin list from strings to variable names. Each CKEditor
      // plugin is located under "CKEditor5.[packageName].[moduleName]". So
      // we convert the list of strings to match the expected variable name.
      editorSettings.plugins = [];
      editorSettings.pluginList.forEach(function(pluginItem) {
        var [packageName,moduleName] = pluginItem.split('.');
        if (typeof CKEditor5[packageName] != 'undefined') {
          editorSettings.plugins.push(CKEditor5[packageName][moduleName]);
        }
      });

      // Hide the resizable grippie while CKEditor is active.
      $(element).siblings('.grippie').hide();

      const beforeAttachValue = element.value;
      CKEditor5.editorClassic.ClassicEditor
        .create(element, editorSettings)
        .then(editor => {
          Backdrop.ckeditor5.setEditorOffset(editor);
          Backdrop.ckeditor5.instances.set(editor.id, editor);
          element.ckeditor5AttachedEditor = editor;
          const valueModified = Backdrop.ckeditor5.checkValueModified(beforeAttachValue, editor.getData());
          if (valueModified && !Backdrop.ckeditor5.bypassContentWarning) {
            Backdrop.ckeditor5.detachWithWarning(element, format, beforeAttachValue);
          }
          return true;
        })
        .catch(error => {
          console.error('The CKEditor instance could not be initialized.');
          console.error(error);
          return false;
        });
    },

    detach: function (element, format, trigger) {
      // Remove any content modification warning.
      if (element.ckeditor5AttachedWarning) {
        element.ckeditor5AttachedWarning.remove();
        delete element.ckeditor5AttachedWarning;
      }

      // Save content and remove any CKEditor 5 instances.
      const editor = element.ckeditor5AttachedEditor;
      if (!editor) {
        return false;
      }

      if (trigger === 'serialize') {
        // CKEditor 5 does not pretty-print HTML source. Format the source
        // before saving it into the source field.
        let newData = editor.getData();
        newData = Backdrop.ckeditor5.formatHtml(newData);
        editor.updateSourceElement(newData);
      }
      else {
        editor.destroy();
        Backdrop.ckeditor5.instances.delete(editor.id);
        delete element.ckeditor5AttachedEditor;
      }

      // Restore the resize grippie.
      $(element).siblings('.grippie').show();
      return !!editor;
    },

    onChange: function (element, callback) {
      const editor = element.ckeditor5AttachedEditor;
      if (editor) {
        editor.model.document.on('change:data', function() {
          Backdrop.debounce(callback, 400)(editor.getData());
        });
      }
      return !!editor;
    },
  };

  Backdrop.ckeditor5 = {
    /**
     * Variable storing the current dialog's save callback.
     */
    saveCallback: null,

    /**
     * Key-value map of all active instances of CKEditor 5.
     */
    instances: new Map(),

    /**
     * Boolean indicating if CKEditor instances should be attached even if they
     * modify content by the act of initializing the editor.
     */
    bypassContentWarning: false,

    /**
     * Open a dialog for a Backdrop-based plugin.
     *
     * This dynamically loads jQuery UI (if necessary) using the Backdrop AJAX
     * framework, then opens a dialog at the specified Backdrop path.
     *
     * @param editor
     *   The CKEditor instance that is opening the dialog.
     * @param string url
     *   The URL that contains the contents of the dialog.
     * @param Object existingValues
     *   Existing values that will be sent via POST to the url for the dialog
     *   contents.
     * @param Function saveCallback
     *   A function to be called upon saving the dialog.
     * @param Object dialogSettings
     *   An object containing settings to be passed to the jQuery UI.
     */
    openDialog: function (editor, url, existingValues, saveCallback, dialogSettings) {
      // Locate a suitable place to display our loading indicator.
      var $toolbar = $(editor.ui.view.toolbar.element);

      // Remove any previous loading indicator.
      $toolbar.find('.ckeditor5-dialog-loading').remove();

      // Add a consistent dialog class.
      var classes = dialogSettings.dialogClass ? dialogSettings.dialogClass.split(' ') : [];
      classes.push('editor-dialog');
      dialogSettings.dialogClass = classes.join(' ');
      dialogSettings.autoResize = true;
      dialogSettings.modal = true;
      dialogSettings.target = '#ckeditor5-modal';

      // Add a "Loading…" message, hide it underneath the CKEditor toolbar, create
      // a Backdrop.ajax instance to load the dialog and trigger it.
      var $content = $('<div class="ck-reset_all-excluded ckeditor5-dialog-loading-wrapper" style="display: none;"><div class="ckeditor5-dialog-loading"><span class="ckeditor5-dialog-loading-link"><a>' + Backdrop.t('Loading...') + '</a></span></div></div>');
      $toolbar.append($content);
      new Backdrop.ajax('ckeditor5-dialog', $content.find('a').get(0), {
        accepts: 'application/vnd.backdrop-dialog',
        dialog: dialogSettings,
        selector: '.ckeditor5-dialog-loading-link',
        url: url,
        event: 'ckeditor5-internal.ckeditor5',
        progress: {'type': 'throbber'},
        submit: {
          editor_object: existingValues
        }
      });
      $content.find('a')
          .on('click', function () { return false; })
          .trigger('ckeditor5-internal.ckeditor5');

      // After a short delay, show "Loading…" message.
      window.setTimeout(function () {
        $content.css('display', 'block');
      }, 500);

      // Store the save callback to be executed when this dialog is closed.
      Backdrop.ckeditor5.saveCallback = saveCallback;
    },

    /**
     * Calculates the top of window offset.
     *
     * The "data-offset-top" attribute is used on the admin toolbar and sticky
     * table headers. Add up the offsets to determine the editor toolbar offset.
     *
     * @returns
     *   The vertical offset in pixels.
     */
    computeOffsetTop: function () {
      var $offsets = $('[data-offset-top]');
      var value, sum = 0;
      for (var i = 0, il = $offsets.length; i < il; i++) {
        value = parseInt($offsets[i].getAttribute('data-offset-top'), 10);
        sum += !isNaN(value) ? value : 0;
      }
      this.offsetTop = sum;
      return sum;
    },

    /**
     * Sets the CKEditor 5 toolbar offset.
     *
     * Setting the offset makes the editor toolbar floats below the admin
     * toolbar and any sticky table headers.
     *
     * @param editor
     *   The CKEditor 5 instance.
     */
    setEditorOffset: function (editor) {
      editor.ui.viewportOffset = {
        'bottom': 0,
        'left': 0,
        'right': 0,
        'top': Backdrop.ckeditor5.computeOffsetTop()
      };
    },

    /**
     * Compare the data before CKEditor 5 is attached and after attachment.
     *
     * This comparison reformats both the before and after values to the same
     * consistent format before doing a string comparison.
     *
     * @param beforeAttachValue
     *   The element value before CKEditor was attached.
     * @param afterAttachValue
     *   The element value after CKEditor was attached.
     *
     * @return {boolean}
     *   Returns true if values have been modified, false if unchanged.
     */
    checkValueModified: function (beforeAttachValue, afterAttachValue) {
      const formattedBeforeValue = Backdrop.ckeditor5.formatHtml(beforeAttachValue);
      const formattedAfterValue = Backdrop.ckeditor5.formatHtml(afterAttachValue);
      return formattedBeforeValue !== formattedAfterValue;
    },

    /**
     * Attach an alert to the editor if the value has been modified.
     *
     * This disables the editor and restores the plain textarea element. The
     * warning can be dismissed to load the editor anyway.
     *
     * @param element
     *   The DOM element to which the editor was attached.
     * @param format
     *   The text format configuration with which the editor was attached.
     * @param beforeAttachValue
     *   The element value before CKEditor was attached.
     *
     * @return {boolean}
     *   Returns true if values have been modified, false if unchanged.
     */
    detachWithWarning: function (element, format, beforeAttachValues) {
      const editor = element.ckeditor5AttachedEditor;
      // Detach the editor.
      Backdrop.filterEditorDetach(element, format);
      // Restore the value to what it was previously.
      element.value = beforeAttachValues;
      // Attach a warning before the field.
      const $warning = $($.parseHTML(Backdrop.theme('ckeditor5ContentModifiedWarning')));
      $warning.insertBefore(element);
      // On click of the link within the warning, attach the editor anyway and
      // remove the warning.
      $warning.find('a').on('click', function(e) {
        // Setting this bypass flag prevents the warning from being re-added.
        Backdrop.ckeditor5.bypassContentWarning = true;
        Backdrop.filterEditorAttach(element, format);
        $warning.remove();
        e.preventDefault();
      });
      element.ckeditor5AttachedWarning = $warning[0];
    }
  };

  // Respond to new dialogs that are opened by CKEditor, closing the AJAX loader.
  $(window).on('dialog:beforecreate', function (e, dialog, $element, settings) {
    $('.ckeditor5-dialog-loading-wrapper').remove();
  });

  // Respond to dialogs that are saved, sending data back to CKEditor.
  $(window).on('editor:dialogsave', function (e, values) {
    if (Backdrop.ckeditor5.saveCallback) {
      Backdrop.ckeditor5.saveCallback(values);
    }
  });

  // Respond to dialogs that are closed, removing the current save handler.
  $(window).on('dialog:afterclose', function (e, dialog, $element) {
    if (Backdrop.ckeditor5.saveCallback) {
      Backdrop.ckeditor5.saveCallback = null;
    }
  });

  // Set the offset to account for admin toolbar.
  $(document).on('offsettopchange', function() {
    Backdrop.ckeditor5.instances.forEach(function(instance) {
      Backdrop.ckeditor5.setEditorOffset(instance);
    });
  });

  /**
   * Display a warning message that loading the editor may modify content.
   */
  Backdrop.theme.prototype.ckeditor5ContentModifiedWarning = function (settings) {
    let warningMessage = '';
    warningMessage += '<div class="ckeditor5-content-modified-warning messages warning">';
    warningMessage += '<span class="ckeditor5-content-modified-message">' + Backdrop.t('Activating CKEditor 5 will reformat the content of this field. Review content carefully after activating.') + '</span> ';
    warningMessage += '<a class="ckeditor5-content-modified-activate" href="#">' + Backdrop.t('Click to activate editor') + '</a>.';
    warningMessage += '</div>';
    return warningMessage;
  }

})(Backdrop, CKEditor5, jQuery);
