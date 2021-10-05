(function ($) {

/**
 * Collapses table rows followed by group rows on the test listing page.
 */
Backdrop.behaviors.simpleTestGroupCollapse = {
  attach: function (context, settings) {
      $(context).find('.simpletest-group').once('simpletest-group-collapse', function () {
        var $group = $(this);
        var $image = $group.find('.simpletest-image');
        $image
          .html(Backdrop.settings.simpleTest.images[0])
          .on('click', function () {
            var $tests = $group.nextUntil('.simpletest-group');
            var expand = !$group.hasClass('expanded');
            $group.toggleClass('expanded', expand);
            $tests.toggleClass('js-hide', !expand);
            $image.html(Backdrop.settings.simpleTest.images[+expand]);
          });
    });
  }
};

/**
 * Toggles test checkboxes to match the group checkbox.
 */
Backdrop.behaviors.simpleTestSelectAll = {
  attach: function (context, settings) {
    $(context).find('.simpletest-group').once('simpletest-group-select-all', function () {
      var $group = $(this);
      var $cell = $group.find('.simpletest-group-select-all');
      var $groupCheckbox = $('<input data-tableselect-id="'+ $cell.attr('id') + '" type="checkbox" id="' + $cell.attr('id') + '-group-select-all" class="form-checkbox" />');
      var $testCheckboxes = $group.nextUntil('.simpletest-group').find('input[type=checkbox]');
      $cell.append($groupCheckbox);
      $testCheckboxes.each(function(){
        $(this).attr('data-tableselect-id', $cell.attr('id'));
      });

      // Toggle the test checkboxes when the group checkbox is toggled.
      $groupCheckbox.on('change', function () {
        var checked = $(this).prop('checked');
        $testCheckboxes.prop('checked', checked);
      });

      // Update the group checkbox when a test checkbox is toggled.
      function updateGroupCheckbox() {
        var allChecked = true;
        $testCheckboxes.each(function () {
          if (!$(this).prop('checked')) {
            allChecked = false;
            return false;
          }
        });
        $groupCheckbox.prop('checked', allChecked);
      }

      $testCheckboxes.on('change', updateGroupCheckbox);
    });
  }
};

/**
 * Filters test rows by search term.
 */
Backdrop.behaviors.simpleTestFilter = {
  attach: function (context, settings) {
    var $input = $('input#edit-search').once('simpletest-table-search');
    var $form = $('#simpletest-form-table');
    var $formData = $form.html();
    var $rows, zebraClass;
    var zebraCounter = 0;

    function filterTestList() {
      var query = $input.val().toLowerCase();

      function showTestItem(index, row) {
        var $row = $(row);
        var $sources = $row.find('label, .description');
        var textMatch = $sources.text().toLowerCase().indexOf(query) !== -1;
        var $match = $row.closest('tr:not(.simpletest-group)');
        if ($input.val().length == 0) {
          $row.removeClass('test-even');
          if ($row.hasClass('simpletest-group')) {
            $row.removeClass('js-hide');
          }
          else {
            $row.addClass('js-hide');
          }
        }
        else {
          if ($row.hasClass('simpletest-group')) {
            $row.addClass('js-hide');
          }
          else if (textMatch) {
            $match.removeClass('js-hide');
            stripeRow($match);
          }
          else {
            $row.addClass('js-hide');
          }
        }
      }

      // Reset the zebra striping for consistent even/odd classes.
      zebraCounter = 0;
      $rows.each(showTestItem);
    }

    function stripeRow($match) {
      zebraClass = (zebraCounter % 2) ? '' : 'test-even';
      $match.removeClass('test-even');
      $match.addClass(zebraClass);
      zebraCounter++;
    }

    if ($form.length && $input.length) {
      $rows = $form.find('tbody tr');
      $input.focus().on('keyup', filterTestList);
    }
  }
}

})(jQuery);
