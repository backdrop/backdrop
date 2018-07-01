(function ($) {

/**
 * Filters the module list table by a text input search string, tags, and
 * module source (core, contrib or custom).
 */
Backdrop.behaviors.moduleFilter = {
  attach: function(context, settings) {
    var $input = $('input.table-filter-text').once('table-filter-text');
    var $form = $('#system-modules');
    var $allRows, $rows;

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

    // Filter the list of modules by provided search string.
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
        var tagSource = $row.find('.module-tags').text().toLowerCase();
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

        // If the row contains the string show the row.
        $row.closest('tr').toggle((rowMatch) && tagMatch && sourceMatch);
      }

      // Filter only if the length of the search query is at least 2 characters.
      if (query.length >= 2 || tag || !core || !contrib || !custom) {
        $('#edit-target').on('change', function() {
          $rows.each(showModuleRow);
        });

        $rows.each(showModuleRow);

        if ($rows.filter(':visible').length === 0) {
          if ($('.filter-empty').length === 0) {
            $('#edit-filter').append('<p class="filter-empty">' + Backdrop.t('There were no results.') + '</p>');
          }
        }
        else {
          $('.filter-empty').remove();
        }
      }
      else {
        $allRows.show();
        $('.filter-empty').remove();
      }
    }

    if ($form.length) {
      $allRows = $form.find('tr');
      $rows = $form.find('tbody tr');

      // @todo Use autofocus attribute when possible.
      $input.focus().on('keyup', filterModuleList);
      $('select[data-filter-tags]').change(filterModuleList);
      $('input[data-filter-source]').change(filterModuleList);
      $input.triggerHandler('keyup');
    }
  }
};

})(jQuery);
