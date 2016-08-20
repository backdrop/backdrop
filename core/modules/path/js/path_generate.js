/**
 * @file
 * Manages checkboxes for Path generation page.
 */
(function ($) {
Backdrop.behaviors.path_generate = {
  attach: function (context) {
	  $("input.path-reset-type, input.path-reset-base").prop('disabled', true);

	  $("input.path-base").change(function () {
    console.log(getNameParts($(this).attr('name')));
      nameParts = getNameParts($(this).attr('name'));
      $('table#permissions').find("input.path-"+nameParts[0]).prop('checked', this.checked);
      updateReset($(this).prop('checked'), nameParts);
	  });

		// If you uncheck a child checkbox, uncheck the base checkbox too.
	  $("input.path-type").change(function () {
			if($(this).is(":checked") == false) {
        nameParts = getNameParts($(this).attr('name'));
        name = 'update['+nameParts[0]+'][base]';
        $('table#permissions').find("input[name='"+name+"']").prop('checked', false);
  		}
      updateReset($(this).prop('checked'), nameParts);
    });

	  $("input.path-reset-base").change(function () {
      console.log("base");
      nameParts = getNameParts($(this).attr('name'));
		  $('table#permissions').find("input.path-reset-"+nameParts[0]).prop('checked', this.checked);
	  });

    /**
     * Disables the reset_alias checkboxes if path alias checkbox is unchecked.
     */
    function getNameParts(name) {
      tag1 = name.split('][');
      tag2 = tag1[0].split('[');
      tag3 = tag1[1].replace(']', '');
      return [tag2[1], tag3];
    }

    /**
     * Disables the reset_alias checkboxes if path alias checkbox is unchecked.
     */
    function updateReset(checked, nameParts) {
      resetName = 'reset_alias'+'['+nameParts[0]+']['+nameParts[1]+']';
      if(checked) {
   		  $('table#permissions').find("input[name='"+resetName+"']").prop('disabled', false);
      }
      else {
   		  $('table#permissions').find("input[name='"+resetName+"']").prop('checked', false).prop('disabled', true);
      }
    }


  }
};

})(jQuery);
