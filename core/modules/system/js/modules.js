(function ($) {

/**
 * Filters the module list table by a text input search string.
 *
 * Additionally accounts for multiple tables being wrapped in "package" fieldset
 * elements.
 */
Backdrop.behaviors.moduleFilter = {
  attach: function(context, settings) {
    var $input = $('input.table-filter-text').once('table-filter-text');
    var $form = $('#system-modules');
    var $rowsAndFieldsets, $rows, $fieldsets;

    // Hide the module requirements.
    $form.find('.requirements').hide();

    // Toggle the requirements info.
    $('a.requirements-toggle').click(function(e) {
      var $requirements = $(this).closest('td').find('.requirements').toggle();
      if ($requirements.is(':visible')) {
        $(this).text(Backdrop.t('less'));
      }
      else {
        $(this).text(Backdrop.t('more'));
      }
      e.preventDefault();
      e.stopPropagation();
    });

    // Hide the package <fieldset> if it doesn't have any visible rows within.
    function hidePackageFieldset(index, element) {
      var $fieldset = $(element);
      var $visibleRows = $fieldset.find('table:not(.sticky-header)').find('tbody tr:visible');
      $fieldset.toggle($visibleRows.length > 0);
    }

    // Fliter the list of modules by provided search string.
    function filterModuleList() {
      var query = $input.val().toLowerCase();
      var tag = $('select[data-filter-tags]').val();
      var core = $('input[data-filter-source="core"]').is(":checked");
      var contrib = $('input[data-filter-source="contrib"]').is(":checked");
      var custom = $('input[data-filter-source="custom"]').is(":checked");

      function showModuleRow(index, row) {
        var $row = $(row);
        var $sources = $row.find('.table-filter-text-source');
        var rowMatch = $sources.text().toLowerCase().indexOf(query) !== -1;
        var $fieldsetTitle = $row.closest('fieldset').find('legend:first');
        var tagSource = $row.find('.module-tags').text().toLowerCase();

        // Finding the fieldset title and filtering it can be expensive and
        // repetitive to do for every row, so save the filtered title as data on
        // the DOM element.
        var filterTitle;
        if (!$fieldsetTitle.data('filterTitle')) {
          // Don't include hidden DOM elements such as the show/hide label.
          filterTitle = $fieldsetTitle.clone().find('.element-invisible').remove().end().text().toLowerCase();
          $fieldsetTitle.data('filterTitle', filterTitle);
        }
        else {
          filterTitle = $fieldsetTitle.data('filterTitle');
        }
        // Compare the search query to the fieldset title.
        var fieldsetTitleMatch = filterTitle.indexOf(query) !== -1;
        // Compare the requested tag to each row's tags.
        var tagMatch = tagSource.indexOf(tag) !== -1;

        // Check the module souce and show only if matched.
        var sourceMatch = false;
        if ($row.hasClass('core') && core ) {
          sourceMatch = true;
        }
        if ($row.hasClass('contrib') && contrib) {
          sourceMatch = true;
        }
        if ($row.hasClass('custom') && custom) {
          sourceMatch = true;
        }

        // If the row contains the string or the fieldset does, show the row.
        $row.closest('tr').toggle((rowMatch || fieldsetTitleMatch) && tagMatch && sourceMatch);
      }

      // Filter only if the length of the search query is at least 2 characters.
      if (query.length >= 2 || tag || !core || !contrib || !custom) {
        $('#edit-target').on('change', function() {
          $rows.each(showModuleRow);
        });

        $rows.each(showModuleRow);

        // We first show() all <fieldset>s to be able to use ':visible'.
        $fieldsets.show().each(hidePackageFieldset);

        if ($fieldsets.filter(':visible').length === 0) {
          if ($('.filter-empty').length === 0) {
            $('#edit-filter').append('<p class="filter-empty">' + Backdrop.t('There were no results.') + '</p>');
          }
        }
        else {
          $('.filter-empty').remove();
        }
      }
      else {
        $rowsAndFieldsets.show();
        $('.filter-empty').remove();
      }
    }

    if ($form.length) {
      $rowsAndFieldsets = $form.find('tr, fieldset');
      $rows = $form.find('tbody tr');
      $fieldsets = $form.find('fieldset');

      // @todo Use autofocus attribute when possible.
      $input.focus().on('keyup', filterModuleList);
      $('select[data-filter-tags]').change(filterModuleList);
      $('input[data-filter-source]').change(filterModuleList);
      $input.triggerHandler('keyup');
    }
  }
};

})(jQuery);
