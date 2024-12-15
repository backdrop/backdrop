(function ($) {

Backdrop.behaviors.menuAdminFieldsetSummaries = {
  attach: function (context) {
    var $context = $(context);
    $context.find('#edit-menu').backdropSetSummary(function () {
      var $enabledMenus = $context.find('.form-item-menu-options input:checked');
      if ($enabledMenus.length) {
        var vals = [];
        $enabledMenus.each(function(n, checkbox) {
          vals.push($(checkbox).siblings('label').text());
        });
        return vals.join(', ');
      }
      else {
        return Backdrop.t('Disabled');
      }
    });
  }
};

Backdrop.behaviors.menuEditItemParents = {
  attach: function (context, settings) {
    $('.form-item-parent-menu', context).once('form-item-parent-menu', function () {
      // Get the menu parent options from settings.
      var menuOptions = settings.menu_edit_item_parents;
      // Get the current menu name from settings.
      var menuName = settings.menu_edit_item_menu;
      // Always move the menu selector before the menu parent selector.
      $('.form-item-parent-menu').insertBefore('#menu-parent-select-wrapper');
      // On load set the current menu as default in the menu select.
      Backdrop.menu_edit_update_parent_list(menuName);
      // Ensure that the menu parent select list is filtered again when any ajax
      // process runs.
      $(document).on('ajaxComplete', function(event, xhr, settings) {
        if (settings.url == Backdrop.settings.basePath + 'system/ajax') {
          var selected = $('[data-menu-parent] :selected').val().split(':')[0];
          Backdrop.menu_edit_update_parent_list(selected);
        }
      });
      // On changing the menu select, update the menu parent select.
      var sel = $('.form-item-parent-menu select');
      sel.on('change', function () {
        Backdrop.menu_edit_update_parent_list(this.value);
      });
    });
  }
}

/**
 * Function to set the options of the menu parent item dropdown.
 */
Backdrop.menu_edit_update_parent_list = function (value) {
  var values = [value];

  var url = Backdrop.settings.basePath + 'admin/structure/menu/parents';
  $.ajax({
    url: location.protocol + '//' + location.host + url,
    type: 'POST',
    data: {'menus[]' : values},
    dataType: 'json',
    success: function (options) {
      // Save key of last selected element.
      var selected = $('[data-menu-parent] :selected').val();
      // Remove all existing options from dropdown.
      var selectForm = $('[data-menu-parent]');
      selectForm.children().remove();
      // Add new options to dropdown.
      $.each(options, function(index, value) {
        $('[data-menu-parent]').append(
          $('<option ' + (index == selected ? ' selected="selected"' : '') + '></option>').val(index).text(value)
        );
      });
    }
  });
};

Backdrop.behaviors.menuChangeParentItems = {
  attach: function (context, settings) {
    $('fieldset#edit-menu input').each(function () {
      $(this).on('change', function () {
        // Update list of available parent menu items.
        Backdrop.menu_update_parent_list();
      });
    });
  }
};

/**
 * Function to set the options of the menu parent item dropdown.
 */
Backdrop.menu_update_parent_list = function () {
  var values = [];

  $('input:checked', $('fieldset#edit-menu')).each(function () {
    // Get the names of all checked menus.
    values.push(Backdrop.checkPlain($(this).val().trim()));
  });

  var url = Backdrop.settings.basePath + 'admin/structure/menu/parents';
  $.ajax({
    url: location.protocol + '//' + location.host + url,
    type: 'POST',
    data: {'menus[]' : values},
    dataType: 'json',
    success: function (options) {
      // Save key of last selected element.
      var selected = $('fieldset#edit-menu #edit-menu-parent :selected').val();
      // Remove all existing options from dropdown.
      var selectForm = $('fieldset#edit-menu #edit-menu-parent');
      selectForm.children().remove();
      // Add new options to dropdown.
      jQuery.each(options, function(index, value) {
        $('fieldset#edit-menu #edit-menu-parent').append(
          $('<option ' + (index == selected ? ' selected="selected"' : '') + '></option>').val(index).text(value)
        );
      });
      // Hide Default parent item form if empty.
      var menuParent = selectForm.parents('.form-item-menu-parent');
      if (selectForm.children().length === 0) {
        menuParent.hide();
      }
      else {
        menuParent.show();
      }
    }
  });
};

})(jQuery);
