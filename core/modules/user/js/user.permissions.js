(function ($) {

/**
 * Ticks and disables checkboxes for permissions inherited from the
 * authenticated role.
 */
Backdrop.behaviors.permissions = {
  attach: function (context, settings) {
    var self = this;
    var authPermissions = Backdrop.settings.userPermissions.authenticatedPermissions;
    var userRole = Backdrop.settings.userPermissions.userRole;

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

      // Create dummy checkboxes for display purposes. We use dummy checkboxes
      // instead of reusing the actual permission checkboxes here because we
      // don't want to alter the submitted form. If we'd automatically tick
      // existing checkboxes, the user.role.* config files would be polluted
      // with redundant permissions.
      var $dummy = $('<input type="checkbox" class="form-checkbox dummy-checkbox" disabled="disabled" checked="checked" />')
        .attr('title', Backdrop.t("This permission is inherited from the authenticated user role."));

      // If editing permissions for an individual role (i.e. via the form at the
      // admin/config/people/permissions/%role_name path).
      if (authPermissions && userRole) {
        // Find all checkboxes for permissions that are already granted to the
        // authenticated role.
        authPermissions.forEach(function(permission) {
          var checkboxID = '#edit-' + userRole + '-' + permission.replace(/[\s_]+/g, '-');
          // Hide the actual checkbox, and show a dummy in its place.
          $table.find('input[type=checkbox]' + checkboxID).addClass('real-checkbox').hide().after($dummy.clone());
        });
      }
      // If editing permissions for all roles (i.e. via the form at the
      // admin/config/people/permissions path).
      else {
        var $hidden_dummy = $dummy.hide();
        // Append hidden dummy checkboxes to all permission checkboxes for all
        // roles excluding the anonymous and authenticated roles.
        $('input[type=checkbox]', this).not('.role-authenticated, .role-anonymous').addClass('real-checkbox').each(function () {
          $hidden_dummy.clone().insertAfter(this);
        });

        // Initialize the authenticated user checkbox.
        $table.on('click.permissions', 'input[type=checkbox].role-authenticated', function(e) {
          self.toggle.apply(e.target);
        });
        $table.find('input[type=checkbox].role-authenticated').each(self.toggle);
      }

      // Re-insert the table into the DOM.
      $ancestor[method]($table);
    });
  },

  /**
   * Toggles display of dummy/actual checkboxes for a specific permission.
   *
   * If the checkbox for the authenticated role is ticked for a specific
   * permission, the respective dummy (ticked and disabled) checkboxes for the
   * other roles are shown, and the actual permission checkboxes are hidden.
   * Otherwise, the actual checkboxes are shown and dummy checkboxes hidden.
   */
  toggle: function () {
    var authCheckbox = this, $row = $(this).closest('tr');
    // jQuery performs too many layout calculations for .hide() and .show(),
    // leading to a major page rendering lag on sites with many roles and
    // permissions. Therefore, we toggle visibility directly.
    $row.find('.real-checkbox').each(function () {
      this.style.display = (authCheckbox.checked ? 'none' : '');
    });
    $row.find('.dummy-checkbox').each(function () {
      this.style.display = (authCheckbox.checked ? '' : 'none');
    });
  }
};

})(jQuery);
