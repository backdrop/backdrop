/**
 * @file
 * Handles AJAX fetching of views, including filter submission and response.
 */
(function ($) {

"use strict";

// Save the query string from the original URL on page load.
var original = {
  path: window.location.protocol + "//" + window.location.hostname + (window.location.port ? ':' + window.location.port : '') + window.location.pathname,
  query: window.location.search || ''
};

window.addEventListener("pageshow", function (event) {
  if (event.persisted) {
    window.location.reload();
  }
});

/**
 * Attaches the AJAX behavior to Views exposed filter forms and key View links.
 */
Backdrop.behaviors.ViewsAjaxView = {};
Backdrop.behaviors.ViewsAjaxView.attach = function() {
  if (Backdrop.settings && Backdrop.settings.views && Backdrop.settings.views.ajaxViews) {
    $.each(Backdrop.settings.views.ajaxViews, function(i, settings) {
      Backdrop.views.instances[i] = new Backdrop.views.ajaxView(settings);
    });
  }
};

/**
 * Removes configuration and state from the page when a view is removed.
 */
  Backdrop.behaviors.ViewsAjaxView.detach = function(context) {
  if (Backdrop.settings && Backdrop.settings.views && Backdrop.settings.views.ajaxViews) {
    $.each(Backdrop.settings.views.ajaxViews, function(i, settings) {
      var $removedView = $('.view-dom-id-' + settings.view_dom_id, context);
      if ($removedView.length) {
        delete Backdrop.settings.views.ajaxViews[i];
        delete Backdrop.views.instances[i];
      }
    });
  }
};

Backdrop.views = {};
Backdrop.views.instances = {};

/**
 * JavaScript object for a certain view.
 */
Backdrop.views.ajaxView = function(settings) {
  var selector = '.view-dom-id-' + settings.view_dom_id;
  this.$view = $(selector);

  // Retrieve the path to use for views' ajax.
  var ajax_path = Backdrop.settings.views.ajax_path;

  // If there are multiple views this might end up showing multiple times.
  if (ajax_path.constructor.toString().indexOf("Array") !== -1) {
    ajax_path = ajax_path[0];
  }

  // Check if there are any GET parameters to send to views.
  var queryString = window.location.search || '';
  if (queryString !== '') {
    // Remove the question mark and Backdrop path component if any.
    queryString = queryString.slice(1).replace(/q=[^&]+&?|&?render=[^&]+/, '');
    if (queryString !== '') {
      // If there is a '?' in ajax_path, clean url are on and & should be used to add parameters.
      queryString = ((/\?/.test(ajax_path)) ? '&' : '?') + queryString;
    }
  }

  // Init the current page too, because the first loaded pager element do
  // not have loadable history and will not work the back button.
  var $body = $('body').once('views-ajax-history-first-page-load');

  if ($body.length) {
    settings.onload_page_item = settings.render_page_item;
  }

  this.element_settings = {
    url: ajax_path + queryString,
    submit: settings,
    setClick: true,
    event: 'click',
    selector: selector,
    progress: { type: 'throbber' }
  };

  this.settings = settings;

  // Add the ajax to exposed forms.
  let formSelector;
  if (settings.exposed_form_in_block) {
    formSelector = 'form#views-exposed-form-'+ settings.view_name.replace(/_/g, '-') + '-' + settings.view_display_id.replace(/_/g, '-');
  }
  else {
    // If the exposed form is child of the view, the dom id is more reliable,
    // especially if dynamically embedded multiple times per page.
    formSelector = '.view-dom-id-' + settings.view_dom_id + ' form.views-exposed-form';
  }
  this.$exposed_form = $(formSelector);
  this.$exposed_form.once('views-exposed-form', $.proxy(this.attachExposedFormAjax, this));

  // Add the ajax to pagers.
  this.$view
    // Don't attach to nested views. Doing so would attach multiple behaviors
    // to a given element.
    .filter($.proxy(this.filterNestedViews, this))
    .once($.proxy(this.attachPagerAjax, this));

  // In order to trigger a refresh, use the following code:
  // @code
  // $('.view-name').trigger('RefreshView');
  // @endcode
  //
  // Add a trigger to update this view specifically.
  var self_settings = this.element_settings;
  self_settings.event = 'RefreshView';
  this.refreshViewAjax = new Backdrop.ajax(this.selector, this.$view[0], self_settings);
  this.applyAjaxOverrides(this.refreshViewAjax);
};

Backdrop.views.ajaxView.prototype.attachExposedFormAjax = function() {
  var button = $('input[type=submit], button[type=submit], input[type=image]', this.$exposed_form);
  button = button[0];

  this.exposedFormAjax = new Backdrop.ajax($(button).attr('id'), button, this.element_settings);
  this.applyAjaxOverrides(this.exposedFormAjax);
};

Backdrop.views.ajaxView.prototype.filterNestedViews= function() {
  // If there is at least one parent with a view class, this view
  // is nested (e.g., an attachment). Bail.
  return !this.$view.parents('.view').length;
};

/**
 * Attach the ajax behavior to each link.
 */
Backdrop.views.ajaxView.prototype.attachPagerAjax = function() {
  this.$view.find('ul.pager > li > a, th a[data-sort], .attachment .views-summary a')
  .each($.proxy(this.attachPagerLinkAjax, this));
};

/**
 * Attach the ajax behavior to a singe link.
 */
Backdrop.views.ajaxView.prototype.attachPagerLinkAjax = function(id, link) {
  var $link = $(link);
  var viewData = {};
  var href = $link.attr('href');
  // Construct an object using the settings defaults and then overriding
  // with data specific to the link.
  $.extend(
    viewData,
    this.settings,
    Backdrop.Views.parseQueryString(href),
    // Extract argument data from the URL.
    Backdrop.Views.parseViewArgs(href, this.settings.view_base_path)
  );

  // For anchor tags, these will go to the target of the anchor rather
  // than the usual location.
  $.extend(viewData, Backdrop.Views.parseViewArgs(href, this.settings.view_base_path));

  this.element_settings.submit = viewData;
  this.pagerAjax = new Backdrop.ajax(false, link, this.element_settings);
  this.applyAjaxOverrides(this.pagerAjax);
};

/**
 * Apply Views-specific overrides to the lifecycle methods of an ajax instance.
 *
 * @param {Backdrop.ajax} ajaxInstance
 *   The ajax instance to configure with Views-specific behavior.
 */
Backdrop.views.ajaxView.prototype.applyAjaxOverrides = function (ajaxInstance) {
  ajaxInstance.beforeSerialize = function (element, options) {
    let enable_ajax_history = !!options.data.enable_ajax_history;
    if (enable_ajax_history && options.data.view_name) {
      // If restoring a previous state the dummy element will have this class,
      // don't need to go through all this processing.
      if ($(element).hasClass('ajax-history-trigger')) {
        return;
      }
    }

    // Check that we handle a click on a link, not a form submission.
    if (enable_ajax_history && options.data.view_name && element && $(element).is('a')) {
      // Strip the view base path so it isn't treated as a parameter.
      let params = new URLSearchParams($(element).attr('href').replace($(element).get(0).pathname + '?', ''));
      if (!$.isEmptyObject(options.data.exclude_ajax_args)) {
        var keysToRemove = [];
        $.each(options.data.exclude_ajax_args, function (index, pathToExclude) {
          params.forEach(function (value, key, parent) {
            if (key.startsWith(pathToExclude)) {
              keysToRemove.push(key);
            }
          });
        });
        keysToRemove.forEach(function (key) {
          params.delete(key);
        });
      }
      Backdrop.Views.addState(options, '?' + params.toString());
    }

    // Call the original Backdrop method with the right context.
    Backdrop.ajax.prototype.beforeSerialize.apply(this, arguments);
  };

  ajaxInstance.beforeSubmit = function (form_values, element, options) {
    if (!!options.data.enable_ajax_history && options && options.data && options.data.view_name) {
      var url = original.path + '?' + new URLSearchParams(new FormData(element.get(0))).toString();
      var currentQuery = Backdrop.Views.parseQueryString(window.location.href);

      // Prepare ajax url
      var ajaxUrl = options.url.split('?')[0];
      var ajaxQuery = Backdrop.Views.parseQueryString(options.url);

      // Remove the page number from the query string, as a new filter has been
      // applied and should return new results.
      if ($.inArray("page", Object.keys(currentQuery)) !== -1) {
        delete currentQuery.page;
        delete ajaxQuery.page;
      }

      // Copy selected values in history state.
      $.each(form_values, function () {
        // Field name ending with [] is a multi value field.
        if (/\[\]$/.test(this.name)) {
          if (!options.data[this.name]) {
            options.data[this.name] = [];
          }
          options.data[this.name].push(this.value);
        }
        // Regular field.
        else {
          options.data[this.name] = this.value;
        }
      });
      // Remove exposed data from the current query to leave behind any
      // non exposed form related query vars.
      element.find('[name]').each(function () {
        let name = this.name ?? this.getAttribute('name');
        if (currentQuery[name]) {
          delete currentQuery[name];
          delete ajaxQuery[name];
        }
      });

      // If the exposed form has checkboxes, we need to check if these are
      // unchecked and if so, remove them from the url
      element.find('input[type="checkbox"]').each(function (key, value) {
        if (!form_values[this.name]) {
          if (currentQuery[this.name]) {
            delete currentQuery[this.name];
          }
          else if (options.data[this.name]) {
            delete options.data[this.name];
          }
          if (ajaxQuery[this.name]) {
            delete ajaxQuery[this.name];
          }
        }
      });

      // Helper function to remove multi-value query keys that match a given
      // name
      let removeMultiValueQueryKeys = function (multiValueParamToRemove, queryParams) {
        Object.getOwnPropertyNames(queryParams).forEach(function (queryKey) {
          let queryKeyWithoutBracket = queryKey.replace(/\[\d+]$/, '');
          if (multiValueParamToRemove === queryKeyWithoutBracket) {
            delete queryParams[queryKey];
          }
        });
        return queryParams;
      };

      // If the exposed form has a multiple select.
      element.find('select[multiple]').each(function (key, value) {
        if ($(value).val().length === 0) {
          delete options.data[this.name];
          delete currentQuery[this.name];
          delete ajaxQuery[this.name];
        }
        // Pagers creates query params that are indexed like this:
        // ?some_param[0]=123,some_param[1]=456
        // instead of this:
        // ?some_param[]=123&some_param[]=456
        // We need to clear them out. The submitted form values will use the
        // non-indexed versions, and we can't have the indexed versions creating
        // a conflict.
        let nameWithoutBracket = this.name.replace(/\[]$/, '');
        currentQuery = removeMultiValueQueryKeys(nameWithoutBracket, currentQuery);
        ajaxQuery = removeMultiValueQueryKeys(nameWithoutBracket, ajaxQuery);
      });

      url += (/\?/.test(url) ? '&' : '?') + $.param(currentQuery);
      // Update options with updated ajax url.
      options.url = ajaxUrl + '?' + $.param(ajaxQuery);
      Backdrop.Views.addState(options, url);
    }

    // Call the original Backdrop method with the right context.
    Backdrop.ajax.prototype.beforeSubmit.apply(this, arguments);
  };

  ajaxInstance.beforeSend = function (jqXHR, options) {
    var data = (typeof options.data === 'string') ? Backdrop.Views.parseQueryString(options.data) : {};

    if (!!data.enable_ajax_history && data.view_name && options.type !== 'GET') {
      // Override the URL to not contain any fields that were submitted.
      options.url = Backdrop.settings.views.ajax_path;
    }
    // Call the original Backdrop method with the right context.
    Backdrop.ajax.prototype.beforeSend.apply(this, arguments);
  };
};

Backdrop.ajax.prototype.commands.viewsScrollTop = function (ajax, response) {
  // Scroll to the top of the view. This will allow users
  // to browse newly loaded content after e.g. clicking a pager
  // link.
  var offset = $(response.selector).offset();

  // We can't guarantee that the scrollable object should be
  // the body, as the view could be embedded in something
  // more complex such as a modal popup. Recurse up the DOM
  // and scroll the first element that has a non-zero top.
  var scrollTarget = response.selector;
  while ($(scrollTarget).scrollTop() === 0 && $(scrollTarget).parent()) {
    scrollTarget = $(scrollTarget).parent();
  }
  // Only scroll upward
  if (offset.top - 10 < $(scrollTarget).scrollTop()) {
    $(scrollTarget).animate({scrollTop: (offset.top - 10)}, 500);
  }
};

})(jQuery);
