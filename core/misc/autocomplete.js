(function ($) {

/**
 * Attaches the autocomplete behavior to all required fields.
 */
Backdrop.behaviors.autocomplete = {
  attach: function (context, settings) {
    var acdb = [];
    $('input.autocomplete', context).once('autocomplete', function () {
      var uri = this.value;
      if (!acdb[uri]) {
        acdb[uri] = new Backdrop.ACDB(uri);
      }
      var $input = $('#' + this.id.substr(0, this.id.length - 13))
        .attr('autocomplete', 'OFF')
        .attr('aria-autocomplete', 'list');
      $($input[0].form).submit(Backdrop.autocompleteSubmit);
      $input.parent()
        .attr('role', 'application')
        .append($('<span class="element-invisible" aria-live="assertive"></span>')
          .attr('id', $input[0].id + '-autocomplete-aria-live')
        );
      new Backdrop.jsAC($input, acdb[uri]);
    });
  }
};

/**
 * Prevents the form from submitting if the suggestions popup is open
 * and closes the suggestions popup when doing so.
 */
Backdrop.autocompleteSubmit = function () {
  var $autocomplete = $('#autocomplete');
  if ($autocomplete.length !== 0) {
    $autocomplete[0].owner.hidePopup();
  }
  return $autocomplete.length === 0;
};

/**
 * An AutoComplete object.
 */
Backdrop.jsAC = function ($input, db) {
  var ac = this;
  this.input = $input[0];
  this.ariaLive = $('#' + this.input.id + '-autocomplete-aria-live');
  this.db = db;

  $input
    .keydown(function (event) { return ac.onkeydown(this, event); })
    .keyup(function (event) { ac.onkeyup(this, event); })
    .blur(function () { ac.hidePopup(); ac.db.cancel(); });
};

/**
 * Handler for the "keydown" event.
 */
Backdrop.jsAC.prototype.onkeydown = function (input, e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 40: // down arrow.
      this.selectDown();
      return false;
    case 38: // up arrow.
      this.selectUp();
      return false;
    default: // All other keys.
      return true;
  }
};

/**
 * Handler for the "keyup" event.
 */
Backdrop.jsAC.prototype.onkeyup = function (input, e) {
  if (!e) {
    e = window.event;
  }
  switch (e.keyCode) {
    case 16: // Shift.
    case 17: // Ctrl.
    case 18: // Alt.
    case 20: // Caps lock.
    case 33: // Page up.
    case 34: // Page down.
    case 35: // End.
    case 36: // Home.
    case 37: // Left arrow.
    case 38: // Up arrow.
    case 39: // Right arrow.
    case 40: // Down arrow.
      return true;

    case 9:  // Tab.
    case 13: // Enter.
    case 27: // Esc.
      this.hidePopup(e.keyCode);
      return true;

    default: // All other keys.
      if (input.value.length > 0 && !input.readOnly) {
        this.populatePopup();
      }
      else {
        this.hidePopup(e.keyCode);
      }
      return true;
  }
};

/**
 * Puts the currently highlighted suggestion into the autocomplete field.
 */
Backdrop.jsAC.prototype.select = function (node) {
  this.input.value = $(node).data('autocompleteValue');
};

/**
 * Highlights the next suggestion.
 */
Backdrop.jsAC.prototype.selectDown = function () {
  if (this.selected && this.selected.nextSibling) {
    this.highlight(this.selected.nextSibling);
  }
  else if (this.popup) {
    var lis = $('li', this.popup);
    if (lis.length > 0) {
      this.highlight(lis.get(0));
    }
  }
};

/**
 * Highlights the previous suggestion.
 */
Backdrop.jsAC.prototype.selectUp = function () {
  if (this.selected && this.selected.previousSibling) {
    this.highlight(this.selected.previousSibling);
  }
};

/**
 * Highlights a suggestion.
 */
Backdrop.jsAC.prototype.highlight = function (node) {
  // Unhighlights a suggestion for "keyup" and "keydown" events.
  if (this.selected !== false) {
    $(this.selected).removeClass('selected');
  }
  $(node).addClass('selected');
  this.selected = node;
  $(this.ariaLive).html($(this.selected).html());
};

/**
 * Unhighlights a suggestion.
 */
Backdrop.jsAC.prototype.unhighlight = function (node) {
  $(node).removeClass('selected');
  this.selected = false;
  $(this.ariaLive).empty();
};

/**
 * Hides the autocomplete suggestions.
 */
Backdrop.jsAC.prototype.hidePopup = function (keycode) {
  // Select item if the right key or mousebutton was pressed.
  if (this.selected && ((keycode && keycode !== 46 && keycode !== 8 && keycode !== 27) || !keycode)) {
    this.input.value = $(this.selected).data('autocompleteValue');
  }
  // Hide popup.
  var popup = this.popup;
  if (popup) {
    this.popup = null;
    $(popup).fadeOut('fast', function () { $(popup).remove(); });
  }
  this.selected = false;
  $(this.ariaLive).empty();
};

/**
 * Positions the suggestions popup and starts a search.
 */
Backdrop.jsAC.prototype.populatePopup = function () {
  var $input = $(this.input);

  // Show popup.
  if (this.popup) {
    $(this.popup).remove();
  }
  this.selected = false;
  this.popup = $('<div id="autocomplete"></div>')[0];
  this.popup.owner = this;

  // Add the popup to the page and position.
  $("body").prepend(this.popup);
  var autocompletInstance = this;
  positionPopup();
  Backdrop.optimizedResize.add(positionPopup, 'autocompletePopup');

  // Do search.
  this.db.owner = this;
  this.db.search(this.input.value);

  function positionPopup() {
    // If the popup has been removed, remove this resize handler.
    if (!autocompletInstance.popup) {
      Backdrop.optimizedResize.remove('autocompletePopup');
      return;
    }

    var offset = $input.offset();
    var paddingLeft = parseInt($input.css('padding-left').replace('px', ''), 10);
    var paddingRight = parseInt($input.css('padding-right').replace('px', ''), 10);
    var paddingWidth = paddingLeft + paddingRight;

    // Because we use "fixed" position, the final location is the offset from
    // the document and the height of the element, minus scroll bar position.
    $(autocompletInstance.popup).css({
      top: ($input.outerHeight() + offset.top - $(document).scrollTop()) + 'px',
      left: (offset.left - $(document).scrollLeft()) + 'px',
      width: ($input.width() + paddingWidth) + 'px',
      position: 'fixed'
    });
  }
};

/**
 * Fills the suggestion popup with any matches received.
 */
Backdrop.jsAC.prototype.found = function (matches) {
  // If no value in the textfield, do not show the popup.
  if (!this.input.value.length) {
    return false;
  }

  // Prepare matches.
  var ac = this;
  var ul = $('<ul></ul>')
    .on('mousedown', 'li', function (e) { ac.select(this); })
    .on('mouseover', 'li', function (e) { ac.highlight(this); })
    .on('mouseout', 'li', function (e) { ac.unhighlight(this); });
  for (var key in matches) {
    if (matches.hasOwnProperty(key)) {
      $('<li></li>')
        .html($('<div></div>').html(matches[key]))
        .data('autocompleteValue', key)
        .appendTo(ul);
    }
  }

  // Show popup with matches, if any.
  if (this.popup) {
    if (ul.children().length) {
      $(this.popup).empty().append(ul).show();
      $(this.ariaLive).html(Backdrop.t('Autocomplete popup'));
    }
    else {
      $(this.popup).css({ visibility: 'hidden' });
      this.hidePopup();
    }
  }
};

Backdrop.jsAC.prototype.setStatus = function (status) {
  switch (status) {
    case 'begin':
      $(this.input).addClass('throbbing');
      $(this.ariaLive).html(Backdrop.t('Searching for matches...'));
      break;
    case 'cancel':
    case 'error':
    case 'found':
      $(this.input).removeClass('throbbing');
      break;
  }
};

/**
 * An AutoComplete DataBase object.
 */
Backdrop.ACDB = function (uri) {
  this.uri = uri;
  this.delay = 300;
  this.cache = {};
};

/**
 * Performs a cached and delayed search.
 */
Backdrop.ACDB.prototype.search = function (searchString) {
  var db = this;
  this.searchString = searchString;

  // See if this string needs to be searched for anyway. The pattern ../ is
  // stripped since it may be misinterpreted by the browser.
  searchString = searchString.replace(/^\s+|\.{2,}\/|\s+$/g, '');
  // Skip empty search strings, or search strings ending with a comma, since
  // that is the separator between search terms.
  if (searchString.length <= 0 ||
    searchString.charAt(searchString.length - 1) === ',') {
    return;
  }

  // See if this key has been searched for before.
  if (this.cache[searchString]) {
    return this.owner.found(this.cache[searchString]);
  }

  // Initiate delayed search.
  if (this.timer) {
    clearTimeout(this.timer);
  }
  this.timer = setTimeout(function () {
    db.owner.setStatus('begin');

    // Ajax GET request for autocompletion.
    $.ajax({
      type: 'GET',
      url: db.uri + '/' + encodeURIComponent(searchString),
      dataType: 'json',
      success: function (matches) {
        if (typeof matches.status === 'undefined' || matches.status !== 0) {
          db.cache[searchString] = matches;
          // Verify if these are still the matches the user wants to see.
          if (db.searchString === searchString) {
            db.owner.found(matches);
          }
          db.owner.setStatus('found');
        }
      },
      error: function (xmlhttp) {
        Backdrop.displayAjaxError(Backdrop.ajaxError(xmlhttp, db.uri));
      }
    });
  }, this.delay);
};

/**
 * Cancels the current autocomplete request.
 */
Backdrop.ACDB.prototype.cancel = function () {
  if (this.owner) {
    this.owner.setStatus('cancel');
  }
  if (this.timer) {
    clearTimeout(this.timer);
  }
  this.searchString = '';
};

})(jQuery);
