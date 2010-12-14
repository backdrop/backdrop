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
  // Build the add display menu, pull the display input buttons into it
  // and mpve them to the add display menu
  var $menu = $('.secondary', '#views-ui-edit-form', context).once('views-ui-render-add-view-button-processed');
  if (!$menu.length) {
    return;
  }
  var $addDisplayDropdown = $('<li class="add"><a href="#"><span class="icon icon-add"></span>Add</a><ul class="action-list" style="display:none;"></ul></li>');
  var $displayButtons = $menu.nextAll('[type="submit"]').detach();
  $displayButtons.appendTo($addDisplayDropdown.find('.action-list')).wrap('<li>');
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
