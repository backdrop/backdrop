(function ($) {

/**
 * Filters the permission list table by a text input search string.
 */
Backdrop.behaviors.permissionsFilter = {
  attach: function(context, settings) {
    var $input = $('input.table-filter-text').once('table-filter-text');
    var $form = $('#user-admin-permissions');
    var $rows = $form.find('tbody tr');
    var $resetLink = $form.find('.search-reset');

    // Filter the list of modules by provided search string.
    function filterPermissionsList() {
      var query = $input.val().toLowerCase();

      function showPermissionRow(index, row) {
        var $row = $(row);

        // Check if the text source items of this row matches, including the
        // permission name and the description.
        var $sources = $row.find('.table-filter-text-source');
        var rowMatch = $sources.text().toLowerCase().indexOf(query) !== -1;

        // Check if the module providing this permission matches the string.
        var $moduleRow = $row.hasClass('module-row') ? $row : $row.prevAll('tr.module-row:first');
        var moduleNameMatch = $moduleRow.text().toLowerCase().indexOf(query) !== -1;

        // If the row or parent module contains the string, show the row.
        if (rowMatch || moduleNameMatch) {
          $row.show();
          // And ensure the corresponding module row is shown.
          $moduleRow.show();
        }
        else {
          $row.hide();
        }
      }

      // Filter only if the length of the search query is at least 2 characters.
      if (query.length >= 2) {
        $rows.each(showPermissionRow);
      }
      else {
        $rows.show();
        $('.filter-empty').remove();
      }
    }

    // Clear out the input field and search query when clicking the reset
    // button.
    function resetPermissionsList(e) {
      // Clear the input field.
      $input.val('').triggerHandler('keyup');
      e.preventDefault();

      // Clear the search query.
      var currentUrl = new URL(window.location);
      currentUrl.searchParams.delete('search');
      window.history.replaceState({}, '', currentUrl);
    }

    if ($form.length) {
      $input.trigger('focus').on('keyup', filterPermissionsList);
      $input.triggerHandler('keyup');
      $resetLink.on('click', resetPermissionsList);
    }
  }
};

/**
 * Shows checked and disabled checkboxes for inherited permissions.
 */
Backdrop.behaviors.permissions = {
  attach: function (context) {
    var self = this;
    $('table#permissions').once('permissions', function () {
      // On a site with many roles and permissions, this behavior initially has
      // to perform thousands of DOM manipulations to inject checkboxes and hide
      // them. By detaching the table from the DOM, all operations can be
      // performed without triggering internal layout and re-rendering processes
      // in the browser.
      var $table = $(this);
      var $ancestor, method;
      if ($table.prev().length) {
        $ancestor = $table.prev();
        method = 'after';
      }
      else {
        $ancestor = $table.parent();
        method = 'append';
      }
      $table.detach();

      // Create dummy checkboxes. We use dummy checkboxes instead of reusing
      // the existing checkboxes here because new checkboxes don't alter the
      // submitted form. If we'd automatically check existing checkboxes, the
      // permission table would be polluted with redundant entries. This
      // is deliberate, but desirable when we automatically check them.

      // Initialize the authenticated user checkbox.
      $table.on('click.permissions', 'input[type=checkbox].role-authenticated', function(e) {
        self.toggle.apply(e.target);
      });
      $table.find('input[type=checkbox].role-authenticated').each(self.toggle);

      // Re-insert the table into the DOM.
      $ancestor[method]($table);
    });
  },

  /**
   * Toggles visibility of all dummy/real checkboxes.
   *
   * If the "authenticated user" checkbox is checked for a specific permission,
   * the respective (checked and disabled) dummy checkboxes in the same row are
   * shown, and the respective real checkboxes are hidden.
   *
   * If the "authenticated user" checkbox is unchecked, the dummy checkboxes are
   * hidden, and the real checkboxes are shown.
   */
  toggle: function () {
    var authCheckbox = this, $row = $(this).closest('tr');
    // jQuery performs too many layout calculations for .hide() and .show(),
    // leading to a major page rendering lag on sites with many roles and
    // permissions. Therefore, we toggle visibility directly by adding/removing
    // the 'element-hidden' CSS class.
    $row.find('.real-checkbox').each(function () {
      if (authCheckbox.checked) {
        $(this).addClass('element-hidden');
      }
      else {
        $(this).removeClass('element-hidden');
      }
    });
    $row.find('.dummy-checkbox').each(function () {
      if (authCheckbox.checked) {
        $(this).removeClass('element-hidden');
      }
      else {
        $(this).addClass('element-hidden');
      }
    });
  }
};

/**
 * Shows/hides warning descriptions on click.
 */
Backdrop.behaviors.permissionWarnings = {
  attach: function(context, settings) {
    var $table = $('table#permissions');

    // Hide the warning description initially.
    $table.find('.permission-warning-description').hide();

    // Toggle the warning description.
    $table.on('click', 'a.warning-toggle', function(e) {
      var $description = $(this).closest('td').find('.permission-warning-description').toggle();
      if ($description.is(':visible')) {
        $(this).text(Backdrop.t('less'));
      }
      else {
        $(this).text(Backdrop.t('more'));
      }
      e.preventDefault();
      e.stopPropagation();
    });
  }
};

})(jQuery);
