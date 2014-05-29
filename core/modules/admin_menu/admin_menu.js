(function($) {

Drupal.adminMenu = Drupal.adminMenu || {};
Drupal.adminMenu.behaviors = Drupal.adminMenu.behaviors || {};
Drupal.adminMenu.hashes = Drupal.adminMenu.hashes || {};

/**
 * Core behavior for Administration menu.
 *
 * Test whether there is an administration menu is in the output and execute all
 * registered behaviors.
 */
Drupal.behaviors.adminMenu = {
  attach: function (context, settings) {
    // Initialize settings.
    settings.admin_menu = $.extend({
      suppress: false,
      margin_top: false,
      position_fixed: false,
      destination: '',
      basePath: settings.basePath,
      hash: 0,
      replacements: {}
    }, settings.admin_menu || {});
    // Check whether administration menu should be suppressed.
    if (settings.admin_menu.suppress) {
      return;
    }
    var $adminMenu = $('#admin-menu:not(.admin-menu-processed)', context);
    // Client-side caching; if administration menu is not in the output, it is
    // fetched from the server and cached in the browser.
    if (!$adminMenu.length && settings.admin_menu.hash) {
      Drupal.adminMenu.getCache(settings.admin_menu.hash, function (response) {
        if (typeof response == 'string' && response.length > 0) {
          $('body', context).append(response);
        }
        var $adminMenu = $('#admin-menu:not(.admin-menu-processed)', context);
        // Apply our behaviors.
        Drupal.adminMenu.attachBehaviors(context, settings, $adminMenu);
        // Allow resize event handlers to recalculate sizes/positions.
        $(window).triggerHandler('resize');
      });
    }
    // If the menu is in the output already, this means there is a new version.
    else {
      // Apply our behaviors.
      Drupal.adminMenu.attachBehaviors(context, settings, $adminMenu);
    }
  }
};

/**
 * Apply active trail highlighting based on current path.
 */
Drupal.adminMenu.behaviors.adminMenuActiveTrail = function (context, settings, $adminMenu) {
  if (settings.admin_menu.activeTrail) {
    $adminMenu.find('#admin-menu-menu > li > ul > li > a[href="' + settings.admin_menu.activeTrail + '"]').addClass('active-trail');
  }
};

/**
 * Apply margin to page.
 *
 * Note that directly applying marginTop does not work in IE. To prevent
 * flickering/jumping page content with client-side caching, this is a regular
 * Drupal behavior.
 */
Drupal.behaviors.adminMenuMarginTop = {
  attach: function (context, settings) {
    if (!settings.admin_menu.suppress && settings.admin_menu.margin_top) {
      $('body:not(.admin-menu)', context).addClass('admin-menu');
    }
  }
};

/**
 * Retrieve content from client-side cache.
 *
 * @param hash
 *   The md5 hash of the content to retrieve.
 * @param onSuccess
 *   A callback function invoked when the cache request was successful.
 */
Drupal.adminMenu.getCache = function (hash, onSuccess) {
  if (Drupal.adminMenu.hashes.hash !== undefined) {
    return Drupal.adminMenu.hashes.hash;
  }
  $.ajax({
    cache: true,
    type: 'GET',
    dataType: 'text', // Prevent auto-evaluation of response.
    global: false, // Do not trigger global AJAX events.
    url: Drupal.settings.admin_menu.basePath.replace(/admin_menu/, 'js/admin_menu/cache/' + hash),
    success: onSuccess,
    complete: function (XMLHttpRequest, status) {
      Drupal.adminMenu.hashes.hash = status;
    }
  });
};

/**
 * TableHeader callback to determine top viewport offset.
 */
Drupal.adminMenu.height = function() {
  var $adminMenu = $('#admin-menu');
  var height = $adminMenu.outerHeight();
  // In IE, Shadow filter adds some extra height, so we need to remove it from
  // the returned height.
  if ($adminMenu.css('filter') && $adminMenu.css('filter').match(/DXImageTransform\.Microsoft\.Shadow/)) {
    height -= $adminMenu.get(0).filters.item("DXImageTransform.Microsoft.Shadow").strength;
  }
  return height;
};

/**
 * @defgroup admin_behaviors Administration behaviors.
 * @{
 */

/**
 * Attach administrative behaviors.
 */
Drupal.adminMenu.attachBehaviors = function (context, settings, $adminMenu) {
  if ($adminMenu.length) {
    $adminMenu.addClass('admin-menu-processed');
    $.each(Drupal.adminMenu.behaviors, function() {
      this(context, settings, $adminMenu);
    });
  }
};

/**
 * Apply 'position: fixed'.
 */
Drupal.adminMenu.behaviors.positionFixed = function (context, settings, $adminMenu) {
  if (settings.admin_menu.position_fixed) {
    $adminMenu.addClass('admin-menu-position-fixed');
    $adminMenu.css('position', 'fixed');

    // Set a data attribute to inform other parts of the page that we're
    // offsetting the top margin, then trigger an offset change. See
    // tableheader.js for an example of how this is utilized.
    var height = $adminMenu.height();
    $adminMenu.attr('data-offset-top', height);
    $(document).triggerHandler('offsettopchange');
  }
};

/**
 * Perform dynamic replacements in cached menu.
 */
Drupal.adminMenu.behaviors.replacements = function (context, settings, $adminMenu) {
  for (var item in settings.admin_menu.replacements) {
    $(item, $adminMenu).html(settings.admin_menu.replacements[item]);
  }
};

/**
 * Inject destination query strings for current page.
 */
Drupal.adminMenu.behaviors.destination = function (context, settings, $adminMenu) {
  if (settings.admin_menu.destination) {
    $('a.admin-menu-destination', $adminMenu).each(function() {
      this.search += (!this.search.length ? '?' : '&') + Drupal.settings.admin_menu.destination;
    });
  }
};

/**
 * Adjust the top level items based on the available viewport width.
 */
Drupal.adminMenu.behaviors.collapseWidth = function (context, settings, $adminMenu) {
  var $menu = $adminMenu.find('#admin-menu-menu');
  var $extra = $adminMenu.find('#admin-menu-extra');
  var resizeTimeout;

  $(window).on('resize.adminMenu', function(event) {
    clearTimeout(resizeTimeout);
    resizeTimeout = setTimeout(function() {
      // Expand the menu items to their full width to check their size.
      $menu.removeClass('dropdown').addClass('top-level');
      $extra.removeClass('dropdown').addClass('top-level');

      $adminMenu.trigger('beforeResize');

      var menuWidth = $menu.width();
      var extraWidth = $extra.width();
      var availableWidth = $adminMenu.width() - $adminMenu.find('#admin-menu-icon').width();

      // Collapse the extra items first if needed.
      if (availableWidth - menuWidth - extraWidth < 20) {
        $extra.addClass('dropdown').removeClass('top-level');
        extraWidth = $extra.width();
      }
      // See if the menu also needs to be collapsed.
      if (availableWidth - menuWidth - extraWidth < 20) {
        $menu.addClass('dropdown').removeClass('top-level');
      }
      $adminMenu.trigger('afterResize');
    }, 50);
  }).triggerHandler('resize.adminMenu');
}

/**
 * Apply JavaScript-based hovering behaviors.
 *
 * @todo This has to run last.  If another script registers additional behaviors
 *   it will not run last.
 */
Drupal.adminMenu.behaviors.hover = function (context, settings, $adminMenu) {
  // Bind events for opening and closing menus on hover/click/touch.
  $adminMenu.on('mouseenter', 'li.expandable', expandChild);
  $adminMenu.on('mouseleave', 'li.expandable', closeChild);

  // On touch devices, the first click on an expandable link should not go to
  // that page, but a second click will. Use touch start/end events to target
  // these devices.
  var touchElement;
  var needsExpanding;
  $adminMenu.on('touchstart touchend click', 'li.expandable > a, li.expandable > span', function(e) {
    // The touchstart event fires before all other events, including mouseenter,
    // allowing us to check the expanded state consistently across devices.
    if (e.type === 'touchstart') {
      touchElement = e.target
      needsExpanding = $(this).siblings('ul').length > 0 && !$(this).siblings('ul').hasClass('expanded');
    }
    // If clicking on a not-yet-expanded item, expand it and suppress the click.
    if ((e.type === 'click' || e.type === 'touchend') && touchElement) {
      if (touchElement === e.target) {
        if (needsExpanding) {
          expandChild.apply($(this).parent()[0], [e]);
          e.preventDefault();
        }
        else if ($(this).is('span')) {
          closeChild.apply($(this).parent()[0], [e]);
        }
      }
      // If the touch ended on a different element than it started, suppress it.
      else if (touchElement !== e.target) {
        e.preventDefault();
      }
    }
  });

  // Close all menus if clicking outside the menu.
  $(document).bind('click', function (e) {
    if ($(e.target).closest($adminMenu).length === 0) {
      $adminMenu.find('ul').removeClass('expanded');
    }
  });

  function expandChild(e) {
    // Stop the timer.
    clearTimeout(this.sfTimer);

    // Display child lists.
    var $childList = $(this).children('ul');

    // Add classes for the expanded trail of links.
    $childList
      .parents('ul').addBack().addClass('expanded')
      .siblings('a, span').addClass('expanded-trail');
    // Immediately hide nephew lists.
    $childList.parent().siblings('li')
      .find('ul.expanded').removeClass('expanded').end()
      .find('.expanded-trail').removeClass('expanded-trail');
  }
  function closeChild(e) {
    // Start the timer.
    var $uls = $(this).find('> ul');
    var $link = $(this).find('> a, > span');
    this.sfTimer = setTimeout(function () {
      $uls.removeClass('expanded');
      $link.removeClass('expanded-trail');
    }, 400);
  }
};

/**
 * Apply the search bar functionality.
 */
Drupal.adminMenu.behaviors.search = function (context, settings, $adminMenu) {
  var $input = $adminMenu.find('.admin-menu-search input');
  // Initialize the current search needle.
  var needle = $input.val();
  // Cache of all links that can be matched in the menu.
  var links;
  // Minimum search needle length.
  var needleMinLength = 2;
  // Append the results container.
  var $results = $('<div class="admin-menu-search-results" />').insertAfter($input.parent());

  /**
   * Executes the search upon user input.
   */
  function keyupHandler(e) {
    var matches, $html, $hideItems, value = $(this).val();

    // Only proceed if the search needle has changed.
    if (value !== needle || e.type === 'focus') {
      needle = value;
      // Initialize the cache of menu links upon first search.
      if (!links && needle.length >= needleMinLength) {
        links = buildSearchIndex($adminMenu.find('#admin-menu-menu .dropdown li:not(.admin-menu-action, .admin-menu-action li) > a'));
      }

      // Close any open items.
      $adminMenu.find('li.highlight').trigger('mouseleave').removeClass('highlight');

      // Empty results container when deleting search text.
      if (needle.length < needleMinLength) {
        $results.empty();
      }
      // Only search if the needle is long enough.
      if (needle.length >= needleMinLength && links) {
        matches = findMatches(needle, links);
        // Build the list in a detached DOM node.
        $html = buildResultsList(matches);
        // Display results.
        $results.empty().append($html);
      }
      $adminMenu.trigger('searchChanged');
    }
  }

  /**
   * Builds the search index.
   */
  function buildSearchIndex($links) {
    return $links
      .map(function () {
        var text = (this.textContent || this.innerText);
        // Skip menu entries that do not contain any text (e.g., the icon).
        if (typeof text === 'undefined') {
          return;
        }
        return {
          text: text,
          textMatch: text.toLowerCase(),
          element: this
        };
      });
  }

  /**
   * Searches the index for a given needle and returns matching entries.
   */
  function findMatches(needle, links) {
    var needleMatch = needle.toLowerCase();
    // Select matching links from the cache.
    return $.grep(links, function (link) {
      return link.textMatch.indexOf(needleMatch) !== -1;
    });
  }

  /**
   * Builds the search result list in a detached DOM node.
   */
  function buildResultsList(matches) {
    var $html = $('<ul class="dropdown" />');
    $.each(matches, function () {
      var result = this.text;
      var $element = $(this.element);

      // Check whether there is a top-level category that can be prepended.
      var $category = $element.closest('#admin-menu-menu > li > ul > li');
      var categoryText = $category.find('> a').text()
      if ($category.length && categoryText) {
        result = categoryText + ': ' + result;
      }

      var $result = $('<li><a href="' + $element.attr('href') + '">' + result + '</a></li>');
      $result.data('original-link', $(this.element).parent());
      $html.append($result);
    });
    return $html;
  }

  /**
   * Highlights selected result.
   */
  function resultsHandler(e) {
    var $this = $(this);
    var show = e.type === 'mouseenter' || e.type === 'focusin' || e.type === 'touchstart';
    // Supress the normal click handling on first touch, only highlighting.
    if (e.type === 'touchstart' && !$(this).hasClass('active-search-item')) {
      e.preventDefault();
    }
    if (show) {
      $adminMenu.find('.active-search-item').removeClass('active-search-item');
      $(this).addClass('active-search-item');
    }
    else {
      $(this).removeClass('active-search-item');
    }
    $this.trigger(show ? 'showPath' : 'hidePath', [this]);
  }

  /**
   * Closes the search results and clears the search input.
   */
  function resultsClickHandler(e, link) {
    var $original = $(this).data('original-link');
    $original.trigger('mouseleave');
    $input.val('').trigger('keyup');
  }

  /**
   * Shows the link in the menu that corresponds to a search result.
   */
  function highlightPathHandler(e, link) {
    if (link) {
      $adminMenu.find('li.highlight').removeClass('highlight');
      var $original = $(link).data('original-link');
      var show = e.type === 'showPath';
      // Toggle an additional CSS class to visually highlight the matching link.
      $original.toggleClass('highlight', show);
      $original.trigger(show ? 'mouseenter' : 'mouseleave');
    }
  }

  function resetSearchDisplay(e) {
    $hideItems = $adminMenu.find('#admin-menu-extra > li > ul > li:not(li.admin-menu-search)').css('display', '');
  }
  function updateSearchDisplay(e) {
    // Build the list of extra items to be hidden if in small window mode.
    $hideItems = $adminMenu.find('#admin-menu-extra > li > ul > li:not(li.admin-menu-search)').css('display', '');
    if ($results.children().length) {
      if ($adminMenu.find('#admin-menu-extra').hasClass('dropdown')) {
        $hideItems.css('display', 'none');
      }
    }
  }

  // Attach showPath/hidePath handler to search result entries.
  $results.on('touchstart mouseenter focus blur', 'li', resultsHandler);
  // Hide the result list after a link has been clicked.
  $results.on('click', 'li', resultsClickHandler);
  // Attach hover/active highlight behavior to search result entries.
  $adminMenu.on('showPath hidePath', '.admin-menu-search-results li', highlightPathHandler);
  // Show/hide the extra parts of the menu on resize.
  $adminMenu.on('beforeResize', resetSearchDisplay)
  $adminMenu.on('afterResize searchChanged', updateSearchDisplay);
  // Attach the search input event handler.
  $input.bind('focus keyup search', keyupHandler);

  // Close search if clicking outside the menu.
  $(document).on('click', function (e) {
    if ($(e.target).closest($adminMenu).length === 0) {
      $results.empty();
    }
  });
};

/**
 * @} End of "defgroup admin_behaviors".
 */

})(jQuery);
