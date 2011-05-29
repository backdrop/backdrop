(function ($) {

/**
 * This script transforms a set of fieldsets into a stack of multipage pages. 
 * Another pane can be entered by clicking next/previous.
 *
 */
Drupal.behaviors.MultiPage = {
  attach: function (context) {
    $('.multipage-panes', context).once('multipage', function () {

      var focusID = $(':hidden.multipage-active-control', this).val();
      var paneWithFocus;

      // Check if there are some fieldsets that can be converted to multipages.
      var $panes = $('> fieldset.multipage', this);
      if ($panes.length == 0) {
        return;
      }

      // Create the next/previous controls.
      var $controls;

      // Transform each fieldset into a multipage with controls.
      $panes.each(function () {
        
        $controls = $('<div class="multipage-controls-list"></div>');
        $(this).append('<div class="multipage-controls clearfix"></div>').append($controls);
        
        var multipageControl = new Drupal.multipageControl({
          title: $('> legend', this).text(),
          fieldset: $(this),
          has_next: $(this).next().length,
          has_previous: $(this).prev().length
        });
        
        $controls.append(multipageControl.item);
        $(this)
          .removeClass('collapsible collapsed')
          .addClass('multipage-pane')
          .data('multipageControl', multipageControl);

        if (this.id == focusID) {
          paneWithFocus = $(this);
        }
      });
      
      // Check if the submit button needs to move to the latest pane.
      if (Drupal.settings.multipage_move_submit && $('.form-actions').length) {
        $('.form-actions').remove().appendTo($panes.last());
      }

      if (!paneWithFocus) {
        // If the current URL has a fragment and one of the tabs contains an
        // element that matches the URL fragment, activate that tab.
        if (window.location.hash && $(window.location.hash, this).length) {
          paneWithFocus = $(window.location.hash, this).closest('.multipage-pane');
        }
        else {
          paneWithFocus = $('multipage-open', this).length ? $('multipage-open', this) : $('> .multipage-pane:first', this);
        }
      }
      if (paneWithFocus.length) {
        paneWithFocus.data('multipageControl').focus();
      }
    });
  }
};

/**
 * The multipagePane object represents a single fieldset a page.
 *
 * @param settings
 *   An object with the following keys:
 *   - title: The name of the tab.
 *   - fieldset: The jQuery object of the fieldset that is the tab pane.
 */
Drupal.multipageControl = function (settings) {
  var self = this;
  $.extend(this, settings, Drupal.theme('multipage', settings));

  this.nextLink.click(function () {
    self.nextPage();
    return false;
  });
  
  this.previousLink.click(function () {
    self.previousPage();
    return false;
  });
  
/*
  // Keyboard events added:
  // Pressing the Enter key will open the tab pane.
  this.nextLink.keydown(function(event) {
    if (event.keyCode == 13) {
      self.focus();
      // Set focus on the first input field of the visible fieldset/tab pane.
      $("fieldset.horizontal-tabs-pane :input:visible:enabled:first").focus();
      return false;
    }
  });

  // Pressing the Enter key lets you leave the tab again.
  this.fieldset.keydown(function(event) {
    // Enter key should not trigger inside <textarea> to allow for multi-line entries.
    if (event.keyCode == 13 && event.target.nodeName != "TEXTAREA") {
      // Set focus on the selected tab button again.
      $(".horizontal-tab-button.selected a").focus();
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
    this.fieldset
      .show()
      .siblings('fieldset.multipage-pane')
        .each(function () {
          var tab = $(this).data('multipageControl');
          tab.fieldset.hide();
        })
        .end()
      .siblings(':hidden.multipage-active-control')
        .val(this.fieldset.attr('id'));
    // Mark the active control for screen readers.
    $('#active-multipage-control').remove();
    this.nextLink.append('<span id="active-multipage-control" class="element-invisible">' + Drupal.t('(active page)') + '</span>');
  },
  
  /**
   * Continues to the next page or step in the form.
   */
  nextPage: function () {
    this.fieldset.next().data('multipageControl').focus();
  },
  
  /**
   * Returns to the previous page or step in the form.
   */
  previousPage: function () {
    this.fieldset.prev().data('multipageControl').focus();
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
    // Display the fieldset.
    this.fieldset.removeClass('multipage-control-hidden').show();
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
    // Hide the fieldset.
    this.fieldset.addClass('horizontal-tab-hidden').hide();
    // Focus the first visible tab (if there is one).
    var $firstTab = this.fieldset.siblings('.multipage-pane:not(.multipage-control-hidden):first');
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
  controls.item.append(controls.nextLink = $('<input type="button" class="form-submit multipage-link-next" value="" />').val(controls.nextTitle = Drupal.t('Next')));
  controls.item.append(controls.previousLink = $('<a class="multipage-link-previous" href="#"></a>'));
  if (!settings.has_next) {
    controls.nextLink.hide();
  }
  if (settings.has_previous) {
    controls.previousLink.append(controls.previousTitle = $('<strong></strong>').text(Drupal.t('Previous')));
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
      // Add required fields mark to any element containing required fields
      $('fieldset.multipage-pane').each(function(i){
        if ($('.error', $(this)).length) {
          Drupal.FieldGroup.setGroupWithfocus($(this));
          $(this).data('multipageControl').focus();
        }
      });
    }
  }
}

})(jQuery);
