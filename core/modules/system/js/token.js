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
    // Keep track of which textfield was last selected/focused.
    $('body').once('token-focus').on('focus', 'textarea, input[type="text"]', function() {
      Backdrop.settings.tokenFocusedField = this;
      Backdrop.settings.tokenFirstClick = false;
    });

    // Replace all token spans with links.
    $(context).find('.token-click-insert .token-key').once('token-click-insert', function() {
      var $span = $(this);
      // Replace the span contents with a clickable link.
      $span.html('<a href="javascript:void(0);" title="' + Backdrop.t('Insert this token into your form') + '">' + $span.html() + '</a>');
    });

    // Bind link clicks to the entire table so we only have a single event.
    $(context).find('table.token-click-insert').once('token-click-insert')
      .each(function() {
        // Reset the first click check when displaying a new table.
        Backdrop.settings.tokenFirstClick = true;
      })
      .on('click', '.token-key a', function() {
        var $tokenLink = $(this);

        // The first click after showing a table, use the closest field to the
        // dialog link (if any).
        if (Backdrop.settings.tokenFirstClick) {
          Backdrop.settings.tokenFirstClick = false;
          var dialogOpeningLinkId = $tokenLink.closest('.token-click-insert').attr('data-token-link-id');
          if (dialogOpeningLinkId) {
            var fieldToFocus = $('#' + dialogOpeningLinkId).closest('.form-item').find('input').get(0);
            // If a field is found and it's not the last focused field, set
            // the cursor to the end of the field value.
            if (fieldToFocus && fieldToFocus != Backdrop.settings.tokenFocusedField) {
              fieldToFocus.selectionStart = fieldToFocus.value.length;
              fieldToFocus.selectionEnd = fieldToFocus.value.length;
              Backdrop.settings.tokenFocusedField = fieldToFocus;
            }
          }
        }

        // Add the token value to the focused text field.
        if (Backdrop.settings.tokenFocusedField) {
          var myField = Backdrop.settings.tokenFocusedField;
          var myValue = $tokenLink.text();

          // IE support.
          if (document.selection) {
            myField.focus();
            var sel = document.selection.createRange();
            sel.text = myValue;
          }
          // Mozilla/Webkit.
          else if (myField.selectionStart || myField.selectionStart == '0') {
            var startPos = myField.selectionStart;
            var endPos = myField.selectionEnd;
            myField.value = myField.value.substring(0, startPos)
              + myValue
              + myField.value.substring(endPos, myField.value.length);
          }
          // Otherwise just tack to the end.
          else {
            myField.value += myValue;
          }
        }
        else {
          alert(Backdrop.t('First click a text field into which the token should be inserted.'));
        }
        return false;
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
    };

    $(context).find('table.token-click-insert').once('token-description')
      .on('click', '.token-more', toggleDescription)
      .find('.token-description').each(function() {
        var $moreLink = $link.clone();
        $(this).css('display', 'none').before(' ').before($moreLink);
      });
  }
};

})(jQuery);
