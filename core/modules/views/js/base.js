/**
 * @file
 * Some basic behaviors and utility functions for Views.
 */
(function ($) {

Backdrop.Views = {};

/**
 * Keep the original beforeSubmit method to be available for overrides.
 */
Backdrop.Views.beforeSubmit = Backdrop.ajax.prototype.beforeSubmit;

/**
 * Keep the original beforeSerialize method to be available for overrides.
 */
Backdrop.Views.beforeSerialize = Backdrop.ajax.prototype.beforeSerialize;

/**
 * Keep the original beforeSend method to be available for overrides.
 */
Backdrop.Views.beforeSend = Backdrop.ajax.prototype.beforeSend;

/**
 * Helper function to parse a querystring.
 */
Backdrop.Views.parseQueryString = function (query) {
  var args = {};
  var pos = query.indexOf('?');
  if (pos != -1) {
    query = query.substring(pos + 1);
  }
  var pair;
  var pairs = query.split('&');
  var pair, key, value;
  for (var i in pairs) {
    if (typeof (pairs[i]) == 'string') {
      pair = pairs[i].split('=');
      // Ignore the 'q' path argument, if present.
      if (pair[0] != 'q' && pair[1]) {
        key = decodeURIComponent(pair[0].replace(/\+/g, ' '));
        value = decodeURIComponent(pair[1].replace(/\+/g, ' '));
        // Field name ends with [], it's multi-values.
        if (/\[\]$/.test(key)) {
          if (!(key in args)) {
            args[key] = [value];
          }
          // Don't duplicate values.
          else if (!$.inArray(value, args[key]) !== -1) {
            args[key].push(value);
          }
        }
        else {
          args[key] = value;
        }
      }
    }
  }
  return args;
};

/**
 * Helper function to return a view's arguments based on a path.
 */
Backdrop.Views.parseViewArgs = function (href, viewPath) {
  var returnObj = {};
  var path = Backdrop.Views.getPath(href);
  // Ensure we have a correct path.
  if (viewPath && path.substring(0, viewPath.length + 1) == viewPath + '/') {
    var args = decodeURIComponent(path.substring(viewPath.length + 1, path.length));
    returnObj.view_args = args;
    returnObj.view_path = path;
  }
  return returnObj;
};

/**
 * Strip off the protocol plus domain from an href.
 */
Backdrop.Views.pathPortion = function (href) {
  // Remove e.g. http://example.com if present.
  var protocol = window.location.protocol;
  if (href.substring(0, protocol.length) == protocol) {
    // 2 is the length of the '//' that normally follows the protocol
    href = href.substring(href.indexOf('/', protocol.length + 2));
  }
  return href;
};

/**
 * Return the Backdrop path portion of an href.
 */
Backdrop.Views.getPath = function (href) {
  href = Backdrop.Views.pathPortion(href);
  href = href.substring(Backdrop.settings.basePath.length, href.length);
  // 3 is the length of the '?q=' added to the url without clean urls.
  if (href.substring(0, 3) == '?q=') {
    href = href.substring(3, href.length);
  }
  var chars = ['#', '?', '&'];
  for (i = 0; i < chars.length; i++) {
    if (href.indexOf(chars[i]) > -1) {
      href = href.substr(0, href.indexOf(chars[i]));
    }
  }
  return href;
};

/**
 * Strip views values and duplicates from URL.
 *
 * @param url
 *   String with the full URL to clean up.
 * @param viewArgs
 *   Object containing field values from views.
 *
 * @return url
 *   String URL with views values and reduced duplicates.
 */
Backdrop.Views.cleanURL = function (url, viewArgs) {
  var args = ('reset' in viewArgs) ? {} : Backdrop.Views.parseQueryString(url);
  var query = [];

  // With clean urls off we need to add the 'q' parameter.
  if (/\?/.test(Backdrop.settings.views.ajax_path)) {
    query.push('q=' + Backdrop.Views.getPath(url));
  }

  $.each(args, function (name, value) {
    // Use values from viewArgs if they exists.
    if (name in viewArgs) {
      value = viewArgs[name];
    }
    if (Array.isArray(value)) {
      $.merge(query, $.map($.uniqueSort(value), function (sub) {
        return encodeURIComponent(name) + '=' + encodeURIComponent(sub);
      }));
    }
    else {
      query.push(encodeURIComponent(name) + '=' + encodeURIComponent(value));
    }
  });

  url = window.location.href.split('?');
  return url[0] + (query.length ? '?' + query.join('&') : '');
};

/**
 * Remove the functions from the state. They can't be pushed into the history.
 *
 * @param state
 *  Object containing the state to be cleaned.
 *
 * @return state
 *  Object that has been cleaned up.
 */
Backdrop.Views.cleanStateForHistory = function (state) {
  var stateWithNoFunctions = {};
  for (var key in state) {
    if (typeof state[key] !== "function") {
      stateWithNoFunctions[key] = state[key];
    }
  }
  return stateWithNoFunctions;
};

/**
 * Parse a URL query string.
 *
 * @param queryString
 *   String containing the query to parse.
 */
Backdrop.Views.parseQuery = function (queryString) {
  var query = {};
  $.map(queryString.split('&'), function (val) {
    var s = val.split('=');
    query[s[0]] = s[1];
  });
  return query;
};

/**
 * Remove 'popstate' handler when adding a new state to avoid an infinite loop.
 *
 * We only use the 'popstate' event to trigger refresh on back or forward click.
 *
 * @param options
 *   Object containing the values from views' AJAX call.
 * @param url
 *   String with the current URL to be cleaned up.
 */
Backdrop.Views.addState = function (options, url) {
  // The data in the history state must be serializable.
  var historyOptions = $.extend({}, options);

  // Store the actual view's dom id.
  Backdrop.settings.lastViewDomID = options.data.view_dom_id;
  $(window).off('popstate', Backdrop.Views.loadView);
  history.pushState(Backdrop.Views.cleanStateForHistory(historyOptions), document.title, Backdrop.Views.cleanURL(url, options.data));
  $(window).on('popstate', Backdrop.Views.loadView);
};

/**
 * Make an AJAX request to update the view when navigating back and forth.
 */
Backdrop.Views.loadView = function () {
  var options;

  // This should be the first loaded page, so init the options object.
  if (history.state === null) {
    var viewsAjaxSettingsKey = 'views_dom_id:' + Backdrop.settings.lastViewDomID;
    if (Backdrop.settings.views.ajaxViews.hasOwnProperty(viewsAjaxSettingsKey)) {
      var viewsAjaxSettings = Backdrop.settings.views.ajaxViews[viewsAjaxSettingsKey];
      var initial_ajax_exposed_input = Backdrop.settings.initial_ajax_exposed_input[viewsAjaxSettingsKey];
      $.extend(viewsAjaxSettings, initial_ajax_exposed_input);
      viewsAjaxSettings.page = Backdrop.settings.views.ajaxViews.onload_page_item;
      options = {
        data: viewsAjaxSettings,
        url: Backdrop.settings.views.ajax_path
      };
    }
  }
  else {
    options = history.state;
  }

  // Need an element to trigger Backdrop's AJAX call.
  var $trigger = $('<div class="ajax-history-trigger"/>');

  // Backdrop's AJAX options.
  var settings = $.extend({
    submit: options.data,
    setClick: true,
    event: 'click',
    selector: '.view-dom-id-' + options.data.view_dom_id,
    progress: { type: 'throbber' },
    httpMethod: 'GET',
  }, options);

  new Backdrop.ajax(false, $trigger[0], settings);
  // Trigger ajax call.
  // @todo check there is no leak, $trigger is never destroyed.
  $trigger.trigger('click');
};

})(jQuery);
