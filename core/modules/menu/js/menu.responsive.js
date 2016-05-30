/**
 * @file
 * Responsive Admin menu items.
 */

(function ($) {
"use strict";

Backdrop.makeMenuResponsive = function(context, settings, menuSelector, menuItemSelector, collapseGracefully) {
  if (context === undefined || settings === undefined || menuSelector === undefined || menuItemSelector === undefined) {
    throw 'Error: Backdrop.makeMenuResponsive does not have all of the arguments it needs to function.';
  }
  if (collapseGracefully === undefined) {
    collapseGracefully = true;
  }
  var $menu = $(context).find(menuSelector).once('responsive-menu');
  if ($menu.length === 0) {
    return;
  }

  var $menuWrapper = $menu.parent(),
      $menuItems = $menu.find(menuItemSelector),
      isMenuResponsive = false,
      menuResponsiveStrategy,
      previousWindowWidth,
      menuItemWidths = [],
      initialRunMenuItemWidths,
      initialRunMenuTextWidths,
      menuItemHeight,
      widestMenuWidth = 0,
      expandControlWidth,
      activeMenuNth = $menu.find('.active').first().index(),
      expandedMenusHeaderPadding = 0,
      defaultHeaderPadding = '20px', // @todo replace with a setting
      // @todo This functionality should go in callback
      $mobileHeaderPadder = $('<div class="responsive-menu-collapsible-strategy-header-padder" style="height: ' + expandedMenusHeaderPadding + 'px"></div>'),
      $body = $('body'),

      // These are essentially breakpoints to be measured against the menuItemArea.
      allMenusWidth,                // Will show all menu items.
      activeMenuItemAndBeforeWidth, // Will chop off menu items after active tab.
      activeMenuItemAndAfterWidth;  // Will chop off menu items before active tab.

  function initResponsiveMenu() {
    menuItemHeight = $menuItems.first().outerHeight();
    $menuWrapper.once('responsive-menu', function(){
      $menu.after(
        '<div class="responsive-menu-control" aria-hidden="true" style="height: ' + menuItemHeight + 'px">' +
          '<span class="responsive-menu-control-label"></span>' +
          '<span class="responsive-menu-control-compact"></span>' +
        '</div>'
      );
      $('.responsive-menu-control', $menuWrapper).click(function(){
        $menuWrapper.toggleClass('expand-dropdown-menu');
        $(this).toggleClass('js-active');
        // @todo Should go in callback
        // If there's not enough room for expanded menu items
        if (expandedMenusHeaderPadding > 0 && $menuWrapper.hasClass('expand-dropdown-menu')) {
          $mobileHeaderPadder.css('height', expandedMenusHeaderPadding + 'px');
          $body.prepend($mobileHeaderPadder);
          $body.scrollTop($body.scrollTop() + expandedMenusHeaderPadding);
        } else {
          $mobileHeaderPadder.remove();
          $body.scrollTop($body.scrollTop() - expandedMenusHeaderPadding);
        }
      });

      // Add control as first item.
      expandControlWidth = $menuWrapper.find('.responsive-menu-control-compact').outerWidth();
      // Wrap tab link text with wrapper so we can get tab width if font size is updated
      $menuItems.find('a').wrapInner('<span class="responsive-menu-link-text-wrapper"></span>');

      calculateMenuWidths();

      // Add classes to display menu items correctly for current screen width.
      adjustMenuDisplay();
    });
  }

  initResponsiveMenu();

  function calculateMenuWidths() {
    // Reset var
    menuItemWidths = [];

    var initialRun = false;
    if (initialRunMenuItemWidths === undefined || initialRunMenuTextWidths === undefined) {
      initialRun = true;
      initialRunMenuTextWidths = [];
    }

    // Calculate the our breakpoints using default menu item widths
    // Add expandControlWidth as a default item because we'll need room for it
    allMenusWidth = expandControlWidth;
    activeMenuItemAndBeforeWidth = expandControlWidth;
    activeMenuItemAndAfterWidth = expandControlWidth;

    $menuItems.each(function(i) {
      var $this = $(this),
          currentMenuItemWidth = $this.outerWidth(),
          currentMenuTextWidth = $this.find('.responsive-menu-link-text-wrapper').outerWidth();

      if (initialRun) {
        initialRunMenuTextWidths.push(currentMenuTextWidth);
      } else {
        // Can't count on the menu item's outerWidth, as that may change at different
        // responsive strategies from this behavior. Instead using initial size
        // (before this behavior applied) and any differences in font size
        // that may have occured
        currentMenuItemWidth = currentMenuTextWidth - initialRunMenuTextWidths[i] + initialRunMenuItemWidths[i];
      }
      menuItemWidths.push(currentMenuItemWidth);
      allMenusWidth += currentMenuItemWidth;
      if (i <= activeMenuNth) {
        activeMenuItemAndBeforeWidth += currentMenuItemWidth;
      }
      if (i >= activeMenuNth) {
        activeMenuItemAndAfterWidth += currentMenuItemWidth;
      }
      if (currentMenuItemWidth > widestMenuWidth) {
        widestMenuWidth = currentMenuItemWidth;
      }
    });

    // Keep track of what initial menu item widths for later behavior
    // breakpoint calculations
    if (initialRun) {
      initialRunMenuItemWidths = menuItemWidths;
    }

    if (activeMenuNth === 0) {
      // If it's the first item, we want to make sure we always see the first 2
      activeMenuItemAndBeforeWidth += menuItemWidths[1];
    } else if (activeMenuNth === $menuItems.length - 1) {
      // If it's the last item, we want to make sure we always see the last 2
      activeMenuItemAndAfterWidth += menuItemWidths[$menuItems.length - 2];
    }
  }

  function closeMenusDropdown() {
    $menuWrapper.removeClass('expand-dropdown-menu');
    $menuWrapper.find('.responsive-menu-control').removeClass('js-active');
    $mobileHeaderPadder.remove();
  }

  function handleResize() {
    var currentWindowWidth = $(window).width();

    // Only fire this if window width has changed.
    if (currentWindowWidth !== previousWindowWidth) {
      // Set previousWindowWidth for next event.
      previousWindowWidth = currentWindowWidth;

      // Shut menu items dropdown if it's open
      closeMenusDropdown();

      // Add classes to display menu items correctly for current screen width.
      adjustMenuDisplay();
    }
  }

  function adjustMenuDisplay() {
    // Reset var
    menuResponsiveStrategy = '';
    // Make sure that we've run initResponsiveMenu(),
    // and that there are menu items on this page.
    if (menuItemWidths.length > 0) {
      var menuItemArea = $menu.outerWidth();
      var accumulatedMenuWidth = expandControlWidth;

      if (menuItemArea >= allMenusWidth) {
        isMenuResponsive = false;
        $menuWrapper.addClass('responsive-menu-at-full')
          .removeClass('responsive-menu-before-strategy responsive-menu-after-strategy responsive-menu-collapsible-strategy');

        // Cleanup things that may have been left over from other
        // responsive tab strategies.
        $menu.find('.duplicated-menu-item').removeClass('duplicated-menu-item');
        $menuWrapper.find('.responsive-menu-dropdown').remove();
        $menu.css({'padding-left': '', 'top': '' });
      }
      else {
        isMenuResponsive = true;

        /**
         * Responsive tab strategies.
         * 'andBefore'  Show the active tab and the ones before it.
         * 'andAfter'   Show the active tab and the ones after it.
         * 'collapsible'     Put all menu items in a dropdown.
         */
        var $isMenuResponsiveDropdown = $('<ul class="primary responsive-menu-dropdown" aria-hidden="true" style="top: ' + menuItemHeight + 'px; width: ' + (widestMenuWidth + expandControlWidth + 20) + 'px"></ul>');
        if (collapseGracefully && menuItemArea >= activeMenuItemAndBeforeWidth) {
          /**
           * 'andBefore' Responsive Menu Strategy.
           */
          menuResponsiveStrategy = 'andBefore';

          var $lastVisibleMenu = null;
          // Manage classes on menu items.
          $menuItems.each(function(i) {
            accumulatedMenuWidth += menuItemWidths[i];
            if (menuResponsiveStrategy === 'andBefore') {
              if (i <= activeMenuNth || accumulatedMenuWidth <= menuItemArea) {
                $(this).removeClass('duplicated-menu-item');
                $lastVisibleMenu = $(this);
              }
              else {
                $isMenuResponsiveDropdown.append($(this).clone());
                $(this).addClass('duplicated-menu-item');
              }
            }
          });

          // Manage classes on wrapper.
          $menuWrapper.addClass('responsive-menu-before-strategy')
            .removeClass('responsive-menu-at-full responsive-menu-after-strategy responsive-menu-collapsible-strategy');

          // Apply expand control's position.
          var expandControlLeft = $lastVisibleMenu.position().left + $lastVisibleMenu.outerWidth();
          $('.responsive-menu-control', $menuWrapper).css('left', expandControlLeft);
          $isMenuResponsiveDropdown.css('right', menuItemArea - expandControlLeft - expandControlWidth);

          // Cleanup things that may have been left over from other
          // responsive tab strategies.
          $menu.css({'padding-left': '', 'top': '' });
          expandedMenusHeaderPadding = 0;
        }
        else if (collapseGracefully && menuItemArea >= activeMenuItemAndAfterWidth) {
          /**
           * 'andAfter' Responsive Menu Strategy
           */
          menuResponsiveStrategy = 'andAfter';
          accumulatedMenuWidth = expandControlWidth;

          // In order to get this dropdown to lay out correctly
          // making new element that comes after the shown menu items.
          // Iterate through menu items in reverse and give appropriate classes.
          $($menuItems.get().reverse()).each(function(reverseI) {
            var i = $menuItems.length - 1 - reverseI;
            accumulatedMenuWidth += menuItemWidths[i];
            if (i >= activeMenuNth || accumulatedMenuWidth <= menuItemArea) {
              $(this).removeClass('duplicated-menu-item');
            }
            else {
              $isMenuResponsiveDropdown.prepend($(this).clone());
              $(this).addClass('duplicated-menu-item');
            }
          });

          // Dropdown control gets left aligned.
          $('.responsive-menu-control', $menuWrapper).css('left', 0);
          $menu.css({'padding-left': expandControlWidth, 'top': '' });

          // Manage classes on wrapper.
          $menuWrapper.addClass('responsive-menu-after-strategy')
            .removeClass('responsive-menu-at-full responsive-menu-before-strategy responsive-menu-collapsible-strategy');

          // Cleanup things that may have been left over from other
          // responsive tab strategies.
          expandedMenusHeaderPadding = 0;
        }
        else {
          /**
           * 'collapsible' Responsive Menu Strategy.
           */
          menuResponsiveStrategy = 'collapsible';

          // Manage classes on menu items and wrappers
          $menu.find('.duplicated-menu-item').removeClass('duplicated-menu-item');
          $menuWrapper.addClass('responsive-menu-collapsible-strategy')
            .removeClass('responsive-menu-before-strategy responsive-menu-after-strategy responsive-menu-at-full');

          // Figure out how to lay primary menu items behind the expand control.
          var menuItemsOffset = activeMenuNth * (menuItemHeight + 2);
          var menuItemsTopDistance = $menuWrapper.position().top;
          if (menuItemsOffset > menuItemsTopDistance) {
            expandedMenusHeaderPadding = menuItemsOffset - menuItemsTopDistance + parseInt(defaultHeaderPadding, 10);
          }

          // Get the active tab's text.
          var $activeMenuText = $('<span class="responsive-menu-control-label">' + $menu.find('li.active a').html() + '</span>');
          $activeMenuText.find('.element-invisible').remove();
          $menuWrapper.find('.responsive-menu-control-label').replaceWith($activeMenuText);
          $menuWrapper.find('.responsive-menu-control').css('left', 'auto');

          // Cleanup things that may have been left over from other
          // responsive tab strategies.
          $menuWrapper.find('.responsive-menu-dropdown').remove();
          $menu.css({'padding-left': 0, 'top': '-' + menuItemsOffset + 'px' });
        }

        // Insert $isMenuResponsiveDropdown to markup if it's not empty.
        if ($isMenuResponsiveDropdown.find('li').length > 0) {
          $isMenuResponsiveDropdown.find('.duplicated-menu-item').removeClass('duplicated-menu-item');
          if ($menuWrapper.find('.responsive-menu-dropdown').length > 0) {
            $menuWrapper.find('.responsive-menu-dropdown').replaceWith($isMenuResponsiveDropdown);
          }
          else {
            $menu.after($isMenuResponsiveDropdown);
          }
        }
      }
    }

  }

  // If they click outside of the responsive menu items, shut them
  $('html').click(function(e){
    var $target = $(e.target);
    if (isMenuResponsive && !$target.is('.responsive-menu-processed') && $target.parents('.responsive-menu-processed').length < 1) {
      closeMenusDropdown();
    }
  });

  // Resource friendly resize event
  Backdrop.optimizedResize.add(handleResize);

  /**
   * Check to see when webfont has loaded and adjust the menu items display
   */
  var checkFontCounter = 0;
  // Append an invisible element that will be monospace font or our desired
  // font. We're using a repeating i because the characters width will
  // drastically change when it's monospace vs. proportional font.
  var $checkFontElement = $('<span id="check-font" style="visibility: hidden;">iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii</span>');
  $checkFontElement.appendTo($body).wrap('<span id="check-font-wrapper"></span>');

  // Function to check the width of the font, if it's substantially different
  // we'll know we our real font has loaded
  function checkFont() {
    var currentWidth = $checkFontElement.width();
    if (currentWidth < 200 || checkFontCounter >= 60) {
      // If our font has loaded, or it's been 6 seconds
      adjustMenuDisplay();
      // Clean up after ourselves
      clearInterval(checkFontInterval);
      $checkFontElement.remove();
      calculateMenuWidths();
    }
    checkFontCounter++;
  }
  var checkFontInterval = setInterval(checkFont, 100);

}

})(jQuery);
