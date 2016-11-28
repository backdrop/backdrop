(function ($) {

/**
 * This script transforms a set of wrappers into a stack of multipage pages.
 * Another pane can be entered by clicking next/previous.
 *
 */
Drupal.behaviors.MultiPage = {
  attach: function (context) {
    $('.multipage-panes', context).once('multipage', function () {

      var focusID = $(':hidden.multipage-active-control', this).val();
      var paneWithFocus;

      // Check if there are some wrappers that can be converted to multipages.
      var $panes = $('> div.field-group-multipage', this);
      var $form = $panes.parents('form');
      if ($panes.length == 0) {
        return;
      }

      // Create the next/previous controls.
      var $controls;

      // Transform each div.multipage-pane into a multipage with controls.
      $panes.each(function () {

        $controls = $('<div class="multipage-controls-list clearfix"></div>');
        $(this).append($controls);

        // Check if the submit button needs to move to the latest pane.
        if (Drupal.settings.field_group.multipage_move_submit && $('.form-actions').length) {
          $('.form-actions', $form).remove().appendTo($($controls, $panes.last()));
        }

        var multipageControl = new Drupal.multipageControl({
          title: $('> .multipage-pane-title', this).text(),
          wrapper: $(this),
          has_next: $(this).next().length,
          has_previous: $(this).prev().length
        });

        $controls.append(multipageControl.item);
        $(this)
          .addClass('multipage-pane')
          .data('multipageControl', multipageControl);

        if (this.id == focusID) {
          paneWithFocus = $(this);
        }

      });

      if (paneWithFocus === undefined) {
        // If the current URL has a fragment and one of the tabs contains an
        // element that matches the URL fragment, activate that tab.
        var hash = window.location.hash.replace(/[=%;,\/]/g, "");
        if (hash !== '#' && $(hash, this).length) {
          paneWithFocus = $(window.location.hash, this).closest('.multipage-pane');
        }
        else {
          paneWithFocus = $('multipage-open', this).length ? $('multipage-open', this) : $('> .multipage-pane:first', this);
        }
      }
      if (paneWithFocus !== undefined) {
        paneWithFocus.data('multipageControl').focus();
      }
    });
  }
};

/**
 * The multipagePane object represents a single div as a page.
 *
 * @param settings
 *   An object with the following keys:
 *   - title: The name of the tab.
 *   - wrapper: The jQuery object of the <div> that is the tab pane.
 */
Drupal.multipageControl = function (settings) {
  var self = this;
  var controls = Drupal.theme('multipage', settings);
  $.extend(self, settings, controls);

  this.nextLink.click(function (e) {
    e.preventDefault();
    self.nextPage();
  });

  this.previousLink.click(function (e) {
    e.preventDefault();
    self.previousPage();
  });

/*
  // Keyboard events added:
  // Pressing the Enter key will open the tab pane.
  this.nextLink.keydown(function(event) {
    if (event.keyCode == 13) {
      self.focus();
      // Set focus on the first input field of the visible wrapper/tab pane.
      $("div.multipage-pane :input:visible:enabled:first").focus();
      return false;
    }
  });

  // Pressing the Enter key lets you leave the tab again.
  this.wrapper.keydown(function(event) {
    // Enter key should not trigger inside <textarea> to allow for multi-line entries.
    if (event.keyCode == 13 && event.target.nodeName != "TEXTAREA") {
      // Set focus on the selected tab button again.
      $(".multipage-tab-button.selected a").focus();
      return false;
    }
  });
*/
};

Drupal.multipageControl.prototype = {

  /**
   * Displays the tab's content pane.
   */
  focus: function () {
    this.wrapper
      .show()
      .siblings('div.multipage-pane')
        .each(function () {
          var tab = $(this).data('multipageControl');
          tab.wrapper.hide();
        })
        .end()
      .siblings(':hidden.multipage-active-control')
        .val(this.wrapper.attr('id'));
    // Mark the active control for screen readers.
    $('#active-multipage-control').remove();
    this.nextLink.after('<span id="active-multipage-control" class="element-invisible">' + Drupal.t('(active page)') + '</span>');
  },

  /**
   * Continues to the next page or step in the form.
   */
  nextPage: function () {
    this.wrapper.next().data('multipageControl').focus();
    $('html, body').scrollTop(this.wrapper.parents('.field-group-multipage-group-wrapper').offset().top);
  },

  /**
   * Returns to the previous page or step in the form.
   */
  previousPage: function () {
    this.wrapper.prev().data('multipageControl').focus();
    $('html, body').scrollTop(this.wrapper.parents('.field-group-multipage-group-wrapper').offset().top);
  },

  /**
   * Shows a horizontal tab pane.
   */
  tabShow: function () {
    // Display the tab.
    this.item.show();
    // Update .first marker for items. We need recurse from parent to retain the
    // actual DOM element order as jQuery implements sortOrder, but not as public
    // method.
    this.item.parent().children('.multipage-control').removeClass('first')
      .filter(':visible:first').addClass('first');
    // Display the wrapper.
    this.wrapper.removeClass('multipage-control-hidden').show();
    // Focus this tab.
    this.focus();
    return this;
  },

  /**
   * Hides a horizontal tab pane.
   */
  tabHide: function () {
    // Hide this tab.
    this.item.hide();
    // Update .first marker for items. We need recurse from parent to retain the
    // actual DOM element order as jQuery implements sortOrder, but not as public
    // method.
    this.item.parent().children('.multipage-control').removeClass('first')
      .filter(':visible:first').addClass('first');
    // Hide the wrapper.
    this.wrapper.addClass('horizontal-tab-hidden').hide();
    // Focus the first visible tab (if there is one).
    var $firstTab = this.wrapper.siblings('.multipage-pane:not(.multipage-control-hidden):first');
    if ($firstTab.length) {
      $firstTab.data('multipageControl').focus();
    }
    return this;
  }
};

/**
 * Theme function for a multipage control.
 *
 * @param settings
 *   An object with the following keys:
 *   - title: The name of the tab.
 * @return
 *   This function has to return an object with at least these keys:
 *   - item: The root tab jQuery element
 *   - nextLink: The anchor tag that acts as the clickable area of the control
 *   - nextTitle: The jQuery element that contains the group title
 *   - previousLink: The anchor tag that acts as the clickable area of the control
 *   - previousTitle: The jQuery element that contains the group title
 */
Drupal.theme.prototype.multipage = function (settings) {

  var controls = {};
  controls.item = $('<span class="multipage-button"></span>');

  controls.previousLink = $('<input type="button" class="form-submit multipage-link-previous" value="" />');
  controls.previousTitle = Drupal.t('Previous page');
  controls.item.append(controls.previousLink.val(controls.previousTitle));

  controls.nextLink = $('<input type="button" class="form-submit multipage-link-next" value="" />');
  controls.nextTitle = Drupal.t('Next page');
  controls.item.append(controls.nextLink.val(controls.nextTitle));

  if (!settings.has_next) {
    controls.nextLink.hide();
  }
  if (!settings.has_previous) {
    controls.previousLink.hide();
  }

  return controls;
};


Drupal.FieldGroup = Drupal.FieldGroup || {};
Drupal.FieldGroup.Effects = Drupal.FieldGroup.Effects || {};

/**
 * Implements Drupal.FieldGroup.processHook().
 */
Drupal.FieldGroup.Effects.processMultipage = {
  execute: function (context, settings, type) {
    if (type == 'form') {

      var $firstErrorItem = false;

      // Add required fields mark to any element containing required fields
      $('div.multipage-pane').each(function(i){
        if ($('.error', $(this)).length) {

          // Save first error item, for focussing it.
          if (!$firstErrorItem) {
            $firstErrorItem = $(this).data('multipageControl');
          }

          Drupal.FieldGroup.setGroupWithfocus($(this));
          $(this).data('multipageControl').focus();
        }
      });

      // Focus on first multipage that has an error.
      if ($firstErrorItem) {
        $firstErrorItem.focus();
      }

    }
  }
}

})(jQuery);
