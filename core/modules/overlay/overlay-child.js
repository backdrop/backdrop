
(function ($) {

/**
 * Attach the child dialog behavior to new content.
 */
Backdrop.behaviors.overlayChild = {
  attach: function (context, settings) {
    // Make sure this behavior is not processed more than once.
    if (this.processed) {
      return;
    }
    this.processed = true;

    // If we cannot reach the parent window, break out of the overlay.
    if (!parent.Backdrop || !parent.Backdrop.overlay) {
      window.location = window.location.href.replace(/([?&]?)render=overlay&?/g, '$1').replace(/\?$/, '');
    }

    var settings = settings.overlayChild || {};

    // If the entire parent window should be refreshed when the overlay is
    // closed, pass that information to the parent window.
    if (settings.refreshPage) {
      parent.Backdrop.overlay.refreshPage = true;
    }

    // If a form has been submitted successfully, then the server side script
    // may have decided to tell the parent window to close the popup dialog.
    if (settings.closeOverlay) {
      parent.Backdrop.overlay.bindChild(window, true);
      // Use setTimeout to close the child window from a separate thread,
      // because the current one is busy processing Backdrop behaviors.
      setTimeout(function () {
        if (typeof settings.redirect == 'string') {
          parent.Backdrop.overlay.redirect(settings.redirect);
        }
        else {
          parent.Backdrop.overlay.close();
        }
      }, 1);
      return;
    }

    // If one of the regions displaying outside the overlay needs to be
    // reloaded immediately, let the parent window know.
    if (settings.refreshRegions) {
      parent.Backdrop.overlay.refreshRegions(settings.refreshRegions);
    }

    // Ok, now we can tell the parent window we're ready.
    parent.Backdrop.overlay.bindChild(window);

    // IE8 crashes on certain pages if this isn't called; reason unknown.
    window.scrollTo(window.scrollX, window.scrollY);

    // Attach child related behaviors to the iframe document.
    Backdrop.overlayChild.attachBehaviors(context, settings);

    // There are two links within the message that informs people about the
    // overlay and how to disable it. Make sure both links are visible when
    // either one has focus and add a class to the wrapper for styling purposes.
    $('#overlay-disable-message', context)
      .focusin(function () {
        $(this).addClass('overlay-disable-message-focused');
        $('a.element-focusable', this).removeClass('element-invisible');
      })
      .focusout(function () {
        $(this).removeClass('overlay-disable-message-focused');
        $('a.element-focusable', this).addClass('element-invisible');
      });
  }
};

/**
 * Overlay object for child windows.
 */
Backdrop.overlayChild = Backdrop.overlayChild || {
  behaviors: {}
};

Backdrop.overlayChild.prototype = {};

/**
 * Attach child related behaviors to the iframe document.
 */
Backdrop.overlayChild.attachBehaviors = function (context, settings) {
  $.each(this.behaviors, function () {
    this(context, settings);
  });
};

/**
 * Capture and handle clicks.
 *
 * Instead of binding a click event handler to every link we bind one to the
 * document and handle events that bubble up. This also allows other scripts
 * to bind their own handlers to links and also to prevent overlay's handling.
 */
Backdrop.overlayChild.behaviors.addClickHandler = function (context, settings) {
  $(document).bind('click.backdrop-overlay mouseup.backdrop-overlay', $.proxy(parent.Backdrop.overlay, 'eventhandlerOverrideLink'));
};

/**
 * Modify forms depending on their relation to the overlay.
 *
 * By default, forms are assumed to keep the flow in the overlay. Thus their
 * action attribute get a ?render=overlay suffix.
 */
Backdrop.overlayChild.behaviors.parseForms = function (context, settings) {
  $('form', context).once('overlay', function () {
    // Obtain the action attribute of the form.
    var action = $(this).attr('action');
    // Keep internal forms in the overlay.
    if (action == undefined || (action.indexOf('http') != 0 && action.indexOf('https') != 0)) {
      action += (action.indexOf('?') > -1 ? '&' : '?') + 'render=overlay';
      $(this).attr('action', action);
    }
    // Submit external forms into a new window.
    else {
      $(this).attr('target', '_new');
    }
  });
};

/**
 * Replace the overlay title with a message while loading another page.
 */
Backdrop.overlayChild.behaviors.loading = function (context, settings) {
  var $title;
  var text = Backdrop.t('Loading');
  var dots = '';

  $(document).bind('backdropOverlayBeforeLoad.backdrop-overlay.backdrop-overlay-child-loading', function () {
    $title = $('#overlay-title').text(text);
    var id = setInterval(function () {
      dots = (dots.length > 10) ? '' : dots + '.';
      $title.text(text + dots);
    }, 500);
  });
};

/**
 * Switch active tab immediately.
 */
Backdrop.overlayChild.behaviors.tabs = function (context, settings) {
  var $tabsLinks = $('#overlay-tabs > li > a');

  $('#overlay-tabs > li > a').bind('click.backdrop-overlay', function () {
    var active_tab = Backdrop.t('(active tab)');
    $tabsLinks.parent().siblings().removeClass('active').find('element-invisible:contains(' + active_tab + ')').appendTo(this);
    $(this).parent().addClass('active');
  });
};

/**
 * If the shortcut add/delete button exists, move it to the overlay titlebar.
 */
Backdrop.overlayChild.behaviors.shortcutAddLink = function (context, settings) {
  // Remove any existing shortcut button markup from the titlebar.
  $('#overlay-titlebar').find('.add-or-remove-shortcuts').remove();
  // If the shortcut add/delete button exists, move it to the titlebar.
  var $addToShortcuts = $('.add-or-remove-shortcuts');
  if ($addToShortcuts.length) {
    $addToShortcuts.insertAfter('#overlay-title');
  }

  $(document).bind('backdropOverlayBeforeLoad.backdrop-overlay.backdrop-overlay-child-loading', function () {
    $('#overlay-titlebar').find('.add-or-remove-shortcuts').remove();
  });
};

/**
 * Use displacement from parent window.
 */
Backdrop.overlayChild.behaviors.alterTableHeaderOffset = function (context, settings) {
  if (Backdrop.settings.tableHeaderOffset) {
    Backdrop.overlayChild.prevTableHeaderOffset = Backdrop.settings.tableHeaderOffset;
  }
  Backdrop.settings.tableHeaderOffset = 'Backdrop.overlayChild.tableHeaderOffset';
};

/**
 * Callback for Backdrop.settings.tableHeaderOffset.
 */
Backdrop.overlayChild.tableHeaderOffset = function () {
  var topOffset = Backdrop.overlayChild.prevTableHeaderOffset ? eval(Backdrop.overlayChild.prevTableHeaderOffset + '()') : 0;

  return topOffset + parseInt($(document.body).css('marginTop'));
};

})(jQuery);
