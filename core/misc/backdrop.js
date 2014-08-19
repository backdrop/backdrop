/**
 * Main JavaScript file for Backdrop CMS.
 *
 * This file provides central functionality to Backdrop, including attaching
 * behaviors (the main way of interacting with JS in Backdrop), theming
 * wrappers, and localization functions.
 */
window.Backdrop = { 'settings': {}, 'behaviors': {}, 'locale': {} };

// Class indicating that JS is enabled; used for styling purpose.
document.documentElement.className += ' js';

// JavaScript should be made compatible with libraries other than jQuery by
// wrapping it in an anonymous closure.
(function (domready, undefined) {

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

// Attach all behaviors.
domready(function () { Backdrop.attachBehaviors(document, BackdropSettings); });

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
 *     do anything simply in response to the element being moved, but because
 *     IFRAME elements reload their "src" when being moved within the DOM,
 *     behaviors bound to IFRAME elements (like WYSIWYG editors) may need to
 *     take some action.
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
       str = str.replace(key, args[key]);
    }
  }
  return str;
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
 * Encodes a Backdrop path for use in a URL.
 *
 * For aesthetic reasons slashes are not escaped.
 */
Backdrop.encodePath = function (item) {
  return window.encodeURIComponent(item).replace(/%2F/g, '/');
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
 * Formats text for emphasized display in a placeholder inside a sentence.
 *
 * @param str
 *   The text to format (plain-text).
 * @return
 *   The formatted text (html).
 */
Backdrop.theme.placeholder = function (str) {
  return '<em class="placeholder">' + Backdrop.checkPlain(str) + '</em>';
};

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

})(domready);
