(function ($, Backdrop) {

"use strict";

Backdrop.behaviors.ckeditor5Admin = {
  attach: function (context, settings) {
    const $context = $(context);

    // Set up toolbar drag-and-drop interface and add/remove allowed HTML tags.
    $context.find('.ckeditor5-toolbar-configuration').once('ckeditor5-toolbar', function() {
      const $wrapper = $(this);
      const $textareaWrapper = $wrapper.find('.form-item-editor-settings-toolbar').hide();
      const $textarea = $textareaWrapper.find('textarea');
      const $toolbarAdmin = $(settings.ckeditor5.toolbarAdmin);
      const sortableSettings = {
        connectWith: '.ckeditor5-buttons',
        placeholder: 'ckeditor5-button-placeholder',
        forcePlaceholderSize: true,
        tolerance: 'pointer',
        cursor: 'move',
        stop: adminToolbarStopDrag
      };
      $toolbarAdmin.insertAfter($textareaWrapper);

      // Remove the invalid buttons after a delay to allow all behaviors to
      // finish attaching.
      window.setTimeout(function() {
        adminToolbarRemoveInvalidButtons();
        adminToolbarInitializeButtons();
      }, 10);

      // Add draggable/sortable behaviors.
      $toolbarAdmin.find('.ckeditor5-buttons').sortable(sortableSettings);
      $toolbarAdmin.find('.ckeditor5-multiple-buttons li').draggable({
        connectToSortable: '.ckeditor5-toolbar-active .ckeditor5-buttons',
        helper: 'clone'
      });

      // Add keyboard support for toolbar buttons.
      $toolbarAdmin.on('keydown', '.ckeditor5-buttons .ckeditor5-button', function(event) {
        const $element = $(event.target);
        const dir = document.documentElement.dir;
        switch (event.key) {
          case 'ArrowLeft':
            adminToolbarButtonMoveLeftRight($element, dir === 'ltr' ? 'previous' : 'next');
            event.preventDefault();
            break;
          case 'ArrowRight':
            adminToolbarButtonMoveLeftRight($element, dir === 'ltr' ? 'next' : 'previous');
            event.preventDefault();
            break;
          case 'ArrowDown':
            adminToolbarButtonMoveUpDown($element, 'next');
            event.preventDefault();
            break;
          case 'ArrowUp':
            adminToolbarButtonMoveUpDown($element, 'previous');
            event.preventDefault();
            break;
        }
      });

      // Special case for adding a multiple-instance button.
      $toolbarAdmin.on('keyup', '.ckeditor5-multiple-buttons .ckeditor5-multiple-button', function(event) {
        if (event.key === 'ArrowUp') {
          adminToolbarButtonClone($(event.target));
          event.preventDefault();
        }
      });

      // Add the handler for adding/removing row buttons.
      $toolbarAdmin.on('click.ckeditor5AddRow', function(event) {
        if ($(event.target).is('a.ckeditor5-row-add')) {
          adminToolbarAddRow.apply(event.target, [event]);
        }
      });
      $toolbarAdmin.on('click.ckeditor5AddRow', function(event) {
        if ($(event.target).is('a.ckeditor5-row-remove')) {
          adminToolbarRemoveRow.apply(event.target, [event]);
        }
      });
      $toolbarAdmin.find('a.ckeditor5-row-remove:first').hide();

      /**
       * Add a new row of buttons.
       */
      function adminToolbarAddRow(event) {
        const $rows = $(this).closest('.ckeditor5-toolbar-active').find('.ckeditor5-row');
        const $newRow = $rows.last().clone();
        $newRow.find('li').remove();
        $newRow.insertAfter($rows.last());
        $newRow.find('.ckeditor5-buttons').sortable(sortableSettings);
        $newRow.find('.ckeditor5-row-controls a').show();
        event.preventDefault();
      }

      /**
       * Remove a row of buttons.
       */
      function adminToolbarRemoveRow(event) {
        const $rows = $(this).closest('.ckeditor5-toolbar-active').find('.ckeditor5-row');
        if ($rows.length === 1) {
          $(this).hide();
        }
        if ($rows.length > 1) {
          const $lastRow = $rows.last();
          const $buttonsToDisable = $lastRow.find('.ckeditor5-button:not(.ckeditor-multiple-button)');
          $buttonsToDisable.each(function(n) {
            const $button = $buttonsToDisable.eq(n);
            const feature = adminToolbarButtonCreateFeature($button);
            adminToolbarRemoveButton($button, feature);
          });
          $lastRow.find('.ckeditor5-buttons').sortable('destroy');
          $lastRow.remove();
          adminToolbarUpdateValue();
        }
        event.preventDefault();
      }

      /**
       * jQuery Sortable stop event. Save updated toolbar positions to the textarea.
       */
      function adminToolbarStopDrag(event, ui) {
        const $element = ui.item;
        // Remove separators when dragged out.
        if ($element.is('.ckeditor5-multiple-button') && $element.closest('.ckeditor5-active-toolbar-configuration').length === 0) {
          $element.remove();
        }
        // Notify the filter system of updated or removed features.
        adminToolbarButtonMoved($element);
      }

      /**
       * Keyup handler for left/right arrow keys.
       */
      function adminToolbarButtonMoveLeftRight($element, direction) {
        if (direction === 'previous') {
          $element.prev('.ckeditor5-button').before($element);
        }
        else {
          $element.next('.ckeditor5-button').after($element);
        }
        $element.focus();
        adminToolbarButtonMoved($element);
      }

      /**
       * Keydown handler for up/down arrow keys.
       */
      function adminToolbarButtonMoveUpDown($element, direction) {
        const $currentRow = $element.closest('.ckeditor5-buttons');
        const $allRows = $element.closest('.ckeditor5-toolbar-configuration').find('.ckeditor5-buttons');
        let currentRowIndex = $allRows.index($currentRow);
        let $targetRow;

        // If there is a previous row, set that as the target.
        if (direction === 'previous' && currentRowIndex > 0) {
          $targetRow = $allRows.eq(currentRowIndex - 1);
        }
        // If there is a next row (including disabled row) set that as target.
        else if (direction === 'next' && currentRowIndex < $allRows.length - 1) {
          $targetRow = $allRows.eq(currentRowIndex + 1);
        }

        // Now actually move the button into the target row.
        if ($targetRow) {
          if ($element.is('.ckeditor5-multiple-button') && $targetRow.closest('.ckeditor5-toolbar-disabled').length) {
            $element.remove();
          }
          else {
            $targetRow.prepend($element);
          }
          $element.focus();
          adminToolbarButtonMoved($element);
        }
      }

      /**
       * Keyup handler for a multiple instance button being added.
       */
      function adminToolbarButtonClone($element) {
        const $targetRow = $element.closest('.ckeditor5-toolbar-configuration').find('.ckeditor5-active-toolbar-configuration .ckeditor5-buttons').last();
        const $newElement = $element.clone();
        $targetRow.prepend($newElement);
        $newElement.focus()
        adminToolbarButtonMoved($newElement);
      }

      /**
       * Notify the filter system of any button changes.
       */
      function adminToolbarButtonMoved($element) {
        const buttonFeature = adminToolbarButtonCreateFeature($element);
        const buttonAdded = $element.closest('.ckeditor5-active-toolbar-configuration').length !== 0;
        if (buttonFeature) {
          if (buttonAdded) {
            Backdrop.editorConfiguration.addedFeature(buttonFeature);
          }
          else {
            Backdrop.editorConfiguration.removedFeature(buttonFeature);
          }
        }

        // Update the underlying text field.
        adminToolbarUpdateValue();
      }

      /**
       * Convert a string of CKEditor tag syntax into an object.
       *
       * @param {String} htmlTag
       *   An HTML string such as '<a href class="external internal">'.
       * @return {Object}
       *   An object with the following keys:
       *   - tags: An array of the tags passed in (only one is supported).
       *   - attributes: An array of attributes on the tags.
       *   - styles: An array of style attributes on the tags.
       *   - classes: An array of all class names from this tag.
       */
      function adminToolbarSplitTag(htmlTag) {
        // Match everything outside of quotes within the tag.
        const attributes = htmlTag.match(/([a-z\-]+)(?:=?['"].*?['"])?/ig);
        // Pop off the first match, which is the tag name itself.
        const tagName = attributes.shift();
        let classList = [], styleList = [];
        if (attributes.indexOf('class') > -1) {
          const classMatches = htmlTag.match(/class="([a-z_\- ]+)"/);
          if (classMatches) {
            classList = classMatches[1].split(/\s/)
          }
        }
        if (attributes.indexOf('style') > -1) {
          const styleMatches = htmlTag.match(/style="([a-z_\- ]+)"/)[1].split(/\s/);
          if (styleMatches) {
            styleList = styleMatches[1].split(/\s/)
          }
        }
        return {
          attributes: attributes,
          tags: [tagName],
          classes: classList,
          styles: styleList,
        };
      }

      /**
       * Create a Backdrop.EditorFeatureHTMLRule instance based on a button DOM element.
       */
      function adminToolbarButtonCreateFeature($element) {
        const requiredHtml = $element.data('required-html') || [];
        const optionalHtml = $element.data('optional-html') || [];
        const buttonName = $element.data('button-name');
        let buttonFeature, buttonRule, buttonRuleDefinition;
        if (buttonName) {
          buttonFeature = new Backdrop.EditorFeature(buttonName);
          requiredHtml.forEach(htmlTag => {
            buttonRuleDefinition = adminToolbarSplitTag(htmlTag);
            buttonRuleDefinition.required = true;
            buttonRule = new Backdrop.EditorFeatureHTMLRule(buttonRuleDefinition);
            buttonFeature.addHTMLRule(buttonRule);
          });
          optionalHtml.forEach(htmlTag => {
            buttonRuleDefinition = adminToolbarSplitTag(htmlTag);
            buttonRuleDefinition.required = false;
            buttonRule = new Backdrop.EditorFeatureHTMLRule(buttonRuleDefinition);
            buttonFeature.addHTMLRule(buttonRule);
          });
        }
        else {
          buttonFeature = false;
        }

        return buttonFeature;
      }

      /**
       * Update the toolbar value textarea.
       */
      function adminToolbarUpdateValue() {
        // Update the toolbar config after updating a sortable.
        const toolbarConfig = [];
        $wrapper.find('.ckeditor5-row').each(function() {
          $(this).find('.ckeditor5-button').each(function() {
            toolbarConfig.push($(this).data('button-name'));
          });
          // CKEditor5 uses a dash to indicate a line break in a row.
          toolbarConfig.push('-');
        });
        // Remove the last row line break.
        toolbarConfig.pop();
        $textarea.val(JSON.stringify(toolbarConfig));
      }

      /**
       * Remove a single button from the toolbar.
       */
      function adminToolbarRemoveButton($button, feature) {
        // Put the button back into the disabled list if it's not a separator.
        if ($button.is('.ckeditor5-multiple-button')) {
          $button.remove();
        }
        else {
          $wrapper.find('.ckeditor5-toolbar-disabled .ckeditor5-buttons').prepend($button);
        }

        // Fire event indicating this button/feature was removed.
        Backdrop.editorConfiguration.removedFeature(feature);
      }

      /**
       * Notify the editor system of the initial button state.
       */
      function adminToolbarInitButton($button, feature, enabled) {
        // Fire event indicating this button's initial status.
        Backdrop.editorConfiguration.initFeature(feature, enabled);
      }

      /**
       * Ensure the configuration of the toolbar is allowed by the filters.
       */
      function adminToolbarRemoveInvalidButtons() {
        const rules = Backdrop.filterConfiguration.getCombinedFilterRules();
        $wrapper.find('.ckeditor5-toolbar-active .ckeditor5-button').each(function () {
          const $button = $(this);
          const feature = adminToolbarButtonCreateFeature($button);
          if (feature && !Backdrop.editorConfiguration.featureIsAllowed(feature, rules)) {
            adminToolbarRemoveButton($button, feature);
          }
        });
        adminToolbarUpdateValue();
      }

      /**
       * Notify listeners to the initial state of the buttons/features.
       */
      function adminToolbarInitializeButtons() {
        $wrapper.find('.ckeditor5-toolbar-active .ckeditor5-button').each(function () {
          const $button = $(this);
          const feature = adminToolbarButtonCreateFeature($button);
          adminToolbarInitButton($button, feature, true);
        });
        $wrapper.find('.ckeditor5-toolbar-disabled .ckeditor5-button').each(function() {
          const $button = $(this);
          const feature = adminToolbarButtonCreateFeature($button);
          adminToolbarInitButton($button, feature, false);
        });
      }
    });

    // Adding or removing a heading option needs to add matching HTML tag.
    $context.find('.ckeditor5-heading-list').once('ckeditor5-heading-list', function() {
      const $checkboxes = $(this).find('input:checkbox');
      const headingFeatures = {};
      $checkboxes.each(function() {
        const headingLevel = this.value;
        const headingFeature = new Backdrop.EditorFeature(headingLevel);
        const headingRule = new Backdrop.EditorFeatureHTMLRule({
          'required': true,
          'tags': [headingLevel]
        });
        headingFeature.addHTMLRule(headingRule)
        Backdrop.editorConfiguration.initFeature(headingFeature);
        headingFeatures[headingLevel] = headingFeature;
      });

      $checkboxes.on('change', function() {
        const headingLevel = this.value;
        const headingFeature = headingFeatures[headingLevel];
        if (this.checked) {
          Backdrop.editorConfiguration.addedFeature(headingFeature);
        }
        else {
          Backdrop.editorConfiguration.removedFeature(headingFeature);
        }
      });
    });
  }
};

/**
 * Respond to the events of the editor system.
 *
 * This handles hiding/showing options based on the enabling, disabling, and
 * initial state of buttons.
 */
Backdrop.behaviors.ckeditor5AdminToggle = {
  attach: function(context, settings) {
    const ckeditor5AdminToggleDependency = function(featureName, enabled) {
      $('[data-ckeditor5-feature-dependency]').each(function() {
        const $element = $(this);
        const dependency = $element.data('ckeditor5-feature-dependency');
        const tab = $element.data('verticalTab');
        if (dependency === featureName) {
          if (enabled) {
            $element.show();
            tab && tab.tabShow();
          }
          else {
            $element.hide();
            tab && tab.tabHide();
          }
        }
      });
    };

    $(context).find('.ckeditor5-toolbar-configuration').once('ckeditor5-settings-toggle', function() {
      $(this).find('[data-ckeditor5-feature-dependency]').hide();
      $(document).on('backdropEditorFeatureInit.ckeditor5AdminToggle', function(e, feature, enabled) {
        ckeditor5AdminToggleDependency(feature.name, enabled);
      });
      $(document).on('backdropEditorFeatureAdded.ckeditor5AdminToggle', function(e, feature) {
        ckeditor5AdminToggleDependency(feature.name, true);
      });
      $(document).on('backdropEditorFeatureRemoved.ckeditor5AdminToggle', function(e, feature) {
        ckeditor5AdminToggleDependency(feature.name, false);
      });
    });
  },
  detach: function(context, settings) {
    $(context).find('#filter-admin-format-form').each(function() {
      $(document).off('.ckeditor5AdminToggle');
    });
  }
};

})(jQuery, Backdrop);
