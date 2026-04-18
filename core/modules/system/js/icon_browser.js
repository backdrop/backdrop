(function($) {

  /**
   * Icon Browser behavior.
   */
  Backdrop.behaviors.iconBrowser = {
  attach: function (context, settings) {

    // Bind AJAX behaviors to all icons showing the class.
    var base_element_settings = {
      'event': 'click',
      'progress': { 'type': 'throbber' }
    };
    $('#system-icon-browser-pager li a', context).once('icon-browser-pager-links').each(function () {
      var element_settings = base_element_settings;
      if ($(this).attr('href')) {
        element_settings.url = $(this).attr('href');
      }
      var base = $(this).attr('class');
      Backdrop.ajax[base] = new Backdrop.ajax(base, this, element_settings);
    });

    /**
     * Selects an icon.
     * 
     * Invoked when a icon is clicked in the Icon Browser.
     */
    function select(event) {
      var $icon = $(event.currentTarget );

      event.preventDefault();
      event.stopPropagation();

      if (Backdrop.settings.iconBrowserSelectedIcon) {
        Backdrop.settings.iconBrowserSelectedIcon.removeClass('selected-icon');
        Backdrop.settings.iconBrowserSelectedIcon.removeAttr('data-after');
        Backdrop.settings.iconBrowserSelectedIcon.removeAttr('aria-selected');
      }

      if (Backdrop.settings.iconBrowserSelectedIcon && $icon[0] === Backdrop.settings.iconBrowserSelectedIcon[0]) {
        Backdrop.settings.iconBrowserSelectedIcon = null;
      }
      else {
        Backdrop.settings.iconBrowserSelectedIcon = $icon;
        Backdrop.settings.iconBrowserSelectedIcon.addClass('selected-icon');
        Backdrop.settings.iconBrowserSelectedIcon.attr('data-after', ' (' + Backdrop.t('selected') + ')');
        Backdrop.settings.iconBrowserSelectedIcon.attr('aria-selected');
      }
      // If we have a focused field, insert the selected icon.
      if (typeof Backdrop.settings.iconBrowserFocusedField !== 'undefined' && Backdrop.settings.iconBrowserSelectedIcon) {
        insert(Backdrop.settings.iconBrowserFocusedField);
      }
    }

    /**
     * Inserts an icon in a DOM text field.
     * 
     * @param myField 
     */
    function insert(myField) {
      if (Backdrop.settings.iconBrowserSelectedIcon) {
        var startPos = myField.selectionStart;
        var endPos = myField.selectionEnd;
        var myValue  = Backdrop.settings.iconBrowserSelectedIcon;
        var myValue  = $(myValue).data('icon-name');
        myField.value = 
          myField.value.substring(0, startPos)
          + myValue
          + myField.value.substring(endPos, myField.value.length)
        ;
        Backdrop.settings.iconBrowserSelectedIcon.removeClass('selected-icon');
        Backdrop.settings.iconBrowserSelectedIcon.removeAttr('aria-selected');
        Backdrop.settings.iconBrowserSelectedIcon = null;
      }
    }

    var click_insert = Backdrop.settings.iconBrowser.click_insert;

    if (click_insert) {
      $('.icon-wrapper').attr('title', 'Select the icon. Click in a text field to insert it.');
      $('.icon-wrapper').on('click', select);
    }

    // Keep track of which textfield was last selected/focused.
    $('.page').find('textarea, input[type="text"]').once('icon-browser-field-focus').on('focus', function() {
      Backdrop.settings.iconBrowserFocusedField = this;
    });

    // Finds a textarea where the icon will be inserted and starts a listener.
    var $input = $('textarea, input[type="text"]', context);
    if ($input.length) {
      $input.once('icon-browser-insert').on('click', function (event) {
        insert(event.target);
      });
    }
  }
}
})(jQuery);
