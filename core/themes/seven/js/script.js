/**
 * @file
 * Responsive Admin tabs.
 */

(function ($) {
"use strict";

Backdrop.behaviors.responsivePrimaryTabs = {
  attach: function(context, settings) {
    var $primaryTabs = $('ul.tabs.primary');
    var $tabsWrapper = $primaryTabs.parent();
    var $tabs = $('li', $primaryTabs);
    var responsiveTabs = false;
    var previousWindowWidth;
    var tabWidths = [];
    var tabHeight;
    var widestTabWidth = 0;
    var expandControlWidth;
    var activeTabNth = $('li.active', $primaryTabs).index();
    var expandedTabsHeaderPadding = 0;
    var defaultHeaderPadding = '20px';
    var $mobileHeaderPadder = $('<div class="responsive-tabs-mobile-header-padder" style="height: ' + expandedTabsHeaderPadding + 'px"></div>');

    // These are essentially breakpoints to be measured against the tabArea.
    var allTabsWidth;             // Will show all tabs.
    var activeTabAndBeforeWidth;  // Will chop off tabs after active tab.
    var activeTabAndAfterWidth;   // Will chop off tabs before active tab.

    function initResponsivePrimaryTabs() {
      tabHeight = $('li:first-child', $primaryTabs).outerHeight();
      $tabsWrapper.once('responsive-tabs', function(){
        $primaryTabs.after(
          '<div class="expand-dropdown-tabs-control" aria-hidden="true" style="height: ' + tabHeight + 'px">' +
            '<span class="expand-dropdown-tabs-label"></span>' +
          '</div>'
        );
        $('.expand-dropdown-tabs-control', $tabsWrapper).click(function(){
          $tabsWrapper.toggleClass('expand-dropdown-tabs');
          $(this).toggleClass('js-active');
          // If there's not enough room for mobile tabs.
          if (expandedTabsHeaderPadding > 0 && $tabsWrapper.hasClass('expand-dropdown-tabs')) {
            $mobileHeaderPadder.css('height', expandedTabsHeaderPadding + 'px');
            $('body').prepend($mobileHeaderPadder);
            $('body').scrollTop($('body').scrollTop() + expandedTabsHeaderPadding);
          } else {
            $mobileHeaderPadder.remove();
            $('body').scrollTop($('body').scrollTop() - expandedTabsHeaderPadding);
          }
        });

        // Add control as first item.
        expandControlWidth = $('.expand-dropdown-tabs-control', $tabsWrapper).outerWidth();

        // Wrap tab link text with wrapper so we can get tab width if font size is updated
        $('a', $tabs).wrapInner('<span class="responsive-tabs-link-text-wrapper"></span>')

        calculateTabWidths();

        // Add classes to display tabs correctly for current screen width.
        adjustTabsDisplay();
      });
    }

    initResponsivePrimaryTabs();

    function calculateTabWidths() {
      // Reset var
      tabWidths = [];

      // Calculate the tab widths before we do anything that will change them.
      // Add expandControlWidth as first tab.
      allTabsWidth = expandControlWidth;
      activeTabAndBeforeWidth = expandControlWidth;
      activeTabAndAfterWidth = expandControlWidth;
      // Add each tab width.
      $tabs.each(function(i) {
        // Tab width is text width + 20px padding on both sides + 2px border-right
        var currentTabWidth = $('.responsive-tabs-link-text-wrapper', this).outerWidth() + 42;
        tabWidths.push(currentTabWidth);
        allTabsWidth += currentTabWidth;
        if (i <= activeTabNth) {
          activeTabAndBeforeWidth += currentTabWidth;
        }
        if (i >= activeTabNth) {
          activeTabAndAfterWidth += currentTabWidth;
        }
        if (currentTabWidth > widestTabWidth) {
          widestTabWidth = currentTabWidth;
        }
      });

      if (activeTabNth === 0) {
        activeTabAndBeforeWidth += tabWidths[1];
      } else if (activeTabNth === $tabs.length - 1) {
        activeTabAndAfterWidth += tabWidths[$tabs.length - 2];
      }

    }

    function closeTabsDropdown() {
      $tabsWrapper.removeClass('expand-dropdown-tabs');
      $('.expand-dropdown-tabs-control', $tabsWrapper).removeClass('js-active');
      $mobileHeaderPadder.remove();
    }

    function handleResize() {
      var currentWindowWidth = $(window).width();

      // Only fire this if window width has changed.
      if (currentWindowWidth !== previousWindowWidth) {
        // Set previousWindowWidth for next event.
        previousWindowWidth = currentWindowWidth;

        // Shut tabs dropdown if it's open
        closeTabsDropdown();

        // Add classes to display tabs correctly for current screen width.
        adjustTabsDisplay();

      }
    }

    function adjustTabsDisplay() {
      var responsiveTabsType;
      // Make sure that we've run initResponsivePrimaryTabs(),
      // and that there are tabs on this page.
      if (tabWidths.length > 0) {
        var firstItemPosition;
        var previousItemPosition = null;
        var tabArea = $primaryTabs.outerWidth();
        var accumulatedTabWidth = expandControlWidth;

        if (tabArea >= allTabsWidth) {
          responsiveTabs = false;
          $tabsWrapper.addClass('desktop-primary-tabs');

          // Cleanup things that may have been left over from other
          // responsive tab strategies.
          $('.duplicated-tab', $primaryTabs).removeClass('duplicated-tab');
          $('.responsive-tabs-dropdown', $tabsWrapper).remove();
          $primaryTabs.css('padding-left', 0);
        }
        else {
          responsiveTabs = true;

          /**
           * Responsive tab strategies.
           * 'andBefore'  Show the active tab and the ones before it.
           * 'andAfter'   Show the active tab and the ones after it.
           * 'mobile'     Put all tabs in a dropdown.
           */
          var $responsiveTabsDropdown = $('<ul class="primary responsive-tabs-dropdown" aria-hidden="true" style="top: ' + tabHeight + 'px; width: ' + (widestTabWidth + expandControlWidth + 20) + 'px"></ul>');
          if (tabArea >= activeTabAndBeforeWidth) {
            /**
             * 'andBefore' Responsive Tab Strategy.
             */
            responsiveTabsType = 'andBefore';

            var $lastVisibleTab = null;
            // Manage classes on tabs.
            $tabs.each(function(i) {
              accumulatedTabWidth += tabWidths[i];
              if (responsiveTabsType === 'andBefore') {
                if (i <= activeTabNth || accumulatedTabWidth <= tabArea) {
                  $(this).removeClass('duplicated-tab');
                  $lastVisibleTab = $(this);
                }
                else {
                  $responsiveTabsDropdown.append($(this).clone());
                  $(this).addClass('duplicated-tab');
                }
              }
            });

            // Manage classes on wrapper.
            $tabsWrapper.addClass('responsive-tabs-before')
              .removeClass('desktop-primary-tabs responsive-tabs-after responsive-tabs-mobile');

            // Apply expand control's position.
            var expandControlLeft = $lastVisibleTab.position().left + $lastVisibleTab.outerWidth();
            $('.expand-dropdown-tabs-control', $tabsWrapper).css('left', expandControlLeft);
            $responsiveTabsDropdown.css('right', tabArea - expandControlLeft - expandControlWidth);

            // Cleanup things that may have been left over from other
            // responsive tab strategies.
            $primaryTabs.css('padding-left', 0);
            expandedTabsHeaderPadding = 0;
          }
          else if (tabArea >= activeTabAndAfterWidth) {
            /**
             * 'andAfter' Responsive Tab Strategy
             */
            responsiveTabsType = 'andAfter';
            accumulatedTabWidth = expandControlWidth;

            // In order to get this dropdown to lay out correctly
            // making new element that comes after the shown tabs.
            var tabsForDropdown = [];
            // Iterate through tabs in reverse and give appropriate classes.
            $($tabs.get().reverse()).each(function(reverseI) {
              var i = $tabs.length - 1 - reverseI;
              accumulatedTabWidth += tabWidths[i];
              if (i >= activeTabNth || accumulatedTabWidth <= tabArea) {
                $(this).removeClass('duplicated-tab');
              }
              else {
                $responsiveTabsDropdown.prepend($(this).clone());
                $(this).addClass('duplicated-tab');
              }
            });

            // Dropdown control gets left aligned.
            $('.expand-dropdown-tabs-control', $tabsWrapper).css('left', 0);
            $primaryTabs.css('padding-left', expandControlWidth);

            // Manage classes on wrapper.
            $tabsWrapper.addClass('responsive-tabs-after').removeClass('desktop-primary-tabs responsive-tabs-before responsive-tabs-mobile');

            // Cleanup things that may have been left over from other
            // responsive tab strategies.
            expandedTabsHeaderPadding = 0;
          }
          else {
            /**
             * 'mobile' Responsive Tab Strategy.
             */
            responsiveTabsType = 'mobile';

            // Manage classes on tabs and wrappers
            $('.duplicated-tab', $primaryTabs).removeClass('duplicated-tab');
            $tabsWrapper.addClass('responsive-tabs-mobile').removeClass('responsive-tabs-before responsive-tabs-after desktop-primary-tabs');

            // Figure out how to lay primary tabs behind the expand control.
            var tabsOffset = activeTabNth * (tabHeight + 2);
            var tabsTopDistance = $tabsWrapper.position().top;
            $primaryTabs.css('top', '-' + tabsOffset + 'px');
            if (tabsOffset > tabsTopDistance) {
              expandedTabsHeaderPadding = tabsOffset - tabsTopDistance + parseInt(defaultHeaderPadding, 10);
            }

            // Get the active tab's text.
            var $activeTabText = $('<span class="expand-dropdown-tabs-label">' + $('li.active a', $primaryTabs).html() + '</span>');
            $('.element-invisible', $activeTabText).remove();
            $('.expand-dropdown-tabs-label', $tabsWrapper).replaceWith($activeTabText);
            $('.expand-dropdown-tabs-control', $tabsWrapper).css('left', 'auto');

            // Cleanup things that may have been left over from other
            // responsive tab strategies.
            $('.responsive-tabs-dropdown', $tabsWrapper).remove();
            $primaryTabs.css('padding-left', 0);
          }

          // Insert $responsiveTabsDropdown to markup if it's not empty.
          if ($('li', $responsiveTabsDropdown).length > 0) {
            $('.duplicated-tab', $responsiveTabsDropdown).removeClass('duplicated-tab');
            if ($('.responsive-tabs-dropdown').length > 0) {
              $('.responsive-tabs-dropdown', $tabsWrapper).replaceWith($responsiveTabsDropdown);
            }
            else {
              $primaryTabs.after($responsiveTabsDropdown);
            }
          }
        }
      }
    }

    // If they click outside of the responsive tabs, shut them
    $('html').click(function(e){
      var $target = $(e.target);
      if (responsiveTabs && !$target.is('.responsive-tabs-processed') && $target.parents('.responsive-tabs-processed').length < 1) {
        closeTabsDropdown();
      }
    });

    /**
     * Check to see when webfont has loaded and adjust the tabs display
     */
    var checkFontCounter = 0;
    // Append an invisible element that will be monospace font or our desired
    // font. We're using a repeating i because the characters width will
    // drastically change when it's monospace vs. proportional font.
    var $checkFontElement = $('<span id="check-font-wrapper"><span id="check-font">iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii</span></span>');
    $('body').append($checkFontElement);

    // Function to check the width of the font, if it's substantially different
    // we'll know we our real font has loaded
    function checkFont() {
      var currentWidth = $checkFontElement.width();
      if (currentWidth < 200 || checkFontCounter >= 60) {
        // If our font has loaded, or it's been 6 seconds
        adjustTabsDisplay();
        // Clean up after ourselves
        clearInterval(checkFontInterval);
        $checkFontElement.remove();
        calculateTabWidths();
      }
      checkFontCounter++;
    }
    var checkFontInterval = setInterval(checkFont, 100);

    // Resource friendly resize event
    var resizeTimeout;
    $(window).on('resize', function (event) {
      clearTimeout(resizeTimeout);
      resizeTimeout = setTimeout(handleResize, 50);
    });
  }

}

})(jQuery);
