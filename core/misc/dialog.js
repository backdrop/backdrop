/**
 * @file
 *
 * Dialog API inspired by HTML5 dialog element:
 * http://www.whatwg.org/specs/web-apps/current-work/multipage/commands.html#the-dialog-element
 */
(function ($) {

"use strict";

Backdrop.settings.dialog = {
  // This option will turn off resizable and draggable.
  autoResize: true,
  // jQuery UI does not support percentage heights, but we convert this property
  // in the resetPosition() function.
  maxHeight: '95%',
  // jQuery UI defaults to 300px, use 100% width to allow for responsive
  // dialogs. CSS can override by specifying a max-width.
  width: '100%',
  position: { my: "center", at: "center", of: window },
  close: function (e) {
    Backdrop.detachBehaviors(e.target, null, 'unload');
  }
};

Backdrop.dialog = function (element, options) {

  function openDialog (settings) {
    settings = $.extend({}, Backdrop.settings.dialog, options, settings);
    // Trigger a global event to allow scripts to bind events to the dialog.
    $(window).trigger('dialog:beforecreate', [dialog, $element, settings]);
    $element.dialog(settings);

    if (settings.autoResize === true || settings.autoResize === 'true') {
      $element
        .dialog('option', {
            resizable: false,
            draggable: false
        })
        .dialog('widget').css('position', 'fixed');
      $(window)
        .on('resize.dialogResize scroll.dialogResize', settings, autoResizeCallback)
        .trigger('resize.dialogResize');
    }
    dialog.open = true;
    $(window).trigger('dialog:aftercreate', [dialog, $element, settings]);
  }

  function closeDialog (value) {
    $(window).trigger('dialog:beforeclose', [dialog, $element]);
    $element.dialog('close');
    dialog.returnValue = value;
    dialog.open = false;
    $(window).off('.dialogResize');
    $(window).trigger('dialog:afterclose', [dialog, $element]);
  }

  /**
   * Resets the current options for positioning.
   *
   * This is used as a window resize and scroll callback to reposition the
   * jQuery UI dialog. Although not a built-in jQuery UI option, this can
   * be disabled by setting autoResize: false in the options array when creating
   * a new Backdrop.dialog().
   */
  function resetPosition (event) {
    var positionOptions = ['width', 'height', 'minWidth', 'minHeight', 'maxHeight', 'maxWidth', 'position'];
    var windowHeight = $(window).height();
    var adjustedOptions = {};
    var option, optionValue, adjustedValue;
    for (var n = 0; n < positionOptions.length; n++) {
      option = positionOptions[n];
      optionValue = event.data[option];
      if (optionValue) {
        // jQuery UI does not support percentages on heights, convert to pixels.
        if (typeof optionValue === 'string' && /%$/.test(optionValue) && /height/i.test(option)) {
          adjustedValue = parseInt(0.01 * parseInt(optionValue, 10) * windowHeight, 10);
          // Don't force the dialog to be bigger vertically than needed.
          if (option === 'height' && $element.parent().outerHeight() < adjustedValue) {
            adjustedValue = 'auto';
          }
          adjustedOptions[option] = adjustedValue;
        }
        else {
          adjustedOptions[option] = optionValue;
        }
      }
    }
    $element.dialog('option', adjustedOptions);
  }

  var undef;
  var $element = $(element);
  var autoResizeTimer;
  var autoResizeCallback = function(event) {
    clearTimeout(autoResizeTimer);
    autoResizeTimer = setTimeout(function() {
      resetPosition(event);
    }, 100);
  };
  var dialog = {
    open: false,
    returnValue: undef,
    show: function () {
      openDialog({modal: false});
    },
    showModal: function () {
      openDialog({modal: true});
    },
    close: closeDialog
  };

  return dialog;
};

})(jQuery);
