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

      CKEditor5.editorClassic.ClassicEditor
        .create(element, editorSettings)
        .then(editor => {
          Backdrop.ckeditor5.setEditorOffset(editor);
          Backdrop.ckeditor5.instances.set(editor.id, editor);
          element.ckeditorAttachedEditor = editor;
          return true;
        })
        .catch(error => {
          console.error('The CKEditor instance could not be initialized.');
          console.error(error);
          return false;
        });
    },

    detach: function (element, format, trigger) {
      // CKEditor's getEditor() method takes a single parameter for "optimized"
      // that defaults to true. This makes it so that only unmodified DOM
      // textarea elements will qualify for locating the CKEditor instance.
      // During detachment, other behaviors may also modify the source textarea,
      // causing CKEditor to lose track of the editor. Therefore pass "false"
      // to use the more aggressive attempt to find the editor instance.
      var editor = element.ckeditorAttachedEditor;
      if (!editor) {
        return false;
      }

      if (trigger === 'serialize') {
        editor.updateSourceElement();
      }
      else {
        editor.destroy();
        Backdrop.ckeditor5.instances.delete(editor.id);
      }

      // Restore the resize grippie.
      $(element).siblings('.grippie').show();
      return !!editor;
    },

    onChange: function (element, callback) {
      var editor = element.ckeditorAttachedEditor;
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

    setEditorOffset: function (editor) {
      editor.ui.viewportOffset = {
        'bottom': 0,
        'left': 0,
        'right': 0,
        'top': Backdrop.ckeditor5.computeOffsetTop()
      };
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

})(Backdrop, CKEditor5, jQuery);
