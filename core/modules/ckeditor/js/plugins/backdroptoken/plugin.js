/**
 * @file
 * Backdrop Link plugin.
 */

(function ($, Backdrop, CKEDITOR) {

"use strict";

CKEDITOR.plugins.add('backdroptoken', {
  init: function (editor) {
    editor.addCommand('backdroptoken', {
      allowedContent: 'a[!href,target]',
      modes: {wysiwyg: 1},
      canUndo: true,
      exec: function (editor) {
        // Set existing values based on selected element.
        var existingValues = {};

        // Prepare a save callback to be used upon saving the dialog.
        var saveCallback = function (returnValues) {};

        // Backdrop.t() will not work inside CKEditor plugins because CKEditor
        // loads the JavaScript file instead of Backdrop. Pull translated strings
        // from the plugin settings that are translated server-side.
        var dialogSettings = {
          dialogClass: 'editor-token-dialog'
        };

        // Open the dialog for the edit form.
        console.log(editor.config.backdrop.tokenDialogUrl);
        Backdrop.ckeditor.openDialog(editor, editor.config.backdrop.tokenDialogUrl, existingValues, saveCallback, dialogSettings);

        $(window).on('dialog:aftercreate', function (e, dialog, $element, settings) {
          console.log($element.find('.token-click-insert'));
        });

      }
    });

    // Add buttons for token.
    if (editor.ui.addButton) {
      editor.ui.addButton('BackdropToken', {
        label: Backdrop.t('Token'),
        command: 'backdroptoken',
        icon: this.path + '/token.png'
      });
    }
  }
});

})(jQuery, Backdrop, CKEDITOR);
