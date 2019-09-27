(function ($) {

Backdrop.behaviors.contentTypes = {
  // Provide the vertical tab summaries.
  attach: function (context) {
    var $context = $(context);

    // Submission form settings.
    $context.find('fieldset#edit-submission').backdropSetSummary(function() {
      var vals = [];
      vals.push(Backdrop.checkPlain($context.find('input[name="title_label"]').val()) || Backdrop.t('Requires a title'));
      return vals.join(', ');
    });

    // Publishing settings.
    // This is implemented as a function, because it is needed once for the
    // initial page load, but then again if the "Schedule for later" checkbox
    // gets unticked.
    function updatePublishVtabSummary() {
      $context.find('#edit-workflow').backdropSetSummary(function () {
        var vals = [];
        var defaultStatus = $context.find('input[name="status_default"]:checked').parent().find('label').text();
        vals.push(Backdrop.checkPlain($.trim(defaultStatus)));
        if ($context.find('input[name="sticky_default"]:checked').length) {
          vals.push(Backdrop.t('Sticky'));
        }
        if ($context.find('input[name="promote_default"]:checked').length) {
          vals.push(Backdrop.t('Promoted'));
        }
        return vals.join(', ');
      });
    }

    // We call this once, to update the "Publishing settings" summary initially.
    updatePublishVtabSummary();

    // Uncheck the "Schedule for later" option if scheduling is disabled.
    $context.find('input[name="scheduling_enabled"]').once().on('change', function() {
      var $checkedStatusDefault = $context.find('input[name="status_default"]:checked');
      if ($checkedStatusDefault.val() === '2') {
        $checkedStatusDefault.prop('checked', false);
        $context.find('input[name="status_default"]:first').prop('checked', true);
        // If the previous selection for "Default status" was "Schedule for
        // later", then unticking the "Show option for scheduling" checkbox
        // would still show "Schedule for later" as the vtab summary - even
        // though the default has been switched to "Published". So we also need
        // to update the "Publishing settings" summary again.
        // See https://github.com/backdrop/backdrop-issues/issues/4098
        updatePublishVtabSummary();
      }
    });

    // Multilingual support.
    $context.find('#edit-multilingual').backdropSetSummary(function() {
      var vals = [];
      if ($context.find('input[name="language"]:checked').length) {
        vals.push(Backdrop.t('Enabled'));
      }
      else {
        vals.push(Backdrop.t('Disabled'));
      }
      return vals.join(', ');
    });

    // Permissions settings.
    // List any permission name that has at least one permission checked.
    $context.find('#edit-permissions').backdropSetSummary(function(context) {
      var permissionNames = [];
      var checkedColumns = [];
      var extraPermissions = 0;
      var columnOffset, $cell;

      // Find all checked boxes and save the column number.
      $(context).find('input:checked:visible').each(function () {
        $cell = $(this).closest('td');
        columnOffset = $cell.closest('tr').children('td').index($cell);
        if (checkedColumns.indexOf(columnOffset) === -1) {
          checkedColumns.push(columnOffset);
        }
      });
      // Sort by column number.
      checkedColumns.sort();

      // Replace the column number with permission names.
      for (var n = 0; n < checkedColumns.length; n++) {
        if (permissionNames.length < 3) {
          permissionNames.push($(context).find('th').eq(checkedColumns[n]).text());
        }
        else {
          extraPermissions++;
        }
      }
      // If 4 permissions only, add the last one.
      if (extraPermissions === 1) {
        permissionNames.push($(context).find('th').eq(checkedColumns[n - 1]).text());
      }
      // If more than 4, show the first 3 and then a count of others.
      else if (extraPermissions >= 2) {
        permissionNames.push(Backdrop.t('@count other roles', {'@count': extraPermissions}));
      }
      return permissionNames.length ? permissionNames.join(', ') : Backdrop.t('No permissions set');
    });

    // Path settings.
    $context.find('#edit-path').backdropSetSummary(function(context) {
      var vals = [];
      vals.push(Backdrop.checkPlain($(context).find('input[name="path_pattern"]').val()) || Backdrop.t('No URL alias pattern set'));
      return vals.join(', ');
    });

    // Focus the input#edit-path-pattern field when clicking on the token
    // browser on the /admin/structure/types/add pages (add a content type).
    $context.find('#edit-path .token-browser-link').once().on('click', function(){
      $('input#edit-path-pattern').focus();
    });

    // Revision settings.
    $context.find('#edit-revision').backdropSetSummary(function() {
      var vals = [];
      var revisionsOn = $context.find('input[name="revision_enabled"]:checked').length;
      var revisionsByDefault = $context.find('input[name="revision_default"]:checked').length;
      if (!revisionsOn && !revisionsByDefault) {
        vals.push(Backdrop.t('Disabled'));
      }
      else if (revisionsOn && revisionsByDefault) {
        vals.push(Backdrop.t('Created by default (optional)'));
      }
      else {
        if (revisionsOn) {
          vals.push(Backdrop.t('Optional'));
        }
        if (revisionsByDefault) {
          vals.push(Backdrop.t('Always created by default'));
        }
      }
      return vals.join(', ');
    });

    // Display settings.
    $context.find('#edit-display').backdropSetSummary(function(context) {
      var vals = [];
      $('input:checked', context).next('label').each(function() {
        vals.push(Backdrop.checkPlain($(this).text()).trim());
      });
      if (!$('#edit-node-submitted', context).is(':checked')) {
        vals.unshift(Backdrop.t("Don't display post information"));
      }
      return vals.join(', ');
    });

  }
};

})(jQuery);
