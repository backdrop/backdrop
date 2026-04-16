(function($) {

  /**
   * Icon Browser behavior.
   *
   * Add indications on all activated icons (those that have been added to the
   * page and are available for CSS and JS use) in browser list and add info
   * to details as well.
   */
  Backdrop.behaviors.iconBrowser = {
  attach: function (context, settings) {

    // Bind AJAX behaviors to all items showing the class.
    var base_element_settings = {
      'event': 'click',
      'progress': { 'type': 'throbber' }
    };
    $('#system-icon-browser-pager li a', context).once('icon-browser-pager-links').each(function () {
      var element_settings = base_element_settings;

      if ($(this).attr('href')) {
        var settings_href = $(this).attr('href');
        var split_href = settings_href.split('?');
        if (split_href[0] != '/icon-browser/dialog') {
          settings_href = '/icon-browser/dialog?'+split_href[1];
        }
        element_settings.url = settings_href;
      }
      var base = $(this).attr('class');
      Backdrop.ajax[base] = new Backdrop.ajax(base, this, element_settings);
    });

    /**
     * Selects a token.
     * 
     * Invoked when a token link is clicked.
     */
    function select(event) {
      var $token = $(event.currentTarget );

      event.preventDefault();
      event.stopPropagation();

      if (Backdrop.settings.iconBrowserSelectedToken) {
        Backdrop.settings.iconBrowserSelectedToken.removeClass('selected-token');
        Backdrop.settings.iconBrowserSelectedToken.removeAttr('data-after');
        Backdrop.settings.iconBrowserSelectedToken.removeAttr('aria-selected');
      }

      if (Backdrop.settings.iconBrowserSelectedToken && $token[0] === Backdrop.settings.iconBrowserSelectedToken[0]) {
        Backdrop.settings.iconBrowserSelectedToken = null;
      }
      else {
        console.log('kik');
        Backdrop.settings.iconBrowserSelectedToken = $token;
        Backdrop.settings.iconBrowserSelectedToken.addClass('selected-token');
        Backdrop.settings.iconBrowserSelectedToken.attr('data-after', ' (' + Backdrop.t('selected') + ')');
        Backdrop.settings.iconBrowserSelectedToken.attr('aria-selected');
      }
      // If we have a focused field, insert the selected token.
      if (typeof Backdrop.settings.iconBrowserFocusedField !== 'undefined' && Backdrop.settings.iconBrowserSelectedToken) {
      console.log(Backdrop.settings.iconBrowserSelectedToken);
        insert(Backdrop.settings.iconBrowserFocusedField);
      }
    }

    /**
     * Inserts a token in a DOM text field.
     * 
     * @param myField 
     */
    function insert(myField) {
      if (Backdrop.settings.iconBrowserSelectedToken) {
        var startPos = myField.selectionStart;
        var endPos = myField.selectionEnd;
        var myValue  = Backdrop.settings.iconBrowserSelectedToken;
        var myValue  = $(myValue).data('icon-name');
        myField.value = 
          myField.value.substring(0, startPos)
          + myValue
          + myField.value.substring(endPos, myField.value.length)
        ;
        Backdrop.settings.iconBrowserSelectedToken.removeClass('selected-token');
        Backdrop.settings.iconBrowserSelectedToken.removeAttr('aria-selected');
        Backdrop.settings.iconBrowserSelectedToken = null;
      }
    }

    var click_insert = Backdrop.settings.iconBrowser.click_insert;

    // tr.setAttribute('aria-level', level);
    // tr.setAttribute('aria-posinset', index);

    // $('.icon-wrapper').addEventListener('click', expand);

    if (click_insert) {
      $('.icon-wrapper').attr('title', 'Select the token. Click in a text field to insert it.');
      $('.icon-wrapper').on('click', select);
    }

    // Set up listener for the token tree button.
    // $(context).find('button').on('click', expand);

    // Keep track of which textfield was last selected/focused.
    $('.page').find('textarea, input[type="text"]').once('token-browser-field-focus').on('focus', function() {
      console.log('Backdrop.settings.iconBrowserSelectedToken');
      Backdrop.settings.iconBrowserFocusedField = this;
    });

    // Finds a textarea where token will be inserted and starts a listener.
    var $input = $('textarea, input[type="text"]', context);
    if ($input.length) {
      $input.once('token-browser-insert').on('click', function (event) {
        insert(event.target);
      });
    }

    // Attach listener to open first token type when there is only one in the table.
    $('body').once('token-browser-dialogopen').on('dialogopen', '.token-browser-dialog', function (event){
      if ($(this).find('table tr.tree-grid-parent').length == 1) {
        $(this).find('table tr.tree-grid-parent').first().find('button').trigger('click');
      }
    });
  }
}
})(jQuery);
