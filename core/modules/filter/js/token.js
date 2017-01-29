/**
 * JavaScript behaviors for the display of the Token UI.
 */
(function ($) {

"use strict";

Backdrop.behaviors.tokenTree = {
  attach: function (context, settings) {
    $(context).find('table.token-tree').once('token-tree', function () {
      $(this).treetable({
        'expandable': true
      });
    });
  }
};

Backdrop.behaviors.tokenLinks = {
  attach: function (context, settings) {
    // Keep track of which textfield was last selected/focused.
    $(context).find('textarea, input[type="text"]').focus(function() {
      Backdrop.settings.tokenFocusedField = this;
    });

    $(context).find('.token-click-insert .token-key').once('token-click-insert', function() {
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
    $(context).find('.token-description').each(function() {
      var $moreLink = $link.clone();
      $moreLink.click(toggleDescription);
      $(this).css('display', 'none').before(' ').before($moreLink);
    });
  }
};

})(jQuery);
