/**
 * Main JavaScript file for Backdrop CMS.
 *
 * This file provides central functionality to Backdrop, including attaching
 * behaviors (the main way of interacting with JS in Backdrop), theme wrappers,
 * and localization functions.
 */
(function ($, undefined) {

// Define the main Backdrop object that holds settings, behavior callbacks, and
// translated strings.
window.Backdrop = window.Backdrop || {};
$.extend(true, window.Backdrop, { 'settings': {}, 'behaviors': {}, 'locale': {} });

// Alias the Backdrop namespace to Drupal if compatibility is enabled.
if (Backdrop.settings.drupalCompatibility) {
  window.Drupal = Backdrop;
}

// Pre-filter Ajax requests to guard against XSS attacks.
// This is similar to the fix that is built in to jQuery 3.0 and higher.
// See https://github.com/jquery/jquery/issues/2432
$.ajaxPrefilter(function (s) {
  if (s.crossDomain) {
    s.contents.script = false;
  }
});

  /**
 * Attach all registered behaviors to a page element.
 *
 * Behaviors are event-triggered actions that attach to page elements, enhancing
 * default non-JavaScript UIs. Behaviors are registered in the Backdrop.behaviors
 * object using the method 'attach' and optionally also 'detach' as follows:
 * @code
 *    Backdrop.behaviors.behaviorName = {
 *      attach: function (context, settings) {
 *        ...
 *      },
 *      detach: function (context, settings, trigger) {
 *        ...
 *      }
 *    };
 * @endcode
 *
 * Backdrop.attachBehaviors is added below to the jQuery.ready event and
 * therefore runs on initial page load. Developers implementing Ajax in their
 * solutions should also call this function after new page content has been
 * loaded, feeding in an element to be processed, in order to attach all
 * behaviors to the new content.
 *
 * Behaviors should use
 * @code
 *   var elements = $(context).find(selector).once('behavior-name');
 * @endcode
 * to ensure the behavior is attached only once to a given element. (Doing so
 * enables the reprocessing of given elements, which may be needed on occasion
 * despite the ability to limit behavior attachment to a particular element.)
 *
 * @param context
 *   An element to attach behaviors to. If none is given, the document element
 *   is used.
 * @param settings
 *   An object containing settings for the current context. If none is given,
 *   the global Backdrop.settings object is used.
 */
Backdrop.attachBehaviors = function (context, settings) {
  context = context || document;
  settings = settings || Backdrop.settings;
  // Execute all of them.
  $.each(Backdrop.behaviors, function () {
    if ($.isFunction(this.attach)) {
      this.attach(context, settings);
    }
  });
};

/**
 * Detach registered behaviors from a page element.
 *
 * Developers implementing AHAH/Ajax in their solutions should call this
 * function before page content is about to be removed, feeding in an element
 * to be processed, in order to allow special behaviors to detach from the
 * content.
 *
 * Such implementations should look for the class name that was added in their
 * corresponding Backdrop.behaviors.behaviorName.attach implementation, i.e.
 * behaviorName-processed, to ensure the behavior is detached only from
 * previously processed elements.
 *
 * @param context
 *   An element to detach behaviors from. If none is given, the document element
 *   is used.
 * @param settings
 *   An object containing settings for the current context. If none given, the
 *   global Backdrop.settings object is used.
 * @param trigger
 *   A string containing what's causing the behaviors to be detached. The
 *   possible triggers are:
 *   - unload: (default) The context element is being removed from the DOM.
 *   - move: The element is about to be moved within the DOM (for example,
 *     during a tabledrag row swap). After the move is completed,
 *     Backdrop.attachBehaviors() is called, so that the behavior can undo
 *     whatever it did in response to the move. Many behaviors won't need to
 *     do anything in response to the element being moved, but because IFRAME
 *     elements reload their "src" when being moved within the DOM, behaviors
 *     bound to IFRAME elements (like WYSIWYG editors) may need to take some
 *     action.
 *   - serialize: When an Ajax form is submitted, this is called with the
 *     form as the context. This provides every behavior within the form an
 *     opportunity to ensure that the field elements have correct content
 *     in them before the form is serialized. The canonical use-case is so
 *     that WYSIWYG editors can update the hidden textarea to which they are
 *     bound.
 *
 * @see Backdrop.attachBehaviors
 */
Backdrop.detachBehaviors = function (context, settings, trigger) {
  context = context || document;
  settings = settings || Backdrop.settings;
  trigger = trigger || 'unload';
  // Execute all of them.
  $.each(Backdrop.behaviors, function () {
    if ($.isFunction(this.detach)) {
      this.detach(context, settings, trigger);
    }
  });
};

/**
 * Encode special characters in a plain-text string for display as HTML.
 *
 * @param str
 *   The string to be encoded.
 * @return
 *   The encoded string.
 * @ingroup sanitization
 */
Backdrop.checkPlain = function (str) {
  str = str.toString()
    .replace(/&/g, '&amp;')
    .replace(/'/g, '&#39;')
    .replace(/"/g, '&quot;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
  return str;
};

/**
 * Replace placeholders with sanitized values in a string.
 *
 * @param str
 *   A string with placeholders.
 * @param args
 *   An object of replacements pairs to make. Incidences of any key in this
 *   array are replaced with the corresponding value. Based on the first
 *   character of the key, the value is escaped and/or themed:
 *    - !variable: inserted as is
 *    - @variable: escape plain text to HTML (Backdrop.checkPlain)
 *    - %variable: escape text and theme as a placeholder for user-submitted
 *      content (checkPlain + Backdrop.theme('placeholder'))
 *
 * @see Backdrop.t()
 * @ingroup sanitization
 */
Backdrop.formatString = function(str, args) {
  // Transform arguments before inserting them.
  for (var key in args) {
    if (args.hasOwnProperty(key)) {
      switch (key.charAt(0)) {
        // Escaped only.
        case '@':
          args[key] = Backdrop.checkPlain(args[key]);
          break;
        // Pass-through.
        case '!':
          break;
        // Escaped and placeholder.
        case '%':
          default:
          args[key] = Backdrop.theme('placeholder', args[key]);
          break;
      }
    }
  }
  return Backdrop.stringReplace(str, args, null);
};

/**
 * Generates a string representation for the given byte count.
 *
 * @param size
 *   A size in bytes.
 * @param langcode
 *   Optional language code to translate to a language other than what is used
 *   to display the page.
 *
 * @return
 *   A translated string representation of the size.
 *
 * @since Backdrop 1.11.0
 */
Backdrop.formatSize = function(size, langcode) {
  var kilobyte = 1024;
  if (size < kilobyte) {
    return Backdrop.formatPlural(size, '1 byte', '@count bytes', {}, {'langcode': langcode });
  }
  else {
    // Convert bytes to kilobytes and round.
    size = Number.parseFloat(size / kilobyte).toFixed(2);
    var units = [
      Backdrop.t('@size KB', {}, {'langcode': langcode}),
      Backdrop.t('@size MB', {}, {'langcode': langcode}),
      Backdrop.t('@size GB', {}, {'langcode': langcode}),
      Backdrop.t('@size TB', {}, {'langcode': langcode})
    ];
    var unit;
    for (var n = 0; n < units.length; n++) {
      unit = units[n];
      if (size >= kilobyte) {
        size = Number.parseFloat(size / kilobyte).toFixed(2);
      }
      else {
        break;
      }
    }
    return unit.replace('@size', size);
  }
};

/**
 * Replace substring.
 *
 * The longest keys will be tried first. Once a substring has been replaced,
 * its new value will not be searched again.
 *
 * @param {String} str
 *   A string with placeholders.
 * @param {Object} args
 *   Key-value pairs.
 * @param {Array|null} keys
 *   Array of keys from the "args".  Internal use only.
 *
 * @return {String}
 *   Returns the replaced string.
 */
Backdrop.stringReplace = function (str, args, keys) {
  if (str.length === 0) {
    return str;
  }

  // If the array of keys is not passed then collect the keys from the args.
  if (!$.isArray(keys)) {
    keys = [];
    for (var k in args) {
      if (args.hasOwnProperty(k)) {
        keys.push(k);
      }
    }

    // Order the keys by the character length. The shortest one is the first.
    keys.sort(function (a, b) { return a.length - b.length; });
  }

  if (keys.length === 0) {
    return str;
  }

  // Take next longest one from the end.
  var key = keys.pop();
  var fragments = str.split(key);

  if (keys.length) {
    for (var i = 0; i < fragments.length; i++) {
      // Process each fragment with a copy of remaining keys.
      fragments[i] = Backdrop.stringReplace(fragments[i], args, keys.slice(0));
    }
  }

  return fragments.join(args[key]);
};

/**
 * Translate strings to the page language or a given language.
 *
 * See the documentation of the server-side t() function for further details.
 *
 * @param str
 *   A string containing the English string to translate.
 * @param args
 *   An object of replacements pairs to make after translation. Incidences
 *   of any key in this array are replaced with the corresponding value.
 *   See Backdrop.formatString().
 *
 * @param options
 *   - 'context' (defaults to the empty context): The context the source string
 *     belongs to.
 *
 * @return
 *   The translated string.
 */
Backdrop.t = function (str, args, options) {
  options = options || {};
  options.context = options.context || '';

  // Fetch the localized version of the string.
  if (Backdrop.locale.strings && Backdrop.locale.strings[options.context] && Backdrop.locale.strings[options.context][str]) {
    str = Backdrop.locale.strings[options.context][str];
  }

  if (args) {
    str = Backdrop.formatString(str, args);
  }
  return str;
};

/**
 * Format a string containing a count of items.
 *
 * This function ensures that the string is pluralized correctly. Since Backdrop.t() is
 * called by this function, make sure not to pass already-localized strings to it.
 *
 * See the documentation of the server-side format_plural() function for further details.
 *
 * @param count
 *   The item count to display.
 * @param singular
 *   The string for the singular case. Please make sure it is clear this is
 *   singular, to ease translation (e.g. use "1 new comment" instead of "1 new").
 *   Do not use @count in the singular string.
 * @param plural
 *   The string for the plural case. Please make sure it is clear this is plural,
 *   to ease translation. Use @count in place of the item count, as in "@count
 *   new comments".
 * @param args
 *   An object of replacements pairs to make after translation. Incidences
 *   of any key in this array are replaced with the corresponding value.
 *   See Backdrop.formatString().
 *   Note that you do not need to include @count in this array.
 *   This replacement is done automatically for the plural case.
 * @param options
 *   The options to pass to the Backdrop.t() function.
 * @return
 *   A translated string.
 */
Backdrop.formatPlural = function (count, singular, plural, args, options) {
  args = args || {};
  args['@count'] = count;
  // Determine the index of the plural form.
  var index = Backdrop.locale.pluralFormula ? Backdrop.locale.pluralFormula(args['@count']) : ((args['@count'] == 1) ? 0 : 1);

  if (index == 0) {
    return Backdrop.t(singular, args, options);
  }
  else if (index == 1) {
    return Backdrop.t(plural, args, options);
  }
  else {
    args['@count[' + index + ']'] = args['@count'];
    delete args['@count'];
    return Backdrop.t(plural.replace('@count', '@count[' + index + ']'), args, options);
  }
};

/**
 * Returns the passed in URL as an absolute URL.
 *
 * @param url
 *   The URL string to be normalized to an absolute URL.
 *
 * @return
 *   The normalized, absolute URL.
 *
 * @see https://github.com/angular/angular.js/blob/v1.4.4/src/ng/urlUtils.js
 * @see https://grack.com/blog/2009/11/17/absolutizing-url-in-javascript
 * @see https://github.com/jquery/jquery-ui/blob/1.11.4/ui/tabs.js#L53
 */
Backdrop.absoluteUrl = function (url) {
  var urlParsingNode = document.createElement('a');

  // Decode the URL first; this is required by IE <= 6. Decoding non-UTF-8
  // strings may throw an exception.
  try {
    url = decodeURIComponent(url);
  } catch (e) {}

  urlParsingNode.setAttribute('href', url);

  // IE <= 7 normalizes the URL when assigned to the anchor node similar to
  // the other browsers.
  return urlParsingNode.cloneNode(false).href;
};

/**
 * Returns the passed in URL as a relative URL to the current site.
 *
 * Relative URLs are returned only for local URLs. This will return the full
 * URL for remote URLs.
 *
 * @param url
 *   The URL string to be normalized to a relative URL.
 *
 * @return
 *   The normalized, relative URL with a leading slash.
 *
 * @since 1.11.0
 */
Backdrop.relativeUrl = function (url) {
  // Normalize to absolute first.
  relativeUrl = Backdrop.absoluteUrl(url);
  // Port is only present on non-HTTP(S) URLs.
  var port = window.location.port ? (':' + window.location.port) : '';
  return relativeUrl.replace(window.location.protocol + '//' + window.location.hostname + port, '');
};

/**
 * Sanitizes a URL for use with jQuery.ajax().
 *
 * @param url
 *   The URL string to be sanitized.
 *
 * @return
 *   The sanitized URL.
 */
Backdrop.sanitizeAjaxUrl = function (url) {
  var regex = /\=\?(&|$)/;
  while (url.match(regex)) {
    url = url.replace(regex, '');
  }
  return url;
}

/**
 * Returns true if the URL is within Backdrop's base path.
 *
 * @param url
 *   The URL string to be tested.
 *
 * @return
 *   Boolean true if local.
 *
 * @see https://github.com/jquery/jquery-ui/blob/1.11.4/ui/tabs.js#L58
 */
Backdrop.urlIsLocal = function (url) {
  // Always use browser-derived absolute URLs in the comparison, to avoid
  // attempts to break out of the base path using directory traversal.
  var absoluteUrl = Backdrop.absoluteUrl(url);
  var protocol = location.protocol;

  // Consider URLs that match this site's base URL but use HTTPS instead of HTTP
  // as local as well.
  if (protocol === 'http:' && absoluteUrl.indexOf('https:') === 0) {
    protocol = 'https:';
  }
  var baseUrl = protocol + '//' + location.host + Backdrop.settings.basePath.slice(0, -1);

  // Decoding non-UTF-8 strings may throw an exception.
  try {
    absoluteUrl = decodeURIComponent(absoluteUrl);
  } catch (e) {}
  try {
    baseUrl = decodeURIComponent(baseUrl);
  } catch (e) {}

  // The given URL matches the site's base URL, or has a path under the site's
  // base URL.
  return absoluteUrl === baseUrl || absoluteUrl.indexOf(baseUrl + '/') === 0;
};

/**
 * Generate the themed representation of a Backdrop object.
 *
 * All requests for themed output must go through this function. It examines
 * the request and routes it to the appropriate theme function. If the current
 * theme does not provide an override function, the generic theme function is
 * called.
 *
 * For example, to retrieve the HTML for text that should be emphasized and
 * displayed as a placeholder inside a sentence, call
 * Backdrop.theme('placeholder', text).
 *
 * @param func
 *   The name of the theme function to call.
 * @param ...
 *   Additional arguments to pass along to the theme function.
 * @return
 *   Any data the theme function returns. This could be a plain HTML string,
 *   but also a complex object.
 */
Backdrop.theme = function (func) {
  var args = Array.prototype.slice.apply(arguments, [1]);

  return (Backdrop.theme[func] || Backdrop.theme.prototype[func]).apply(this, args);
};

/**
 * Freeze the current body height (as minimum height). Used to prevent
 * unnecessary upwards scrolling when doing DOM manipulations.
 */
Backdrop.freezeHeight = function () {
  Backdrop.unfreezeHeight();
  $('<div id="freeze-height"></div>').css({
    position: 'absolute',
    top: '0px',
    left: '0px',
    width: '1px',
    height: $('body').css('height')
  }).appendTo('body');
};

/**
 * Unfreeze the body height.
 */
Backdrop.unfreezeHeight = function () {
  $('#freeze-height').remove();
};

/**
 * Encodes a Backdrop path for use in a URL.
 *
 * For aesthetic reasons slashes are not escaped.
 */
Backdrop.encodePath = function (item) {
  return window.encodeURIComponent(item).replace(/%2F/g, '/');
};

/**
 * Get the text selection in a textarea.
 */
Backdrop.getSelection = function (element) {
  var range1, range2, start, end;
  if (typeof element.selectionStart != 'number' && document.selection) {
    // The current selection.
    range1 = document.selection.createRange();
    range2 = range1.duplicate();
    // Select all text.
    range2.moveToElementText(element);
    // Now move 'dummy' end point to end point of original range.
    range2.setEndPoint('EndToEnd', range1);
    // Now we can calculate start and end points.
    start = range2.text.length - range1.text.length;
    end = start + range1.text.length;
    return { 'start': start, 'end': end };
  }
  return { 'start': element.selectionStart, 'end': element.selectionEnd };
};

/**
 * Add a global variable which determines if the window is being unloaded.
 *
 * This is primarily used by Backdrop.displayAjaxError().
 */
Backdrop.beforeUnloadCalled = false;
$(window).bind('beforeunload pagehide', function () {
    Backdrop.beforeUnloadCalled = true;
});

/**
 * Displays a JavaScript error from an Ajax response when appropriate to do so.
 */
Backdrop.displayAjaxError = function (message) {
  // Skip displaying the message if the user deliberately aborted (for example,
  // by reloading the page or navigating to a different page) while the Ajax
  // request was still ongoing. See, for example, the discussion at
  // http://stackoverflow.com/questions/699941/handle-ajax-error-when-a-user-clicks-refresh.
  if (!Backdrop.beforeUnloadCalled) {
    alert(message);
  }
};

/**
 * Build an error message from an Ajax response.
 */
Backdrop.ajaxError = function (xmlhttp, uri, customMessage) {
  var statusCode, statusText, pathText, responseText, readyStateText, message;
  if (xmlhttp.status) {
    statusCode = "\n" + Backdrop.t("An AJAX HTTP error occurred.") +  "\n" + Backdrop.t("HTTP Result Code: !status", {'!status': xmlhttp.status});
  }
  else {
    statusCode = "\n" + Backdrop.t("An AJAX HTTP request terminated abnormally.");
  }
  statusCode += "\n" + Backdrop.t("Debugging information follows.");
  pathText = "\n" + Backdrop.t("Path: !uri", {'!uri': uri} );
  statusText = '';
  // In some cases, when statusCode == 0, xmlhttp.statusText may not be defined.
  // Unfortunately, testing for it with typeof, etc, doesn't seem to catch that
  // and the test causes an exception. So we need to catch the exception here.
  try {
    statusText = "\n" + Backdrop.t("StatusText: !statusText", {'!statusText': $.trim(xmlhttp.statusText)});
  }
  catch (e) {}

  responseText = '';
  // Again, we don't have a way to know for sure whether accessing
  // xmlhttp.responseText is going to throw an exception. So we'll catch it.
  try {
    responseText = "\n" + Backdrop.t("ResponseText: !responseText", {'!responseText': $.trim(xmlhttp.responseText) } );
  } catch (e) {}

  // Make the responseText more readable by stripping HTML tags and newlines.
  responseText = responseText.replace(/<("[^"]*"|'[^']*'|[^'">])*>/gi,"");
  responseText = responseText.replace(/[\n]+\s+/g,"\n");

  // We don't need readyState except for status == 0.
  readyStateText = xmlhttp.status == 0 ? ("\n" + Backdrop.t("ReadyState: !readyState", {'!readyState': xmlhttp.readyState})) : "";

  // Additional message beyond what the xmlhttp object provides.
  customMessage = customMessage ? ("\n" + Backdrop.t("CustomMessage: !customMessage", {'!customMessage': customMessage})) : "";

  message = statusCode + pathText + statusText + customMessage + responseText + readyStateText;
  return message;
};

/**
 * Run a callback after the given font has been loaded.
 *
 * If the font is already loaded, the callback will be executed immediately.
 *
 * @param fontName
 *   Font name as shown in CSS.
 * @param callback
 *   Function to run once font has loaded.
 *
 * @since 1.4.5
 */
Backdrop.isFontLoaded = function(fontName, callback) {
  if (typeof fontName === 'undefined') {
    return false;
  }

  if (typeof Backdrop.fontsLoaded[fontName] === 'undefined') {
    Backdrop.fontsLoaded[fontName] = false;
  }
  else if (Backdrop.fontsLoaded[fontName]) {
    // Fonts loaded, run the callback and don't run anything else.
    if (typeof callback !== 'undefined') {
      callback();
    }
    return true;
  }

  var $body = $('body');
  var checkFontCounter = 0;
  // Append an invisible element that will be monospace font or our desired
  // font. We're using a repeating i because the characters width will
  // drastically change when it's monospace vs. proportional font.
  var $checkFontElement = $('<span id="check-font" style="font-family: \'' + fontName + '\', monospace;">iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii</span>');
  // Control is the same but it will always be our 'control' font, monospace.
  var $checkFontElementControl = $checkFontElement.clone().attr('id', 'check-font-control').css('font-family', 'monospace');
  var $checkFontElements = $('<span id="check-font-wrapper" aria-hidden="true" style="visibility: hidden; position: absolute; z-index: -100;"></span>').append($checkFontElement).append($checkFontElementControl);
  $body.append($checkFontElements);

  // Function to check the width of the font, if it's substantially different
  // we'll know we our real font has loaded
  function checkFont() {
    var currentWidth = $checkFontElement.width();
    var controlWidth = $checkFontElementControl.width();
    if (controlWidth !== currentWidth || checkFontCounter >= 60) {
      Backdrop.fontsLoaded[fontName] = true;
      // If our font has loaded, or it's been 6 seconds
      if (typeof callback !== 'undefined') {
        callback();
      }
      // Clean up after ourselves
      clearInterval(checkFontInterval);
      $checkFontElements.remove();
    }
    checkFontCounter++;
  }
  var checkFontInterval = setInterval(checkFont, 100);

  // If we got here, font is not yet loaded, but the callback will be fired
  // when it is ready.
  return false;
};
if (typeof Backdrop.fontsLoaded === 'undefined') {
  Backdrop.fontsLoaded = {};
}

// Class indicating that JS is enabled; used for styling purpose.
$('html').addClass('js');

/**
 * Additions to jQuery.support.
 */
$(function () {
  /**
   * Boolean indicating whether or not position:fixed is supported.
   */
  if (jQuery.support.positionFixed === undefined) {
    var el = $('<div style="position:fixed; top:10px" />').appendTo(document.body);
    jQuery.support.positionFixed = el[0].offsetTop === 10;
    el.remove();
  }
});

// On page ready, attach behaviors in which all other Backdrop JS is handled.
$(function () {
  Backdrop.attachBehaviors(document, Backdrop.settings);
});

/**
 * The default themes.
 */
Backdrop.theme.prototype = {

  /**
   * Formats text for emphasized display in a placeholder inside a sentence.
   *
   * @param str
   *   The text to format (plain-text).
   * @return
   *   The formatted text (html).
   */
  placeholder: function (str) {
    return '<em class="placeholder">' + Backdrop.checkPlain(str) + '</em>';
  }
};

/**
 * Add helper functions for feature detection
 */
Backdrop.featureDetect = {};

/**
 * Tests for flex-wrap as it's typically most important in flexbox layouts.
 *
 * @return {boolean} True if browser supports flex-wrap.
 *
 * @since 1.4.4
 */
Backdrop.featureDetect.flexbox = function() {
  var $body = $('body'),
      $flexboxTestElement = $('<div style="display: flex; flex-wrap: wrap; width: 0; height: 0;"></div>');

  if ($body.hasClass('has-flexbox')) {
    return true;
  } else if ($body.hasClass('no-flexbox')) {
    return false;
  } else {
    $body.append($flexboxTestElement);
    if ($flexboxTestElement.css('display') === 'flex' && $flexboxTestElement.css('flex-wrap') === 'wrap') {
      $body.addClass('has-flexbox');
      $flexboxTestElement.remove();
      return true;
    }
    else {
      $body.addClass('no-flexbox');
      $flexboxTestElement.remove();
      return false;
    }
  }
};

/**
 * Resource friendly window resize function
 * Groups all window resize functions and runs them only when a frame comes up
 * From: developer.mozilla.org/en-US/docs/Web/Events/resize#requestAnimationFrame_customEvent
 *
 * Example use:
 * @code
 *   Backdrop.optimizedResize.add(function() {
 *     console.log('Smooth AND resource effecient!');
 *   });
 * @endcode
 */
Backdrop.optimizedResize = (function() {
  // Set defaults.
  var callbacks = {};
  var running = false;
  var counter = 0;

  // Fired on resize event.
  function resize() {
    if (!running) {
      running = true;
      // Provide setTimeout fallback to old browsers.
      if (window.requestAnimationFrame) {
        window.requestAnimationFrame(runCallbacks);
      } else {
        setTimeout(runCallbacks, 66);
      }
    }
  }

  // Run the actual callbacks.
  function runCallbacks() {
    for (var callbackName in callbacks) {
      if (callbacks.hasOwnProperty(callbackName)) {
        callbacks[callbackName]();
      }
    }
    running = false;
  }

  // Adds callback to loop.
  function addCallback(callback, callbackName) {
    // Populate a unique callback name if one is not provided.
    callbackName = callbackName || ('callback-' + (counter++));
    if (callback) {
      callbacks[callbackName] = callback;
    }
  }

  // Removes a callback from the list of resize handlers.
  function removeCallback(callbackName) {
    if (callbacks[callbackName]) {
      delete callbacks[callbackName];
    }
  }

  return {
    // Public methods to add/remove callbacks.
    add: function(callback, callbackName) {
      if (!callbacks.length) {
        window.addEventListener('resize', resize);
        window.addEventListener('scroll', resize);
      }
      addCallback(callback, callbackName);
    },
    remove: function(callbackName) {
      removeCallback(callbackName);
    },
    trigger: function() {
      runCallbacks();
    }
  }
}());

/**
 * Limits the invocations of a function in a given time frame.
 *
 * This can be useful to respond to an event that fires very frequently,
 * such as a "keyup" event while a user is typing in a field.
 *
 * A common use of debouncing in other systems is window resizing,
 * however Backdrop provides the Backdrop.optimizedResize() method,
 * which should be used for that purpose.
 *
 * @param function func
 *   The function to be invoked.
 * @param number wait
 *   The time period within which the callback function should only be
 *   invoked once. For example if the wait period is 250ms, then the callback
 *   will only be called at most 4 times per second.
 * @param bool immediate
 *   Whether we wait at the beginning or end to execute the function.
 *
 * @return function
 *   The debounced function.
 *
 * @since 1.18.2 Method added.
 */
Backdrop.debounce = function (func, wait, immediate) {
  var timeout;
  var result;
  return function () {
    for (var _len = arguments.length, args = new Array(_len), _key = 0; _key < _len; _key++) {
      args[_key] = arguments[_key];
    }

    var context = this;

    var later = function later() {
      timeout = null;

      if (!immediate) {
        result = func.apply(context, args);
      }
    };

    var callNow = immediate && !timeout;
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);

    if (callNow) {
      result = func.apply(context, args);
    }

    return result;
  };
};

})(jQuery);
