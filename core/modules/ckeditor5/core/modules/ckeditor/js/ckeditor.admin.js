(function ($, Backdrop) {

"use strict";

Backdrop.ckeditor = Backdrop.ckeditor || {};

Backdrop.behaviors.ckeditorAdmin = {
  attach: function (context, settings) {
    var $context = $(context);
    $context.find('.ckeditor-toolbar-configuration').once('ckeditor-toolbar', function() {
      var $wrapper = $(this);
      var $textareaWrapper = $(this).find('.form-item-editor-settings-toolbar').hide();
      var $textarea = $textareaWrapper.find('textarea');
      var $toolbarAdmin = $(settings.ckeditor.toolbarAdmin);
      var sortableSettings = {
        connectWith: '.ckeditor-buttons',
        placeholder: 'ckeditor-button-placeholder',
        forcePlaceholderSize: true,
        tolerance: 'pointer',
        cursor: 'move',
        stop: adminToolbarStopDrag
      };
      var groupSortableSettings = {
        connectWith: '.ckeditor-toolbar-groups',
        cursor: 'move',
        tolerance: 'touch',
        stop: adminToolbarStopDrag
      };
      $toolbarAdmin.insertAfter($textareaWrapper);

      // Add draggable/sortable behaviors.
      $toolbarAdmin.find('.ckeditor-buttons').sortable(sortableSettings);
      $toolbarAdmin.find('.ckeditor-toolbar-groups').sortable(groupSortableSettings);
      $toolbarAdmin.find('.ckeditor-multiple-buttons li').draggable({
        connectToSortable: '.ckeditor-toolbar-active .ckeditor-buttons',
        helper: 'clone'
      });

      // Add the show/hide groups link.
      var $activeToolbar = $toolbarAdmin.parent().find('.ckeditor-toolbar-active');
      var $toolbarGroupToggle = $(Backdrop.theme('ckeditorButtonGroupNamesToggle'));
      $toolbarGroupToggle.shown = true;
      $toolbarGroupToggle.text(Backdrop.t('Show group labels'));
      $toolbarGroupToggle.insertBefore($activeToolbar);
      $toolbarGroupToggle.bind('click.ckeditorToogleGroups', function(event) {
        adminToolbarToggleGroups.apply(event.target, [$toolbarGroupToggle]);
      });

      // Disable clicking on the individual buttons.
      $toolbarAdmin.find('.ckeditor-button a').click(function(event) {
        return false;
      });

      // Add the handler for modifying group names.
      $toolbarAdmin.bind('click.ckeditorRenameGroup', function(event) {
        if ($(event.target).is('.ckeditor-toolbar-group-name')) {
          adminToolbarRenameGroup.apply(event.target, [event]);
        }
      });

      // Add the handler for adding a new group.
      $toolbarAdmin.bind('click.ckeditorAddGroup', function(event) {
        if ($(event.target).is('a.ckeditor-group-add')) {
          adminToolbarAddGroup.apply(event.target, [event]);
        }
      });

      // Add the handler for adding/removing row buttons.
      $toolbarAdmin.bind('click.ckeditorAddRow', function(event) {
        if ($(event.target).is('a.ckeditor-row-add')) {
          adminToolbarAddRow.apply(event.target, [event]);
        }
      });
      $toolbarAdmin.bind('click.ckeditorAddRow', function(event) {
        if ($(event.target).is('a.ckeditor-row-remove')) {
          adminToolbarRemoveRow.apply(event.target, [event]);
        }
      });
      $toolbarAdmin.find('a.ckeditor-row-remove:first').hide();

      /**
       * Show/hide the toolbar group labels.
       */
      function adminToolbarToggleGroups($toolbarGroupToggle) {
        if ($toolbarGroupToggle.shown) {
          $toolbarGroupToggle.shown = false;
          $toolbarGroupToggle.text(Backdrop.t('Hide group labels'));
          $activeToolbar.addClass('ckeditor-group-names-are-visible');
        }
        else {
          $toolbarGroupToggle.shown = true;
          $toolbarGroupToggle.text(Backdrop.t('Show group labels'));
          $activeToolbar.removeClass('ckeditor-group-names-are-visible');
        }
      }

      /**
       * Rename a group.
       */
      function adminToolbarRenameGroup(event) {
        var $label = $(this);
        var currentText = $label.text();
        var newText = window.prompt(Backdrop.t('Enter a label for this group. This will be used by screenreaders and other accessibility software.'), currentText);
        if (newText) {
          $label.text(newText);
          $label.parent().data('group-name', newText);
        }
      };

      /**
       * Add a new group of buttons to the current row.
       */
      function adminToolbarAddGroup(event) {
        var $groups = $(this).closest('.ckeditor-row').find('.ckeditor-toolbar-groups');
        var $group;
        var newText = window.prompt(Backdrop.t('Enter a label for this group. This will be used by screenreaders and other accessibility software.'));
        if (newText) {
          $group = $(Backdrop.theme('ckeditorButtonGroup'));
          $group.find('.ckeditor-toolbar-group-name').text(newText);
          $group.data('group-name', newText);
          $groups.append($group);
          $groups.find('.ckeditor-buttons').sortable(sortableSettings);
          $groups.sortable(groupSortableSettings);
        }
        event.preventDefault();
      }

      /**
       * Add a new row of buttons.
       */
      function adminToolbarAddRow(event) {
        var $rows = $(this).closest('.ckeditor-toolbar-active').find('.ckeditor-row');
        var $newRow = $rows.last().clone();
        $newRow.find('.ckeditor-toolbar-groups').empty();
        $newRow.insertAfter($rows.last());
        $newRow.find('.ckeditor-buttons').sortable(sortableSettings);
        $newRow.find('.ckeditor-toolbar-groups').sortable(groupSortableSettings);
        $newRow.find('.ckeditor-row-controls a').show();
        redrawToolbarGradient();
        event.preventDefault();
      }

      /**
       * Remove a row of buttons.
       */
      function adminToolbarRemoveRow(event) {
        var $rows = $(this).closest('.ckeditor-toolbar-active').find('.ckeditor-row');
        if ($rows.length === 1) {
          $(this).hide();
        }
        if ($rows.length > 1) {
          var $lastRow = $rows.last();
          var $disabledButtons = $wrapper.find('.ckeditor-toolbar-disabled .ckeditor-buttons');
          $lastRow.find().children(':not(.ckeditor-multiple-button)').prependTo($disabledButtons);
          $lastRow.find('.ckeditor-buttons').sortable('destroy');
          $lastRow.find('.ckeditor-toolbar-groups').sortable('destroy');
          $lastRow.remove();
          redrawToolbarGradient();
          adminToolbarValue();
        }
        event.preventDefault();
      }

      /**
       * Browser quirk work-around to redraw CSS3 gradients.
       */
      function redrawToolbarGradient() {
        $wrapper.find('.ckeditor-toolbar-active').css('position', 'relative');
        window.setTimeout(function() {
          $wrapper.find('.ckeditor-toolbar-active').css('position', '');
        }, 10);
      }

      /**
       * jQuery Sortable stop event. Save updated toolbar positions to the textarea.
       */
      function adminToolbarStopDrag(event, ui) {
        var $element = ui.item;
        // Remove separators when dragged out.
        if ($element.is('.ckeditor-button-separator') && $element.closest('.ckeditor-active-toolbar-configuration').length === 0) {
          $element.remove();
        }
        adminToolbarValue();
      }

      /**
       * Update the toolbar value textarea.
       */
      function adminToolbarValue() {
        // Update the toolbar config after updating a sortable.
        var toolbarConfig = [];
        var $group, rowGroups, groupButtons;
        $wrapper.find('.ckeditor-row').each(function() {
          rowGroups = [];
          $(this).find('.ckeditor-toolbar-group').each(function() {
            $group = $(this);
            groupButtons = [];
            $group.find('.ckeditor-button').each(function() {
              groupButtons.push($(this).data('button-name'));
            });
            rowGroups.push({
              name: $group.data('group-name'),
              items: groupButtons
            });
          });
          toolbarConfig.push(rowGroups);
        });
        $textarea.val(JSON.stringify(toolbarConfig));
      }

    });
  }
};

/**
 * Themes the contents of an empty button group.
 */
Backdrop.theme.prototype.ckeditorButtonGroup = function () {
  // This may look odd but we indeed need an <li> wrapping an empty <ul>.
  return '<li class="ckeditor-toolbar-group"><h3 class="ckeditor-toolbar-group-name"></h3><ul class="ckeditor-buttons ckeditor-toolbar-group-buttons"></ul></li>';
};

/**
 * Themes a button that will toggle the button group names in active config.
 */
Backdrop.theme.prototype.ckeditorButtonGroupNamesToggle = function () {
  return '<a class="ckeditor-groupnames-toggle" role="button" aria-pressed="false"></a>';
};

})(jQuery, Backdrop);
