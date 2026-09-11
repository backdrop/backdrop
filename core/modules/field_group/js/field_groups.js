
(function($) {

/**
 * Backdrop FieldGroup object.
 */
Backdrop.FieldGroup = Backdrop.FieldGroup || {};
Backdrop.FieldGroup.Effects = Backdrop.FieldGroup.Effects || {};
Backdrop.FieldGroup.groupWithFocus = null;

Backdrop.FieldGroup.setGroupWithFocus = function(element) {
  element.css({display: 'block'});
  Backdrop.FieldGroup.groupWithFocus = element;
}

Backdrop.FieldGroup.setDetailsWithFocus = function(element) {
  element.attr('open', 'TRUE');
  Backdrop.FieldGroup.groupWithFocus = element;
}

/**
 * Implements Backdrop.FieldGroup.processHook().
 */
Backdrop.FieldGroup.Effects.processFieldset = {
  execute: function (context, settings, type) {
    if (type == 'form') {
      // Add required fields mark to any fieldsets containing required fields
      $('fieldset.fieldset', context).once('fieldgroup-effects', function(i) {
        if ($(this).is('.required-fields') && $(this).find('.form-required').length > 0) {
          $('legend span.fieldset-legend', $(this)).eq(0).append(' ').append($('.form-required').eq(0).clone());
        }
        if ($('.error', $(this)).length) {
          $('legend span.fieldset-legend', $(this)).eq(0).addClass('error');
          Backdrop.FieldGroup.setGroupWithFocus($(this));
        }
      });
    }
  }
}

/**
 * Implements Backdrop.FieldGroup.processHook().
 */
 Backdrop.FieldGroup.Effects.processDetails = {
  execute: function (context, settings, type) {
    if (type == 'form') {
      // Add required fields mark to any details containing required fields
      $('details', context).once('fieldgroup-effects', function(i) {
        if ($(this).is('.required-fields') && $(this).find('.form-required').length > 0) {
          $('summary span', $(this)).eq(0).append(' ').append($('.form-required').eq(0).clone());
        }
        if ($('.error', $(this)).length) {
          $('summary span', $(this)).eq(0).addClass('error');
          Backdrop.FieldGroup.setDetailsWithFocus($(this));
        }
      });
    }
  }
}

/**
 * Implements Backdrop.FieldGroup.processHook().
 */
Backdrop.FieldGroup.Effects.processTabs = {
  execute: function (context, settings, type) {
    if (type == 'form') {

      var errorFocussed = false;

      // Add required fields mark to any fieldsets containing required fields
      $('fieldset.vertical-tabs-pane', context).once('fieldgroup-effects', function(i) {
        if ($(this).is('.required-fields') && $(this).find('.form-required').length > 0) {
          $(this).data('verticalTab').link.find('strong:first').after($('.form-required').eq(0).clone()).after(' ');
        }
        if ($('.error', $(this)).length) {
          $(this).data('verticalTab').link.parent().addClass('error');
          // Focus the first tab with error.
          if (!errorFocussed) {
            Backdrop.FieldGroup.setGroupWithFocus($(this));
            $(this).data('verticalTab').focus();
            errorFocussed = true;
          }
        }
      });
    }
  }
}

/**
 * Behaviors.
 */
Backdrop.behaviors.fieldGroups = {
  attach: function (context, settings) {

    // Vertical tabs: fixes css for field groups.
    $('.fieldset-wrapper .fieldset > legend').css({ display: 'block' });
    $('.vertical-tabs fieldset.fieldset').addClass('default-fallback');

    // Fieldsets: set the hash in url to remember last user selection.
    $('.group-wrapper ul li').once('group-wrapper-ul-processed', function() {
      var fieldGroupNavigationListIndex = $(this).index();
      $(this).children('a').click(function() {
        var fieldset = $('.group-wrapper fieldset').get(fieldGroupNavigationListIndex);
        // Grab the first id, holding the wanted hash url.
        var hashUrl = $(fieldset).attr('id').replace(/^field_group-/, '').split(' ')[0];
        window.location.hash = hashUrl;
      });
    });

  }
};

})(jQuery);
