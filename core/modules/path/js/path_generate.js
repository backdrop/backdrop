/**
 * @file
 * Manages checkboxes for Path generation page.
 */
(function ($) {
Backdrop.behaviors.path_generate = {
  attach: function (context) {
	  $("input.path-reset").prop('disabled', true);

    $("select[name='operations[operation]']").change(function(value){
      $delete = ($(this).val()=='delete');
      $("table#path-bulk-alias").toggleClass('delete', $delete);
    });

    // When you select an entity checkbox (like "content"), select the 
    // checkboxes for all the subtypes
	  $("input.path-base").change(function () {
      var type = $(this).attr('data-path-type');
      $("input.path-type[data-path-type='"+type+"']").prop('checked', this.checked);
      updateReset($(this).prop('checked'), type);
	  });

		// If you uncheck a subtype checkbox, uncheck the entity checkbox.
	  $("input.path-type").change(function () {
      var type = $(this).attr('data-path-type');
      var $base = $("input.path-base[data-path-type='"+type+"']");
			if($(this).is(":checked") == false) {
        $base.prop('checked', false);
  		}
      else {
        unchecked = $("input.path-type[data-path-type='"+type+"']:checkbox:not(:checked)");
        if(unchecked.length < 1) {
          $base.prop('checked', true);
        }
      }
      updateReset($(this).prop('checked'), type, $(this).attr('data-path-name'));
      updateReset($base.prop('checked'), type, $base.attr('data-path-name'));
    });

    // When you select an entity reset checkbox (like "content"), select the 
    // reset checkboxes for all the subtypes.
	  $("input.path-reset-base").change(function () {
      var type = $(this).attr('data-path-type');
		  $("input.path-reset[data-path-type='"+type+"']").prop('checked', this.checked);
	  });

    // Find all <th> with class select-all, and insert the check all checkbox.
    var strings = { 'selectAll': Backdrop.t('Select all rows in this table'), 'selectNone': Backdrop.t('Deselect all rows in this table') };
    $('th.path-th-alias').prepend($('<input type="checkbox" class="form-checkbox" />').attr('title', strings.selectAll)).click(function (event) {
      if ($(event.target).is('input[type="checkbox"]')) {
        $("input.path-type, input.path-base").each(function () {
          this.checked = event.target.checked;
          updateReset($(this).prop('checked'), $(this).attr('data-path-type'), $(this).attr('data-path-name'));
        });
      }
    });

  /**
     * Disables the reset_alias checkboxes if path alias checkbox is unchecked.
     */
    function updateReset(checked, type, name) {
      if(name) {
        $reset = $("input.path-reset[data-path-name='"+name+"']"); 
      }
      else {
        $reset = $("input.path-reset[data-path-type='"+type+"']"); 
      }
      if(checked) {
   		  $($reset).prop('disabled', false);
      }
      else {
   		  $($reset).prop('checked', false).prop('disabled', true);
      }
    }


  }
};

})(jQuery);
