// $Id$

Drupal.viewsUi = {};


Drupal.behaviors.viewsUiAddView = {};

Drupal.behaviors.viewsUiAddView.attach = function (context, settings) {
  var $ = jQuery;
  // Prepopulate the page title, block title, and menu link fields with the
  // view name.
  var $fields = $('#edit-page-title, #edit-block-title, #edit-page-link-properties-title');
  new Drupal.viewsUi.FormFieldFiller($fields);

  // Prepopulate the path field with a URLified version of the view name.
  var $pathField = $('#edit-page-path');
  // Allow only numbers, letters, and dashes in the path.
  var exclude = new RegExp('[^a-z0-9\\-]+', 'g');
  var replace = '-';
  new Drupal.viewsUi.FormFieldFiller($pathField, exclude, replace);
};

/**
 * Constructor for the Drupal.viewsUi.FormFieldFiller object.
 *
 * Prepopulates a form field based on the view name.
 *
 * @param $target
 *   A jQuery object representing the form field to prepopulate.
 * @param exclude
 *   Optional. A regular expression representing characters to exclude from the
 *   target field.
 * @param replace
 *   Optional. A string to use as the replacement value for disallowed
 *   characters.
 */
Drupal.viewsUi.FormFieldFiller = function ($target, exclude, replace) {
  var $ = jQuery;
  this.source = $('#edit-description');
  this.target = $target;
  this.exclude = exclude || false;
  this.replace = replace || '';
  this.initialize();
  // Object constructor; no return value.
};

/**
 * Bind the form-filling behavior.
 */
Drupal.viewsUi.FormFieldFiller.prototype.initialize = function () {
  var $ = jQuery;
  // Populate the form field when the source changes.
  this.source.bind('keyup.viewsUi change.viewsUi', $.proxy(this, 'populate'));
  // Quit populating the field as soon as it gets focus.
  this.target.bind('focus.viewsUi', $.proxy(this, 'unbind'));
};

/**
 * Get the source form field value as altered by the regular expression.
 */
Drupal.viewsUi.FormFieldFiller.prototype.getTransliterated = function () {
  var from = this.source.val();
  if (this.exclude) {
    from = from.toLowerCase().replace(this.exclude, this.replace);
  }
  return from;
};

/**
 * Populate the target form field with the altered source field value.
 */
Drupal.viewsUi.FormFieldFiller.prototype.populate = function () {
  var transliterated = this.getTransliterated();
  this.target.val(transliterated);
};

/**
 * Stop prepopulating the form field.
 */
Drupal.viewsUi.FormFieldFiller.prototype.unbind = function () {
  this.source.unbind('.viewsUi');
  this.target.unbind('.viewsUi');
};



Drupal.behaviors.viewsUiEditView = {};

Drupal.behaviors.viewsUiEditView.attach = function (context, settings) {
  //jQuery('.views-displays').once('views-ui-edit-view').tabs();
};

/**
 * The input field items that add displays must be rendered as <input> elements.
 * The following behavior detaches the <input> elements from the DOM, wraps them
 * in an unordered list, then appends them to the list of tabs.
 */
Drupal.behaviors.viewsUiRenderAddViewButton = {};

Drupal.behaviors.viewsUiRenderAddViewButton.attach = function (context, settings) {
  var $ = jQuery;
  // Build the add display menu and pull the display input buttons into it.
  var $menu = $('#views-ui-edit-form .secondary', context).once('views-ui-render-add-view-button-processed');
  if (!$menu.length) {
    return;
  }
  var $addDisplayDropdown = $('<li class="add"><a href="#"><span class="icon icon-add"></span>Add</a><ul class="action-list" style="display:none;"></ul></li>');
  var $displayButtons = $menu.nextAll('input.add-display').detach();
  $displayButtons.appendTo($addDisplayDropdown.find('.action-list')).wrap('<li>')
    .parent().first().addClass('first').end().last().addClass('last');
  $addDisplayDropdown.appendTo($menu);
  
  // Add the click handler for the add display button
  $('li.add > a', $menu).bind('click', function (event) {
    event.preventDefault();
    var $trigger = $(this);
    Drupal.behaviors.viewsUiRenderAddViewButton.toggleMenu($trigger);
  });
  // Add a mouseleave handler to close the dropdown when the user mouses
  // away from the item. We use mouseleave instead of mouseout because
  // the user is going to trigger mouseout when she moves from the trigger
  // link to the sub menu items.
  // We use the live binder because the open class on this item will be 
  // toggled on and off and we want the handler to take effect in the cases
  // that the class is present, but not when it isn't.
  $('li.add', $menu).live('mouseleave', function (event) {
    var $this = $(this);
    var $trigger = $this.children('a[href="#"]');
    if ($this.children('.action-list').is(':visible')) {
      Drupal.behaviors.viewsUiRenderAddViewButton.toggleMenu($trigger);
    }
  });
};

/**
 * @note [@jessebeach] I feel like the following should be a more generic function and
 * not written specifically for this UI, but I'm not sure where to put it.
 */
Drupal.behaviors.viewsUiRenderAddViewButton.toggleMenu = function ($trigger) {
  $trigger.parent().toggleClass('open');
  $trigger.next().slideToggle('fast');
}


Drupal.behaviors.viewsUiSearchOptions = {};

Drupal.behaviors.viewsUiSearchOptions.attach = function (context) {
  var $ = jQuery;
  // The add item form may have an id of views-ui-add-item-form--n.
  var $form = $(context).find('form[id^="views-ui-add-item-form"]').first();
  // Make sure we don't add more than one event handler to the same form.
  $form = $form.once('views-ui-filter-options');
  if ($form.length) {
    new Drupal.viewsUi.OptionsSearch($form);
  }
};

/**
 * Constructor for the viewsUi.OptionsSearch object.
 *
 * The OptionsSearch object filters the available options on a form according
 * to the user's search term. Typing in "taxonomy" will show only those options
 * containing "taxonomy" in their label.
 */
Drupal.viewsUi.OptionsSearch = function ($form) {
  this.$form = $form;
  // Add a keyup handler to the search box.
  this.$searchBox = this.$form.find('#edit-options-search');
  this.$searchBox.keyup(jQuery.proxy(this.handleKeyup, this));
  // Get a list of option labels and their corresponding divs and maintain it
  // in memory, so we have as little overhead as possible at keyup time.
  this.options = this.getOptions(this.$form.find('.filterable-option'));
};

/**
 * Assemble a list of all the filterable options on the form.
 *
 * @param $allOptions
 *   A jQuery object representing the rows of filterable options to be
 *   shown and hidden depending on the user's search terms.
 */
Drupal.viewsUi.OptionsSearch.prototype.getOptions = function ($allOptions) {
  var $ = jQuery;
  var i, $label, $option;
  var options = [];
  var length = $allOptions.length;
  for (i = 0; i < length; i++) {
    $option = $($allOptions[i]);
    $label = $option.find('label');
    options[i] = {
      // Search on the lowercase version of the label text.
      'labelText': $label.text().toLowerCase(),
      // Maintain a reference to the jQuery object for each row, so we don't
      // have to create a new object inside the performance-sensitive keyup
      // handler.
      '$div': $option
    }
  }
  return options;
};

/**
 * Keyup handler for the search box that hides or shows the relevant options.
 */
Drupal.viewsUi.OptionsSearch.prototype.handleKeyup = function (event) {
  var found, i, j, option, search, words, wordsLength, zebraClass, zebraCounter;

  // Determine the user's search query. The label text has been converted to
  // lowercase.
  search = this.$searchBox.val().toLowerCase();
  words = search.split(' ');
  wordsLength = words.length;

  // Start the counter for restriping rows.
  zebraCounter = 0;

  // Search through the labels in the form for matching text.
  var length = this.options.length;
  for (i = 0; i < length; i++) {
    // Use a local variable for the option being searched, for performance.
    option = this.options[i];
    found = true;
    // Each word in the search string has to match the label in order for the
    // label to be shown.
    for (j = 0; j < wordsLength; j++) {
      if (option.labelText.indexOf(words[j]) === -1) {
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
};
