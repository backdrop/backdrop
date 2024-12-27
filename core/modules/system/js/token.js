/**
 * JavaScript behaviors for the display of the Token UI.
 */
(function ($) {

"use strict";

Backdrop.behaviors.tokenBrowser = {
  attach: function (context, settings) {
    var allAncestors = {};

    var buffer;
    var trTemplate;
    var buttonTemplate;
    var linkTemplate;
    var noLinkTemplate;
    var nameTemplate;
    var rawTemplate;
    var descriptionTemplate;

    // Create DOM element templates and assign attributes. 
    buffer = document.createDocumentFragment();
    trTemplate = document.createElement('tr');
    buttonTemplate = document.createElement('button');
    linkTemplate = document.createElement('a');
    noLinkTemplate = document.createElement('span');
    nameTemplate = document.createElement('td');
    rawTemplate = document.createElement('td');
    descriptionTemplate = document.createElement('td');

    trTemplate.setAttribute('role', 'row');
    trTemplate.setAttribute('aria-expanded', 'false');
    trTemplate.setAttribute('aria-busy', 'false');

    linkTemplate.setAttribute('href', 'javascript:void(0);');
    linkTemplate.setAttribute('class', 'token-key');

    nameTemplate.setAttribute('role', 'gridcell');
    nameTemplate.setAttribute('class', 'token-name');

    rawTemplate.setAttribute('role', 'gridcell');
    rawTemplate.setAttribute('class', 'token-raw');

    descriptionTemplate.setAttribute('role', 'gridcell');
    descriptionTemplate.setAttribute('class', 'token-description');

    /**
     * Selects a token.
     * 
     * Invoked when a token link is clicked.
     */
    function select(event) {
      var $token = $(event.target);

      event.preventDefault();
      event.stopPropagation();

      if (Backdrop.settings.tokenBrowserSelectedToken) {
        Backdrop.settings.tokenBrowserSelectedToken.removeClass('selected-token');
        Backdrop.settings.tokenBrowserSelectedToken.removeAttr('data-after');
        Backdrop.settings.tokenBrowserSelectedToken.removeAttr('aria-selected');
      }

      if (Backdrop.settings.tokenBrowserSelectedToken && $token[0] === Backdrop.settings.tokenBrowserSelectedToken[0]) {
        Backdrop.settings.tokenBrowserSelectedToken = null;
      }
      else {
        Backdrop.settings.tokenBrowserSelectedToken = $token;
        Backdrop.settings.tokenBrowserSelectedToken.addClass('selected-token');
        Backdrop.settings.tokenBrowserSelectedToken.attr('data-after', ' (' + Backdrop.t('selected') + ')');
        Backdrop.settings.tokenBrowserSelectedToken.attr('aria-selected');
      }
      // If we have a focused field, insert the selected token.
      if (typeof Backdrop.settings.tokenBrowserFocusedField !== 'undefined' && Backdrop.settings.tokenBrowserSelectedToken) {
        insert(Backdrop.settings.tokenBrowserFocusedField);
      }
    }

    /**
     * Inserts a token in a DOM text field.
     * 
     * @param myField 
     */
    function insert(myField) {
      if (Backdrop.settings.tokenBrowserSelectedToken) {
        var startPos = myField.selectionStart;
        var endPos = myField.selectionEnd;
        var myValue  = Backdrop.settings.tokenBrowserSelectedToken.text();
        myField.value = 
          myField.value.substring(0, startPos)
          + myValue
          + myField.value.substring(endPos, myField.value.length)
        ;
        Backdrop.settings.tokenBrowserSelectedToken.removeClass('selected-token');
        Backdrop.settings.tokenBrowserSelectedToken.removeAttr('aria-selected');
        Backdrop.settings.tokenBrowserSelectedToken = null;
      }
    }

    /**
     * Gets ancestors for a token type.
     * 
     * The ancestors are the tokens above a given chained token.
     * 
     * @returns array
     *  An array of token parts. 
     */
    function getAncestors($cell) {
      var ancestors = allAncestors[$cell.data('raw')];

      return ancestors ? ancestors : [];
    }

    /**
     * Helper to extract the token from the ajax response.
     * 
     * @returns string
     *  Token part name.
     */
    function getToken($cell, type) {
      var token = $cell.data('token');

      return token ? token : type;
    }

    /**
     * Displays a row.
     */
    function display(parent, value, callback) {
      var current = parent;
      var level, top = Number(parent.getAttribute('aria-level'));
      var expand = [];

      expand[top + 1] = true;

      while (current = current.nextElementSibling) {
        level = Number(current.getAttribute('aria-level'));

        if (level <= top) {
          break;
        }

        expand[level + 1] = expand[level] && current.getAttribute('aria-expanded') === 'true';

        if (expand[level]) {
          current.style.display = value;
        }
      }

      callback();
    }

    /**
     * Builds a row.
     * 
     * @returns DOMelement
     *   The table row.
     */
    function row(element, level, index) {
      var click_insert = Backdrop.settings.tokenBrowser.click_insert;
      var tr = trTemplate.cloneNode(false);
      var button = buttonTemplate.cloneNode(false);
      if (click_insert) {
        var link = linkTemplate.cloneNode(false);
      }
      else {
        var link = noLinkTemplate.cloneNode(false);
      }
      var name = nameTemplate.cloneNode(false);
      var raw = rawTemplate.cloneNode(false);
      var description = descriptionTemplate.cloneNode(false);

      tr.setAttribute('aria-level', level);
      tr.setAttribute('aria-posinset', index);

      button.addEventListener('click', expand);
      button.innerHTML = 'Expand';

      if (click_insert) {
        link.setAttribute('title', 'Select the token ' + element.raw + '. Click in a text field to insert it.');
        link.addEventListener('click', select);
      }

      name.setAttribute('data-token', element.token);
      name.setAttribute('data-type', element.type);
      name.setAttribute('data-raw', element.raw);

      raw.appendChild(link);

      name.innerHTML = element.name;
      link.innerHTML = element.raw;
      description.innerHTML = element.description;

      if (element.type) {
        name.insertBefore(button, name.firstChild);
        tr.classList.add('tree-grid-parent');
      }
      else {
        tr.classList.add('tree-grid-leaf');
      }

      allAncestors[element.raw] = element.ancestors;

      tr.appendChild(name);
      tr.appendChild(raw);
      tr.appendChild(description);

      return tr;
    }

    /**
     * Fetches tokens down the token tree via ajax.
     */
    function fetch($row, $cell, callback) {
      var type = $cell.data('type');
      var token = getToken($cell, type);
      var ancestors = getAncestors($cell);
      var url = Backdrop.settings.basePath + 'token-browser/token/' + type;
      var parameters = {};

      ancestors.push(token);

      parameters.ancestors = JSON.stringify(ancestors);
      parameters.token = Backdrop.settings.tokenBrowser.token;
      parameters.show_restricted = Backdrop.settings.tokenBrowser.restricted;

      $.ajax({
        'url': url,
        'data': parameters,
        'success': function (data) {
          var level = Number($row.attr('aria-level')) + 1;
          var position = 1;

          for (var key in data) {
            buffer.appendChild(row(data[key], level, position++));
          }

          $row.attr('aria-setsize', buffer.childElementCount);
          $row.after(buffer);
          callback(true);
        },
        'error': function (request) {
          callback(false);
        },
        'dataType': 'json'
      });
    }

    /**
     * Expands a token branch.
     */
    function expand(event) {
      var $button = $(event.target);
      var $cell = $button.parent();
      var $row = $cell.parent();

      event.preventDefault();
      event.stopPropagation();

      $button.off('click', expand);

      if ($row.data('fetched')) {
        display($row[0], 'table-row', function () {
          $row.attr('aria-expanded', 'true');
          $button.text('Collapse');
          $button.on('click', collapse);
        });
      }
      else {
        $row.attr('aria-busy', 'true');
        $button.text('Loading...');

        fetch($row, $cell, function (success) {
          if (success) {
            $row.data('fetched', true);
            $row.attr('aria-expanded', 'true');
            $button.text('Collapse');
            $button.on('click', collapse);
          }
          else {
            $button.text('Expand');
            $button.on('click', expand);
          }

          $row.attr('aria-busy', 'false');
        });
      }
    }

    /**
     * Collapses a token branch.
     */
    function collapse(event) {
      var $button = $(event.target);
      var $cell = $button.parent();
      var $row = $cell.parent();

      event.preventDefault();
      event.stopPropagation();

      $button.off('click', collapse);

      display($row[0], 'none', function () {
        $row.attr('aria-expanded', 'false');
        $button.text('Expand');
        $button.on('click', expand);
      });
    }

    // Set up listener for the token tree button.
    $(context).find('button').on('click', expand);

    // Keep track of which textfield was last selected/focused.
    $(context).find('textarea, input[type="text"]').once('token-browser-field-focus').on('focus', function() {
      Backdrop.settings.tokenBrowserFocusedField = this;
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
