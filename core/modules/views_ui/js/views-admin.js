/**
 * @file
 * Some basic behaviors and utility functions for Views UI.
 */
(function ($) {

"use strict";

Backdrop.viewsUi = {};

/**
 * Improve the user experience of the views edit interface.
 */
Backdrop.behaviors.viewsUiEditView = {
  attach: function () {
    // Only show the SQL rewrite warning when the user has chosen the
    // corresponding checkbox.
    $('#edit-query-options-disable-sql-rewrite').on('click', function () {
      $('.sql-rewrite-warning').toggleClass('js-hide');
    });
  }
};

/**
 * In the add view wizard, use the view name to prepopulate form fields such as
 * page title and menu link.
 */
Backdrop.behaviors.viewsUiAddView = {
  attach: function (context) {
    var $context = $(context);
    var replace = '-';
    var suffix;

    // The page title, block title, and menu link fields can all be prepopulated
    // with the view name - no regular expression needed.
    var $fields = $context.find('[id^="edit-page-title"], [id^="edit-block-title"], [id^="edit-page-link-properties-title"]');
    if ($fields.length) {
      if (!this.fieldsFiller) {
        this.fieldsFiller = new Backdrop.viewsUi.FormFieldFiller($fields);
      }
      else {
        // After an AJAX response, this.fieldsFiller will still have event
        // handlers bound to the old version of the form fields (which don't exist
        // anymore). The event handlers need to be unbound and then rebound to the
        // new markup. Note that jQuery.live is difficult to make work in this
        // case because the IDs of the form fields change on every AJAX response.
        this.fieldsFiller.rebind($fields);
      }
    }
    // Prepopulate the path field with a URLified version of the view name.
    var $pathField = $context.find('[id^="edit-page-path"]');
    if ($pathField.length) {
      if (!this.pathFiller) {
        this.pathFiller = new Backdrop.viewsUi.FormFieldFiller($pathField, replace);
      }
      else {
        this.pathFiller.rebind($pathField);
      }
    }
    // Populate the RSS feed field with a URLified version of the view name, and
    // an .xml suffix (to make it unique).
    var $feedField = $context.find('[id^="edit-page-feed-properties-path"]');
    if ($feedField.length) {
      if (!this.feedFiller) {
        suffix = '.xml';
        this.feedFiller = new Backdrop.viewsUi.FormFieldFiller($feedField, replace, suffix);
      }
      else {
        this.feedFiller.rebind($feedField);
      }
    }
  }
};

/**
 * Constructor for the Backdrop.viewsUi.FormFieldFiller object.
 *
 * Prepopulates a form field based on the view name.
 *
 * @param $target
 *   A jQuery object representing the form field to prepopulate.
 * @param replace
 *   Optional. A string to use as the replacement value for disallowed
 *   characters.
 * @param suffix
 *   Optional. A suffix to append at the end of the target field content.
 */
Backdrop.viewsUi.FormFieldFiller = function ($target, replace, suffix) {
  this.source = $('#edit-human-name');
  this.target = $target;
  this.replace = replace || '';
  this.suffix = suffix || '';

  // Copy the transliteration options from the machine name element.
  var machineNameData = $('[data-machine-name]').data('machine-name');
  this.transliterationOptions = {
    replace: machineNameData.replace,
    replace_pattern: machineNameData.replace_pattern,
    replace_token: machineNameData.replace_token,
    langcode: machineNameData.langcode
  };

  // Create bound versions of this instance's object methods to use as event
  // handlers. This will let us unbind those specific handlers later on.
  // NOTE: $.proxy will not work for this because it assumes we want only
  // one bound version of an object method, whereas we need one version per
  // object instance.
  var self = this;
  this.populate = function () {return self._populate.call(self);};
  this.unbind = function () {return self._unbind.call(self);};

  this.bind();
  // Object constructor; no return value.
};

$.extend(Backdrop.viewsUi.FormFieldFiller.prototype, {
  /**
   * Bind the form-filling behavior.
   */
  bind: function () {
    this.unbind();
    // Populate the form field when the source changes.
    this.source.on('keyup.viewsUi change.viewsUi', this.populate);
    // Quit populating the field as soon as it gets focus.
    this.target.on('focus.viewsUi', this.unbind);
  },

  /**
   * Get the source form field value as altered by the passed-in parameters.
   */
  getTransliterated: function () {
    var self = this;
    return $.ajax({
      url: Backdrop.settings.basePath + "?q=" + Backdrop.encodePath("system/transliterate/" + self.source.val().toLowerCase()),
      data: self.transliterationOptions,
      dataType: "text"
    });
  },

  /**
   * Use the title for populating the fields, or send a request
   * for a translitarated version of the source field value when needed.
   */
  _populate: function () {
    if (this.replace == '') {
      this.target.val(this.source.val() + this.suffix);
    }
    else {
      var transliterated = this.getTransliterated();
      var self = this;
      transliterated.done(function(machine) {
        // Replace the machine name placeholder with the specific one for this
        // field. e.g. A hyphen instead of an underscore for the path.
        machine = machine.replace(new RegExp(self.transliterationOptions.replace,  'g'), self.replace);
        self.target.val(machine + self.suffix);
      });
    }
  },

  /**
   * Stop prepopulating the form fields.
   */
  _unbind: function () {
    this.source.off('keyup.viewsUi change.viewsUi', this.populate);
    this.target.off('focus.viewsUi', this.unbind);
  },

  /**
   * Bind event handlers to the new form fields, after they're replaced via AJAX.
   */
  rebind: function ($fields) {
    this.target = $fields;
    this.bind();
  }
});

Backdrop.behaviors.addItemForm = {
  attach: function (context) {
    // The add item form may have an id of views-ui-add-item-form--n.
    var $form = $(context).find('form[id^="views-ui-add-item-form"]').addBack('form[id^="views-ui-add-item-form"]').first();
    // Make sure we don't add more than one event handler to the same form.
    $form.once('views-ui-add-item-form', function() {
      new Backdrop.viewsUi.AddItemForm($form);
    });
  }
};

Backdrop.viewsUi.AddItemForm = function ($form) {
  $form.on('click', '.views-filterable-options :checkbox', $.proxy(this.handleCheck, this));

  // Find the wrapper of the displayed text and hide it until items are checked.
  this.$form = $form;
  this.$selected_div = this.$form.find('.views-selected-options').parent();
  this.$selected_div.hide();
  this.checkedItems = [];
};

Backdrop.viewsUi.AddItemForm.prototype.handleCheck = function (event) {
  var $target = $(event.target);
  var label = $.trim($target.next().text());
  // Add/remove the checked item to the list.
  if ($target.is(':checked')) {
    this.$selected_div.css('display', 'block');
    this.checkedItems.push(label);
  }
  else {
    var position = $.inArray(label, this.checkedItems);
    // Delete the item from the list and take sure that the list doesn't have undefined items left.
    for (var i = 0; i < this.checkedItems.length; i++) {
      if (i == position) {
        this.checkedItems.splice(i, 1);
        i--;
        break;
      }
    }
    // Hide it again if none item is selected.
    if (this.checkedItems.length == 0) {
      this.$selected_div.hide();
    }
  }
  this.refreshCheckedItems();
};

/**
 * Refresh the display of the checked items.
 */
Backdrop.viewsUi.AddItemForm.prototype.refreshCheckedItems = function () {
  // Perhaps we should precache the text div, too.
  this.$selected_div.find('.views-selected-options')
    .html(this.checkedItems.join(', '))
    .trigger('dialogContentResize');
};

/**
 * The input field items that add displays must be rendered as <input> elements.
 * The following behavior detaches the <input> elements from the DOM, wraps them
 * in an unordered list, then appends them to the list of tabs.
 */
Backdrop.behaviors.viewsUiRenderAddViewButton = {
  attach: function (context) {
    // Build the add display menu and pull the display input buttons into it.
    var $menu = $(context).find('#views-display-menu-tabs').once('views-ui-render-add-view-button-processed');
    if (!$menu.length) {
      return;
    }
    var $addDisplayDropdown = $('<li class="add"><a href="#"><span class="icon add"></span>' + Backdrop.t('Add') + '</a><ul class="action-list" style="display:none;"></ul></li>');
    var $displayButtons = $menu.nextAll('input.add-display').detach();
    $displayButtons.appendTo($addDisplayDropdown.find('.action-list')).wrap('<li>')
      .parent().first().addClass('first').end().last().addClass('last');
    // Remove the 'Add ' prefix from the button labels since they're being palced
    // in an 'Add' dropdown.
    // @todo This assumes English, but so does $addDisplayDropdown above. Add
    //   support for translation.
    $displayButtons.each(function () {
      var label = $(this).val();
      if (label.substr(0, 4) === 'Add ') {
        $(this).val(label.substr(4));
      }
    });
    $addDisplayDropdown.appendTo($menu);

    // Add the click handler for the add display button
    $menu.find('li.add > a').on('click', function (event) {
      event.preventDefault();
      var $trigger = $(this);
      Backdrop.behaviors.viewsUiRenderAddViewButton.toggleMenu($trigger);
    });
    // Add a mouseleave handler to close the dropdown when the user mouses
    // away from the item. We use mouseleave instead of mouseout because
    // the user is going to trigger mouseout when she moves from the trigger
    // link to the sub menu items.
    // We use the live binder because the open class on this item will be
    // toggled on and off and we want the handler to take effect in the cases
    // that the class is present, but not when it isn't.
    $('li.add', $menu).on('mouseleave', function (event) {
      var $this = $(this);
      var $trigger = $this.children('a[href="#"]');
      if ($this.children('.action-list').is(':visible')) {
        Backdrop.behaviors.viewsUiRenderAddViewButton.toggleMenu($trigger);
      }
    });
  }
};

Backdrop.behaviors.viewsUiRenderAddViewButton.toggleMenu = function ($trigger) {
  $trigger.parent().toggleClass('open');
  $trigger.next().toggle();
};

Backdrop.behaviors.viewsUiSearchOptions = {
  attach: function (context) {
    // The add item form may have an id of views-ui-add-item-form--n.
    var $form = $(context).find('form[id^="views-ui-add-item-form"]').addBack('form[id^="views-ui-add-item-form"]').first();
    // Make sure we don't add more than one event handler to the same form.
    $form.once('views-ui-filter-options', function() {
      new Backdrop.viewsUi.OptionsSearch($form);
    });
  }
};

/**
 * Constructor for the viewsUi.OptionsSearch object.
 *
 * The OptionsSearch object filters the available options on a form according
 * to the user's search term. Typing in "taxonomy" will show only those options
 * containing "taxonomy" in their label.
 */
Backdrop.viewsUi.OptionsSearch = function ($form) {
  this.$form = $form;
  // Add a keyup handler to the search box.
  this.$searchBox = this.$form.find('#edit-options-search');
  this.$searchBox.keyup($.proxy(this.handleKeyup, this));
  // Get a list of option labels and their corresponding divs and maintain it
  // in memory, so we have as little overhead as possible at keyup time.
  this.options = this.getOptions(this.$form.find('.filterable-option'));
  // Restripe on initial loading.
  this.handleKeyup();

  // Add a change handler to the filter group select.
  this.$filterGroup = this.$form.find('select[name="group"]');
  this.$filterGroup.change($.proxy(this.filterGroup, this));
  // Trap the ENTER key in the search box so that it doesn't submit the form.
  this.$searchBox.on('keypress', function (event) {
    if (event.which == 13) {
      event.preventDefault();
    }
  });
};

$.extend(Backdrop.viewsUi.OptionsSearch.prototype, {
  /**
   * Assemble a list of all the filterable options on the form.
   *
   * @param $allOptions
   *   A $ object representing the rows of filterable options to be
   *   shown and hidden depending on the user's search terms.
   */
  getOptions: function ($allOptions) {
    var i, $label, $description, $option;
    var options = [];
    var length = $allOptions.length;
    for (i = 0; i < length; i++) {
      $option = $($allOptions[i]);
      $label = $option.find('label');
      $description = $option.find('div.description');
      options[i] = {
        // Search on the lowercase version of the label text + description.
        'searchText': $label.text().toLowerCase() + " " + $description.text().toLowerCase(),
        // Maintain a reference to the jQuery object for each row, so we don't
        // have to create a new object inside the performance-sensitive keyup
        // handler.
        '$div': $option
      };
    }
    return options;
  },

  /**
   * Keyup handler for the search box that hides or shows the relevant options.
   */
  handleKeyup: function (event) {
    var found, i, j, option, search, words, wordsLength, zebraClass, zebraCounter;

    // Determine the user's search query. The search text has been converted to
    // lowercase.
    search = this.$searchBox.val().toLowerCase();
    words = search.split(' ');
    wordsLength = words.length;

    // Start the counter for restriping rows.
    zebraCounter = 0;

    // Search through the search texts in the form for matching text.
    var length = this.options.length;
    for (i = 0; i < length; i++) {
      // Use a local variable for the option being searched, for performance.
      option = this.options[i];
      found = true;
      // Each word in the search string has to match the item in order for the
      // item to be shown.
      for (j = 0; j < wordsLength; j++) {
        if (option.searchText.indexOf(words[j]) === -1) {
          found = false;
        }
      }
      if (found) {
        // Show the checkbox row, and restripe it.
        zebraClass = (zebraCounter % 2) ? 'odd' : 'even';
        option.$div.show();
        option.$div.removeClass('even odd');
        option.$div.addClass(zebraClass);
        zebraCounter++;
      }
      else {
        // The search string wasn't found; hide this item.
        option.$div.hide();
      }
    }
  },

  /**
   * Filter down the list of all options based on group.
   */
  filterGroup: function() {
    var groupName = this.$filterGroup.val();
    if (groupName === 'all') {
      this.$form.find('.views-filterable-group').show();
    }
    else {
      this.$form.find('.views-filterable-group').hide();
      this.$form.find('.views-filterable-group-' + groupName).show();
    }
  }
});

Backdrop.behaviors.viewsUiPreview = {
 attach: function (context) {
   // Only act on the edit view form.
   var $contextualFiltersBucket = $(context).find('.views-display-column .views-ui-display-tab-bucket.contextual-filters');
   if ($contextualFiltersBucket.length === 0) {
     return;
   }

   // If the display has no contextual filters, hide the form where you enter
   // the contextual filters for the live preview. If it has contextual filters,
   // show the form.
   var $contextualFilters = $contextualFiltersBucket.find('.views-display-setting a');
   if ($contextualFilters.length) {
     $('#preview-args').parent().show();
   }
   else {
     $('#preview-args').parent().hide();
   }

   // Executes an initial preview.
   if ($('#edit-displays-live-preview').once('edit-displays-live-preview').is(':checked')) {
     $('#preview-submit').once('edit-displays-live-preview').trigger('click');
    }
  }
};

/**
 * Remove links when rearranging fields.
 */
Backdrop.behaviors.viewsUiRemoveLink = {
  attach: function (context) {
    $('a.views-remove-link').once('views-processed').click(function(event) {
      var id = $(this).attr('id').replace('views-remove-link-', '');
      $('#views-row-' + id).hide();
      $('#views-removed-' + id).attr('checked', true);
      event.preventDefault();
    });
  }
};

Backdrop.behaviors.viewsUiRearrangeFilter = {
  attach: function (context) {
    // Only act on the rearrange filter form.
    if (typeof Backdrop.tableDrag === 'undefined' || typeof Backdrop.tableDrag['views-rearrange-filters'] === 'undefined') {
      return;
    }
    var $context = $(context);
    var $table = $context.find('#views-rearrange-filters').once('views-rearrange-filters');
    var $operator = $context.find('.form-item-filter-groups-operator').once('views-rearrange-filters');
    if ($table.length) {
      new Backdrop.viewsUi.RearrangeFilterHandler($table, $operator);
    }
  }
};

/**
 * Improve the UI of the rearrange filters dialog box.
 */
Backdrop.viewsUi.RearrangeFilterHandler = function ($table, $operator) {
  // Keep a reference to the <table> being altered and to the div containing
  // the filter groups operator dropdown (if it exists).
  this.table = $table;
  this.operator = $operator;
  this.hasGroupOperator = this.operator.length > 0;

  // Keep a reference to all draggable rows within the table.
  this.draggableRows = $table.find('.draggable');

  // Keep a reference to the buttons for adding and removing filter groups.
  this.addGroupButton = $('input#views-add-group');
  this.removeGroupButtons = $table.find('input.views-remove-group');

  // Add links that duplicate the functionality of the (hidden) add and remove
  // buttons.
  this.insertAddRemoveFilterGroupLinks();

  // When there is a filter groups operator dropdown on the page, create
  // duplicates of the dropdown between each pair of filter groups.
  if (this.hasGroupOperator) {
    this.dropdowns = this.duplicateGroupsOperator();
    this.syncGroupsOperators();
  }

  // Add methods to the tableDrag instance to account for operator cells (which
  // span multiple rows), the operator labels next to each filter (e.g., "And"
  // or "Or"), the filter groups, and other special aspects of this tableDrag
  // instance.
  this.modifyTableDrag();

  // Initialize the operator labels (e.g., "And" or "Or") that are displayed
  // next to the filters in each group, and bind a handler so that they change
  // based on the values of the operator dropdown within that group.
  var self = this;
  window.setTimeout(function() {
    self.redrawOperatorLabels();
  }, 100);

  $table.find('.views-group-title select')
    .once('views-rearrange-filter-handler')
    .on('change.views-rearrange-filter-handler', $.proxy(this, 'redrawOperatorLabels'));

  // Bind handlers so that when a "Remove" link is clicked, we:
  // - Update the rowspans of cells containing an operator dropdown (since they
  //   need to change to reflect the number of rows in each group).
  // - Redraw the operator labels next to the filters in the group (since the
  //   filter that is currently displayed last in each group is not supposed to
  //   have a label display next to it).
  $table.find('a.views-groups-remove-link')
    .once('views-rearrange-filter-handler')
    .on('click.views-rearrange-filter-handler', $.proxy(this, 'updateRowspans'))
    .on('click.views-rearrange-filter-handler', $.proxy(this, 'redrawOperatorLabels'));
};

$.extend(Backdrop.viewsUi.RearrangeFilterHandler.prototype, {
  /**
   * Insert links that allow filter groups to be added and removed.
   */
  insertAddRemoveFilterGroupLinks: function () {
    // Insert a link for adding a new group at the top of the page, and make it
    // match the action links styling used in a typical page.tpl.php. Note that
    // Backdrop does not provide a theme function for this markup, so this is the
    // best we can do.
    $('<ul class="action-links"><li><a id="views-add-group-link" href="#">' + this.addGroupButton.val() + '</a></li></ul>')
      .prependTo(this.table.parent())
      // When the link is clicked, dynamically click the hidden form button for
      // adding a new filter group.
      .once('views-rearrange-filter-handler')
      .on('click.views-rearrange-filter-handler', $.proxy(this, 'clickAddGroupButton'));

    // Find each (visually hidden) button for removing a filter group and insert
    // a link next to it.
    var length = this.removeGroupButtons.length;
    var i;
    for (i = 0; i < length; i++) {
      var $removeGroupButton = $(this.removeGroupButtons[i]);
      var buttonId = $removeGroupButton.attr('id');
      $('<a href="#" class="views-remove-group-link">' + Backdrop.t('Remove group') + '</a>')
        .insertBefore($removeGroupButton)
        // When the link is clicked, dynamically click the corresponding form
        // button.
        .once('views-rearrange-filter-handler')
        .on('click.views-rearrange-filter-handler', { buttonId: buttonId }, $.proxy(this, 'clickRemoveGroupButton'));
    }
  },

  /**
   * Dynamically click the button that adds a new filter group.
   */
  clickAddGroupButton: function (event) {
    // Due to conflicts between Backdrop core's AJAX system and the Views AJAX
    // system, the only way to get this to work seems to be to trigger both the
    // .mousedown() and .submit() events.
    this.addGroupButton
      .trigger('mousedown')
      .trigger('submit');
    event.preventDefault();
  },

  /**
   * Dynamically click a button for removing a filter group.
   *
   * @param event
   *   Event being triggered, with event.data.buttonId set to the ID of the
   *   form button that should be clicked.
   */
  clickRemoveGroupButton: function (event) {
    this.table.find('#' + event.data.buttonId).trigger('mousedown').trigger('submit');
    event.preventDefault();
  },

  /**
   * Move the groups operator so that it's between the first two groups, and
   * duplicate it between any subsequent groups.
   */
  duplicateGroupsOperator: function () {
    var $dropdowns, $newRow, $titleRow;
    var $titleRows = $('tr.views-group-title');

    // Get rid of the explanatory text around the operator; its placement is
    // explanatory enough.
    this.operator.find('label').add('div.description').addClass('element-invisible');
    this.operator.find('select').addClass('form-select');

    // Keep a list of the operator dropdowns, so we can sync their behavior later.
    $dropdowns = this.operator;

    // Move the operator to a new row just above the second group.
    $titleRow = $('tr#views-group-title-2');
    $newRow = $('<tr class="filter-group-operator-row"><td colspan="5"></td></tr>');
    $newRow.find('td').append(this.operator);
    $newRow.insertBefore($titleRow);
    var $fakeOperator, i, length = $titleRows.length;
    // Starting with the third group, copy the operator to a new row above the
    // group title.
    for (i = 2; i < length; i++) {
      $titleRow = $($titleRows[i]);
      // Make a copy of the operator dropdown and put it in a new table row.
      $fakeOperator = this.operator.clone();
      $fakeOperator.attr('id', '');
      $newRow = $newRow.clone();
      $newRow.find('td').html($fakeOperator);
      $newRow.insertBefore($titleRow);
      $dropdowns = $dropdowns.add($fakeOperator);
    }

    return $dropdowns;
  },

  /**
   * Make the duplicated groups operators change in sync with each other.
   */
  syncGroupsOperators: function () {
    if (this.dropdowns.length < 2) {
      // We only have one dropdown (or none at all), so there's nothing to sync.
      return;
    }

    this.dropdowns.on('change', $.proxy(this, 'operatorChangeHandler'));
  },
  /**
   * Click handler for the operators that appear between filter groups.
   * Forces all operator dropdowns to have the same value.
   */
  operatorChangeHandler: function (event) {
    var $target = $(event.target);
    var operators = this.dropdowns.find('select').not($target);

    // Change the other operators to match this new value.
    operators.val($target.val());
  },

  modifyTableDrag: function () {
    var tableDrag = Backdrop.tableDrag['views-rearrange-filters'];
    var filterHandler = this;

    /**
     * Override the row.onSwap method from tabledrag.js.
     *
     * When a row is dragged to another place in the table, several things need
     * to occur.
     * - The row needs to be moved so that it's within one of the filter groups.
     * - The operator cells that span multiple rows need their rowspan attributes
     *   updated to reflect the number of rows in each group.
     * - The operator labels that are displayed next to each filter need to be
     *   redrawn, to account for the row's new location.
     */
    tableDrag.row.prototype.onSwap = function () {
      if (filterHandler.hasGroupOperator) {
        // Make sure the row that just got moved (this.group) is inside one of
        // the filter groups (i.e. below an empty marker row or a draggable). If
        // it isn't, move it down one.
        var thisRow = $(this.group);
        var previousRow = thisRow.prev('tr');
        if (previousRow.length && !previousRow.hasClass('group-message') && !previousRow.hasClass('draggable')) {
          // Move the dragged row down one.
          var next = thisRow.next();
          if (next.is('tr')) {
            this.swap('after', next);
          }
        }
        filterHandler.updateRowspans();
      }
      // Redraw the operator labels that are displayed next to each filter, to
      // account for the row's new location.
      filterHandler.redrawOperatorLabels();
    };

    /**
     * Override the onDrop method from tabledrag.js.
     */
    tableDrag.onDrop = function () {
      // If the tabledrag change marker (i.e., the "*") has been inserted inside
      // a row after the operator label (i.e., "And" or "Or") rearrange the items
      // so the operator label continues to appear last.
      var changeMarker = $(this.oldRowElement).find('.tabledrag-changed');
      if (changeMarker.length) {
        // Search for occurrences of the operator label before the change marker,
        // and reverse them.
        var operatorLabel = changeMarker.prevAll('.views-operator-label');
        if (operatorLabel.length) {
          operatorLabel.insertAfter(changeMarker);
        }
      }
      // Make sure the "group" dropdown is properly updated when rows are dragged
      // into an empty filter group. This is borrowed heavily from the block.js
      // implementation of tableDrag.onDrop().
      var groupRow = $(this.rowObject.element).prevAll('tr.group-message').get(0);
      var groupName = groupRow.className.replace(/([^ ]+[ ]+)*group-([^ ]+)-message([ ]+[^ ]+)*/, '$2');
      var groupField = $('select.views-group-select', this.rowObject.element);
      if ($(this.rowObject.element).prev('tr').is('.group-message') && !groupField.is('.views-group-select-' + groupName)) {
        var oldGroupName = groupField.attr('class').replace(/([^ ]+[ ]+)*views-group-select-([^ ]+)([ ]+[^ ]+)*/, '$2');
        groupField.removeClass('views-group-select-' + oldGroupName).addClass('views-group-select-' + groupName);
        groupField.val(groupName);
      }
    };
  },

  /**
   * Redraw the operator labels that are displayed next to each filter.
   */
  redrawOperatorLabels: function () {
    for (var i = 0; i < this.draggableRows.length; i++) {
      // Within the row, the operator labels are displayed inside the first table
      // cell (next to the filter name).
      var $draggableRow = $(this.draggableRows[i]);
      var $firstCell = $draggableRow.find('td:first');
      if ($firstCell.length) {
        // The value of the operator label ("And" or "Or") is taken from the
        // first operator dropdown we encounter, going backwards from the current
        // row. This dropdown is the one associated with the current row's filter
        // group.
        var operatorValue = $draggableRow.prevAll('.views-group-title').find('option:selected').last().html();
        var operatorLabel = '<span class="views-operator-label">' + operatorValue + '</span>';
        // If the next visible row after this one is a draggable filter row,
        // display the operator label next to the current row. (Checking for
        // visibility is necessary here since the "Remove" links hide the removed
        // row but don't actually remove it from the document).
        var $nextRow = $draggableRow.nextAll(':visible').eq(0);
        var $existingOperatorLabel = $firstCell.find('.views-operator-label');
        if ($nextRow.hasClass('draggable')) {
          // If an operator label was already there, replace it with the new one.
          if ($existingOperatorLabel.length) {
            $existingOperatorLabel.replaceWith(operatorLabel);
          }
          // Otherwise, append the operator label to the end of the table cell.
          else {
            $firstCell.append(operatorLabel);
          }
        }
        // If the next row doesn't contain a filter, then this is the last row
        // in the group. We don't want to display the operator there (since
        // operators should only display between two related filters, e.g.
        // "filter1 AND filter2 AND filter3"). So we remove any existing label
        // that this row has.
         else {
           $existingOperatorLabel.remove();
         }
       }
     }
   },
   /**
   * Update the rowspan attribute of each cell containing an operator dropdown.
   */
  updateRowspans: function () {
    var i, $row, $currentEmptyRow, draggableCount, $operatorCell;
    var rows = $(this.table).find('tr');
    var length = rows.length;
    for (i = 0; i < length; i++) {
      $row = $(rows[i]);
      if ($row.hasClass('views-group-title')) {
        // This row is a title row.
        // Keep a reference to the cell containing the dropdown operator.
        $operatorCell = $row.find('td.group-operator');
        // Assume this filter group is empty, until we find otherwise.
        draggableCount = 0;
        $currentEmptyRow = $row.next('tr');
        $currentEmptyRow.removeClass('group-populated').addClass('group-empty');
        // The cell with the dropdown operator should span the title row and
        // the "this group is empty" row.
        $operatorCell.attr('rowspan', 2);
      }
      else if ($row.hasClass('draggable') && $row.is(':visible')) {
        // We've found a visible filter row, so we now know the group isn't empty.
        draggableCount++;
        $currentEmptyRow.removeClass('group-empty').addClass('group-populated');
        // The operator cell should span all draggable rows, plus the title.
        $operatorCell.attr('rowspan', draggableCount + 1);
      }
    }
  }
});

/**
 * Add a select all checkbox, which checks each checkbox at once.
 */
Backdrop.behaviors.viewsFilterConfigSelectAll = {
  attach: function (context) {
    // Show the select all checkbox.
    $(context).find('#views-ui-config-item-form div.form-item-options-value-all').once('filterConfigSelectAll')
      .show()
      .find('input[type=checkbox]')
      .on('click', function () {
        var checked = $(this).is(':checked');
        // Update all checkbox beside the select all checkbox.
        $(this).parents('.form-checkboxes').find('input[type=checkbox]').each(function () {
          $(this).attr('checked', checked);
        });
      });
    // Uncheck the select all checkbox if any of the others are unchecked.
    $('#views-ui-config-item-form').find('div.form-type-checkbox').not($('.form-item-options-value-all'))
      .find('input[type=checkbox]')
      .on('click', function () {
        if ($(this).is('checked') === false) {
          $('#edit-options-value-all').prop('checked', false);
        }
      });
  }
};

/**
 * Ensure the desired default button is used when a form is implicitly submitted via an ENTER press on textfields, radios, and checkboxes.
 *
 * @see http://www.w3.org/TR/html5/association-of-controls-and-forms.html#implicit-submission
 */
Backdrop.behaviors.viewsImplicitFormSubmission = {
  attach: function (context, settings) {
    $(':text, :password, :radio, :checkbox', context).once('viewsImplicitFormSubmission', function() {
      $(this).keypress(function(event) {
        if (event.which == 13) {
          var formId = this.form.id;
          if (formId && settings.viewsImplicitFormSubmission && settings.viewsImplicitFormSubmission[formId] && settings.viewsImplicitFormSubmission[formId].defaultButton) {
            event.preventDefault();
            var buttonId = settings.viewsImplicitFormSubmission[formId].defaultButton;
            var $button = $('#' + buttonId, this.form);
            if ($button.length == 1 && $button.is(':enabled')) {
              if (Backdrop.ajax && Backdrop.ajax[buttonId]) {
                $button.trigger(Backdrop.ajax[buttonId].element_settings.event);
              }
              else {
                $button.click();
              }
            }
          }
        }
      });
    });
  }
};

/**
 * Remove icon class from elements that are themed as buttons or dropbuttons.
 */
Backdrop.behaviors.viewsRemoveIconClass = {
  attach: function (context) {
    $(context).find('.dropbutton').once('dropbutton-icon', function () {
      $(this).find('.icon').removeClass('icon');
    });
  }
};

/**
 * Change "Expose filter" buttons into checkboxes.
 */
Backdrop.behaviors.viewsUiCheckboxify = {
  attach: function (context, settings) {
    var $buttons = $('#edit-options-expose-button-button, #edit-options-group-button-button').once('views-ui-checkboxify');
    var length = $buttons.length;
    var i;
    for (i = 0; i < length; i++) {
      new Backdrop.viewsUi.Checkboxifier($buttons[i]);
    }
  }
};

/**
 * Change the default widget to select the default group according to the
 * selected widget for the exposed group.
 */
Backdrop.behaviors.viewsUiChangeDefaultWidget = {
  attach: function () {
    function changeDefaultWidget (event) {
      if ($(event.target).prop('checked')) {
        $('input.default-radios').hide();
        $('td.any-default-radios-row').parent().hide();
        $('input.default-checkboxes').show();
      }
      else {
        $('input.default-checkboxes').hide();
        $('td.any-default-radios-row').parent().show();
        $('input.default-radios').show();
      }
    }
    // Update on widget change.
    $('input[name="options[group_info][multiple]"]')
      .on('change', changeDefaultWidget)
      // Update the first time the form is rendered.
      .trigger('change');
  }
};

/**
 * Attaches an expose filter button to a checkbox that triggers its click event.
 *
 * @param button
 *   The DOM object representing the button to be checkboxified.
 */
Backdrop.viewsUi.Checkboxifier = function (button) {
  this.$button = $(button);
  this.$parent = this.$button.parent('div.views-expose, div.views-grouped');
  this.$input = this.$parent.find('input:checkbox, input:radio');
  // Hide the button and its description.
  this.$button.hide();
  this.$parent.find('.exposed-description, .grouped-description').hide();

  this.$input.click($.proxy(this, 'clickHandler'));
};

/**
 * When the checkbox is checked or unchecked, simulate a button press.
 */
Backdrop.viewsUi.Checkboxifier.prototype.clickHandler = function (e) {
  this.$button.mousedown();
  this.$button.submit();
};

/**
 * Change the Apply button text based upon the override select state.
 */
Backdrop.behaviors.viewsUiOverrideSelect = {
  attach: function (context) {
    $(context).find('#edit-override-dropdown').once('views-ui-override-button-text', function () {
      var $select = $(this);
      var $submit = $select.closest('form').find('.form-submit[value="' + Backdrop.t('Apply') + '"]');
      var old_value = $submit.val();

      $submit.once('views-ui-override-button-text')
        .on('mouseup', function () {
            $submit.val(old_value);
          return true;
        });

      $select.on('change', function () {
        if ($select.val() === 'default') {
          $submit.val(Backdrop.t('Apply (all displays)'));
        }
        else if ($select.val() === 'default_revert') {
          $submit.val(Backdrop.t('Revert to default'));
        }
        else {
          $submit.val(Backdrop.t('Apply (this display)'));
        }

        // Update dialog copies of buttons if present.
        $submit.closest('.ui-dialog-content').trigger('dialogButtonsChange');
      }).trigger('change');
    });
  }
};

Backdrop.behaviors.viewsModalContent = {
  attach: function (context) {
    $('body').once('viewsDialog').on('dialogContentResize.viewsDialog', '.ui-dialog-content', Backdrop.behaviors.viewsModalContent.handleDialogResize);
    // When expanding details, make sure the modal is resized.
    $(context).find('.scroll').once('detailsUpdate').on('click', 'summary', function (e) {
      $(e.currentTarget).trigger('dialogContentResize');
    });
  },
  detach: function (context, settings, trigger) {
    if (trigger === 'unload') {
      $('body').removeOnce('viewsDialog').off('.viewsDialog');
    }
  },
  handleDialogResize: function (e) {
    var $modal = $(e.currentTarget);
    var $viewsOverride = $modal.find('[data-views-offset]');
    var $scroll = $modal.find('[data-backdrop-views-scroll]');
    var offset = 0;
    var modalHeight;
    if ($viewsOverride.length && $scroll.length) {
      // Add a class to do some styles adjustments.
      $modal.closest('.views-ui-dialog').addClass('views-ui-dialog-scroll');
      // Let scroll element take all the height available.
      $scroll.css({ overflow: 'visible', height: 'auto' });
      modalHeight = $modal.height();
      $viewsOverride.each(function () { offset += $(this).outerHeight(); });

      // Take internal padding into account.
      var scrollOffset = $scroll.outerHeight() - $scroll.height();
      $scroll.height(modalHeight - offset - scrollOffset);
      // Reset scrolling properties.
      $modal.css('overflow', 'hidden');
      $scroll.css('overflow', 'auto');
    }
  }
};

})(jQuery);
