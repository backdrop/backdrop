/**
 * @file
 * Some basic behaviors and utility functions for Views.
 */
(function ($) {

Backdrop.Views = {};

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
  for (var i = 0; i < pairs.length; i++) {
    pair = pairs[i].split('=');
    // Ignore the 'q' path argument, if present.
    if (pair[0] !== 'q' && pair[1]) {
      args[decodeURIComponent(pair[0].replace(/\+/g, ' '))] = decodeURIComponent(pair[1].replace(/\+/g, ' '));
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

})(jQuery);
