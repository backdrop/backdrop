(function($){
/**
 * To make a form auto submit, all you have to do is 3 things:
 *
 * Attach the autosubmit library to the form.
 *
 * @code
 *   $form['#attached']['library'][] = array('system', 'backdrop.autosubmit');
 * @endcode
 *
 * On elements you want to auto-submit when changed, add the autosubmit
 * class. With FAPI, add:
 * @code
 *   $form['field_name']['#attributes']['class'] = array('autosubmit');
 * @endcode
 *
 * If you want to have auto-submit for every form element,
 * add the autosubmit-full-form to the form. With FAPI, add:
 * @code
 *   $form['#attributes']['class'] = array('autosubmit-full-form');
 * @endcode
 *
 * If you want to exclude a field from the autosubmit-full-form auto submission,
 * add the class autosubmit-exclude to the form element. With FAPI, add:
 * @code
 *   $form['field_name']['#attributes']['class'] = array('autosubmit-exclude');
 * @endcode
 *
 * Finally, you have to identify which button you want clicked for autosubmit.
 * @code
 *   $form['my_button']['#attributes']['class'] = array('autosubmit-click');
 * @endcode
 *
 * Currently only 'select', 'radio', 'checkbox' and 'textfield' types are
 * supported.
 */
Backdrop.behaviors.autosubmit = {
  attach: function(context) {
    // 'this' references the form element
    function triggerSubmit (element) {
      var $this = $(this);

      // Variable "element" will have a value only when text fields trigger
      // this. If element is undefined, then remove the data to prevent
      // potential focus on a previously processed element.
      if (element === undefined)  {
        $('body').removeData('autosubmit-last-focus-id');
      }
      else {
        $('body').data('autosubmit-last-focus-id', $(element).attr('id'));
      }

      // Submit the form.
      $this.find('.autosubmit-click').trigger('click');
    }

    // Listener to ajaxStop will re-focus on the text field as needed.
    $(document).off('ajaxStop.autosubmit').on('ajaxStop.autosubmit', function () {
      let id = $('body').data('autosubmit-last-focus-id');
      if (id === undefined) {
        return;
      }
      let $textInput = $('#' + id);
      let pos = $textInput.val().length;
      $textInput.focus();
      $textInput[0].setSelectionRange(pos, pos);
      $('body').removeData('autosubmit-last-focus-id');
    });

    // The change event bubbles so we only need to bind it to the outer form.
    $('form.autosubmit-full-form', context)
      .add('.autosubmit', context)
      .filter('form, select, input:not(:text, :submit)')
      .once('autosubmit')
      .on('change', function (e) {
        // don't trigger on text change for full-form
        if ($(e.target).is(':not(:text, :submit, .autosubmit-exclude)')) {
          triggerSubmit.call(e.target.form);
        }
      });

    // e.keyCode: key
    var discardKeyCode = [
      16, // shift
      17, // ctrl
      18, // alt
      20, // caps lock
      33, // page up
      34, // page down
      35, // end
      36, // home
      37, // left arrow
      38, // up arrow
      39, // right arrow
      40, // down arrow
      9, // tab
      13, // enter
      27  // esc
    ];
    // Don't wait for change event on textfields
    $('.autosubmit-full-form input:text, input:text.autosubmit', context)
      .filter(':not(.autosubmit-exclude)')
      .once('autosubmit', function () {
        // each textinput element has his own timeout
        var timeoutID = 0;
        $(this)
          .on('keydown keyup', function (e) {
            if ($.inArray(e.keyCode, discardKeyCode) === -1) {
              timeoutID && clearTimeout(timeoutID);
            }
          })
          .on('keyup', function(e) {
            if ($.inArray(e.keyCode, discardKeyCode) === -1) {
              // Provide the target element to triggerSubmit.
              timeoutID = setTimeout($.proxy(triggerSubmit, this.form, e.target), 500);
            }
          })
          .on('change', function (e) {
            if ($.inArray(e.keyCode, discardKeyCode) === -1) {
              // Provide the target element to triggerSubmit.
              timeoutID = setTimeout($.proxy(triggerSubmit, this.form, e.target), 500);
            }
          });
      });
  }
}

})(jQuery);
