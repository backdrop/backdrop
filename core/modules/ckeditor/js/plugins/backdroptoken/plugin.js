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

        // Open the dialog for the token browser.
        Backdrop.ckeditor.openDialog(editor, editor.config.backdrop.tokenDialogUrl, existingValues, saveCallback, dialogSettings);

        $(window).on('dialog:aftercreate', function (e, dialog, $element, settings) {
          $element.find('table.token-tree').once('token-tree', function () {
            $(this).treetable({
              'expandable': true
            });
          });
          $element.find('.token-click-insert .token-key').once('token-click-insert', function() {
            var newThis = $('<a href="javascript:void(0);" title="' + Backdrop.t('Insert this token into your form') + '">' + $(this).html() + '</a>').click(function(){
            });
            $(this).html(newThis);
          });
          var more = '&rtrif; ' + Backdrop.t('more');
          var less = '&dtrif; ' + Backdrop.t('less');
          var $link = $('<a class="token-more" href="#">' + more + ' </a>');
          var toggleDescription = function() {
            if ($(this).toggleClass('open').hasClass('open')) {
              $(this).html(less).siblings('.token-description').css('display', 'block');
            }
            else {
              $(this).html(more).siblings('.token-description').css('display', 'none');
            }
            return false;
          }
          $element.find('.token-description').once('token-description').each(function() {
            var $moreLink = $link.clone();
            $moreLink.click(toggleDescription);
            $(this).css('display', 'none').before(' ').before($moreLink);
          });
          $('.token-key').once('token-key').click(function(e) {
            var tokenText = $(this).text();
            editor.insertText( tokenText );
          });
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
