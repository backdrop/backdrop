/**
 * @file
 * Manages checkboxes for Path generation page.
 */

(function ($) {
Backdrop.behaviors.path_generate = {
  attach: function (context) {

    // Hide the Update existing checkboxes if deleting aliases
    $("select[name='operations[operation]']").change(function(value){
      $delete = ($(this).val()=='delete');
      $("table#path-bulk-alias").toggleClass('delete', $delete);
    });

    // When you select an entity checkbox (like "content"), select the 
    // checkboxes for all the subtypes
    $("input.path-base").change(function () {
      var type = $(this).attr('data-path-type');
      $("input.path-type[data-path-type='"+type+"']").prop('checked', this.checked);
    });

    $("input.path-type").change(function () {
      var type = $(this).attr('data-path-type');
      var $base = $("input.path-base[data-path-type='"+type+"']");
      // If you uncheck a subtype checkbox, uncheck the entity checkbox.
      if($(this).is(":checked") == false) {
        $base.prop('checked', false);
        $("input.select-all-alias").prop('checked', false);
      }
      // If all subtype checkboxes checked, check the base checkbox.
      else {
        unchecked = $("input.path-type[data-path-type='"+type+"']:checkbox:not(:checked)");
        if(unchecked.length < 1) {
          $base.prop('checked', true);
        }
      }
    });

    // When you select an entity reset checkbox (like "content"), select the 
    // reset checkboxes for all the subtypes.
    $("input.path-reset-base").change(function () {
      var type = $(this).attr('data-path-type');
      $("input.path-reset[data-path-type='"+type+"']").prop('checked', this.checked);
    });

    $("input.path-reset").change(function () {
      var type = $(this).attr('data-path-type');
      var $base = $("input.path-reset-base[data-path-type='"+type+"']");
      // If you uncheck a subtype checkbox, uncheck the entity checkbox.
      if($(this).is(":checked") == false) {
        $base.prop('checked', false);
      }
      // If all subtype checkboxes checked, check the base checkbox.
      else {
        unchecked = $("input.path-reset[data-path-type='"+type+"']:checkbox:not(:checked)");
        if(unchecked.length < 2) {
          $base.prop('checked', true);
        }
      }
    });

    // Add check all checkboxes in the table head row.
    var strings = { 'selectAll': Backdrop.t('Select all rows in this table'), 'selectNone': Backdrop.t('Deselect all rows in this table') };
    $('th.path-th-alias').prepend($('<input type="checkbox" class="select-all-alias form-checkbox" />').attr('title', strings.selectAll)).click(function (event) {
      if ($(event.target).is('input[type="checkbox"]')) {
        $("input.path-type, input.path-base").each(function () {
          this.checked = event.target.checked;
        });
      }
    });
    $('th.path-th-delete').prepend($('<input type="checkbox" class="select-all-delete form-checkbox" />').attr('title', strings.selectAll)).click(function (event) {
      if ($(event.target).is('input[type="checkbox"]')) {
        $("input.path-reset").each(function () {
          this.checked = event.target.checked;
        });
      }
    });

    // Uncheck the "select all" checkboxes if not all checkboxes schecked.
    $('input[type="checkbox"]').change(function () {
      uncheckedAll = ($("input.path-type:checkbox:not(:checked)").length)+($("input.path-base:checkbox:not(:checked)").length);
      $("input.select-all-alias").prop('checked', uncheckedAll<1);
      uncheckedAllDelete = $("input.path-reset:checkbox:not(:checked)").length;
      $("input.select-all-delete").prop('checked', uncheckedAllDelete<1);
    });

  }
};
})(jQuery);
