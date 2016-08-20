/**
 * @file
 * Manages checkboxes for Path generation page.
 */
(function ($) {
Backdrop.behaviors.path_generate = {
  attach: function (context) {
	  $("input.path-reset").prop('disabled', true);

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
			if($(this).is(":checked") == false) {
        $("input.path-base[data-path-type='"+type+"']").prop('checked', false);
  		}
      updateReset($(this).prop('checked'), type, $(this).attr('data-path-name'));
    });

    // When you select an entity reset checkbox (like "content"), select the 
    // reset checkboxes for all the subtypes.
	  $("input.path-reset-base").change(function () {
      var type = $(this).attr('data-path-type');
		  $("input.path-reset[data-path-type='"+type+"']").prop('checked', this.checked);
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
