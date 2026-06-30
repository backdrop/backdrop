
(function($) {

/**
 * Backdrop FieldGroup object.
 */
Backdrop.FieldGroup = Backdrop.FieldGroup || {};
Backdrop.FieldGroup.Effects = Backdrop.FieldGroup.Effects || {};
Backdrop.FieldGroup.groupWithfocus = null;

Backdrop.FieldGroup.setGroupWithfocus = function(element) {
  element.css({display: 'block'});
  Backdrop.FieldGroup.groupWithfocus = element;
}

Backdrop.FieldGroup.setDetailsWithfocus = function(element) {
  element.attr('open', 'TRUE');
  Backdrop.FieldGroup.groupWithfocus = element;
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
          Backdrop.FieldGroup.setGroupWithfocus($(this));
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
          Backdrop.FieldGroup.setDetailsWithfocus($(this));
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
            Backdrop.FieldGroup.setGroupWithfocus($(this));
            $(this).data('verticalTab').focus();
            errorFocussed = true;
          }
        }
      });
    }
  }
}

/**
 * Implements Backdrop.FieldGroup.processHook().
 *
 * TODO clean this up meaning check if this is really  necessary.
 */
Backdrop.FieldGroup.Effects.processDiv = {
  execute: function (context, settings, type) {

    $('div.collapsible', context).once('fieldgroup-effects', function() {
      var $wrapper = $(this);

      // Turn the legend into a clickable link, but retain span.field-group-format-toggler
      // for CSS positioning.

      var $toggler = $('span.field-group-format-toggler:first', $wrapper);
      var $link = $('<a class="field-group-format-title" href="#"></a>');
      $link.prepend($toggler.contents());

      // Add required field markers if needed
      if ($(this).is('.required-fields') && $(this).find('.form-required').length > 0) {
        $link.append(' ').append($('.form-required').eq(0).clone());
      }

      $link.appendTo($toggler);

      // .wrapInner() does not retain bound events.
      $link.click(function () {
        var wrapper = $wrapper.get(0);
        // Don't animate multiple times.
        if (!wrapper.animating) {
          wrapper.animating = true;
          var speed = $wrapper.hasClass('speed-fast') ? 300 : 1000;
          if ($wrapper.hasClass('effect-none') && $wrapper.hasClass('speed-none')) {
            $('> .field-group-format-wrapper', wrapper).toggle();
            wrapper.animating = false;
          }
          else if ($wrapper.hasClass('effect-blind')) {
            $('> .field-group-format-wrapper', wrapper).toggle('blind', {}, speed);
            wrapper.animating = false;
          }
          else {
            $('> .field-group-format-wrapper', wrapper).toggle(speed, function() {
              wrapper.animating = false;
            });
          }
        }
        $wrapper.toggleClass('collapsed');
        return false;
      });

    });
  }
};

/**
 * Behaviors.
 */
Backdrop.behaviors.fieldGroups = {
  attach: function (context, settings) {

    // Vertical tabs: fixes css for fieldgroups.
    $('.fieldset-wrapper .fieldset > legend').css({ display: 'block' });
    $('.vertical-tabs fieldset.fieldset').addClass('default-fallback');

    // Fieldsets: set the hash in url to remember last userselection.
    $('.group-wrapper ul li').once('group-wrapper-ul-processed', function() {
      var fieldGroupNavigationListIndex = $(this).index();
      $(this).children('a').click(function() {
        var fieldset = $('.group-wrapper fieldset').get(fieldGroupNavigationListIndex);
        // Grab the first id, holding the wanted hashurl.
        var hashUrl = $(fieldset).attr('id').replace(/^field_group-/, '').split(' ')[0];
        window.location.hash = hashUrl;
      });
    });

  }
};

})(jQuery);
