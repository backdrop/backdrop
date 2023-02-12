/**
 * JavaScript behaviors for the display of the Token UI.
 */
(function ($) {

"use strict";

Backdrop.behaviors.tokenTree = {
  attach: function (context, settings) {
    $(context).find('table.token-tree').once('token-tree', function () {
      $(this).treetable({
        'expandable': true,
        'clickableNodeNames': true
      });
    });
  }
};

Backdrop.behaviors.tokenInsert = {
  attach: function (context, settings) {
    // Keep track of which textfield was last selected/focused. This method
    // works around .focus() not working with iframes, which is what CKEditor
    // fields use. See https://stackoverflow.com/a/28932220.
    $(document).on('focusout', function() {
      // Using setTimout to let the event pass the run loop.
      setTimeout(function() {
        // See: http://api.jquery.com/focus-selector
        // ... If you are looking for the currently focused element,
        // $(document.activeElement) will retrieve it without having to search
        // the whole DOM tree.
        var focusedElement = document.activeElement;

        var isCKEditorIframe = focusedElement instanceof HTMLIFrameElement && $(focusedElement).hasClass('cke_wysiwyg_frame');
        var isTextArea = focusedElement instanceof HTMLTextAreaElement;
        var isTextInput = focusedElement instanceof HTMLInputElement && $(focusedElement).attr('type') === 'text';

        if (isCKEditorIframe || isTextArea || isTextInput) {
          Backdrop.settings.tokenFocusedField = focusedElement;
console.log(Backdrop.settings.tokenFocusedField);
        }
      }, 0);
    });

    $(context).find('.token-click-insert .token-key').once('token-click-insert', function() {
      var tokenLink = $('<a href="javascript:void(0);" title="' + Backdrop.t('Insert this token into your form') + '">' + $(this).html() + '</a>').click(function(){
        var focusField = Backdrop.settings.tokenFocusedField;
        var focusFieldType = Object.prototype.toString.call(focusField).match(/^\[object\s(.*)\]$/)[1];
        var tokenValue = $(this).text();
console.log('myField: ' + focusField);
console.log('myFieldType: ' + focusFieldType);
console.log('myValue: ' + tokenValue);

        switch (focusFieldType) {
          case 'HTMLIFrameElement':
            // @todo: Insert token into the <body> of the CKEditor iframe.
console.log('CKEditor iframe');
            break;
          case 'HTMLTextAreaElement':
          case 'HTMLInputElement':
console.log('textarea or input text fields');
            // IE support.
            if (document.selection) {
              focusField.focus();
              var sel = document.selection.createRange();
              sel.text = tokenValue;
            }
            // Mozilla/Webkit.
            else if (focusField.selectionStart || focusField.selectionStart == '0') {
              var startPos = focusField.selectionStart;
              var endPos = focusField.selectionEnd;
              focusField.value = focusField.value.substring(0, startPos)
                + tokenValue
                + focusField.value.substring(endPos, focusField.value.length);
            }
            // Otherwise just tack to the end.
            else {
              focusField.value += tokenValue;
            }
            break;
          default: 
            alert(Backdrop.t('First click a text field into which the token should be inserted.'));
        }

        return false;
      });
      $(this).html(tokenLink);
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
    $(context).find('.token-description').each(function() {
      var $moreLink = $link.clone();
      $moreLink.click(toggleDescription);
      $(this).css('display', 'none').before(' ').before($moreLink);
    });
  }
};

})(jQuery);
