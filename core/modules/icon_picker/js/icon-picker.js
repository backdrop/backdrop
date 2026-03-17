/**
 * @file
 * Icon picker dialog behavior for icon_select form elements.
 */

(function ($, Backdrop) {
  'use strict';

  /**
   * Internal state for the shared picker dialog.
   *
   * A single dialog instance is reused across all icon_select elements on the
   * page. activeField tracks which input to populate on confirmation.
   */
  var picker = {
    dialog: null,
    $content: null,
    $results: null,
    $search: null,
    $pager: null,
    $message: null,
    activeField: null,
    selectedIcon: null,
    currentPage: 0,
    limit: 48,
    endpoint: '',
    loading: false
  };

  /**
   * Builds the dialog content and registers it with Backdrop.dialog.
   *
   * Called once on first use; subsequent calls are no-ops.
   */
  function buildDialog() {
    if (picker.dialog) {
      return;
    }

    // Build the content including footer buttons so we control their markup
    // and can use Backdrop's .button classes directly.
    var $content = $(
      '<div class="icon-picker-content">'
      + '<div class="icon-picker-filter">'
      + '<label class="icon-picker-filter-label">' + Backdrop.t('Icon name') + '</label>'
      + '<div class="icon-picker-filter-controls">'
      + '<input type="search" class="form-text icon-picker-search"'
      + ' placeholder="' + Backdrop.t('Search\u2026') + '">'
      + '<button type="button" class="button icon-picker-filter-btn">'
      + Backdrop.t('Filter') + '</button>'
      + '</div>'
      + '</div>'
      + '<div class="icon-picker-message"></div>'
      + '<div class="icon-picker-results" role="listbox"'
      + ' aria-label="' + Backdrop.t('Available icons') + '"></div>'
      + '<div class="icon-picker-pager"></div>'
      + '<div class="icon-picker-footer">'
      + '<button type="button" class="button button-primary icon-picker-select">'
      + Backdrop.t('Select') + '</button>'
      + '<button type="button" class="button icon-picker-cancel">'
      + Backdrop.t('Cancel') + '</button>'
      + '</div>'
      + '</div>'
    );

    // Hide before appending so the div does not render on the page until the
    // dialog is opened. jQuery UI manages visibility from this point on.
    $content.hide().appendTo('body');

    picker.$content = $content;
    picker.$results = $content.find('.icon-picker-results');
    picker.$search  = $content.find('.icon-picker-search');
    picker.$pager   = $content.find('.icon-picker-pager');
    picker.$message = $content.find('.icon-picker-message');

    picker.dialog = Backdrop.dialog($content[0], {
      title: Backdrop.t('Select an Icon'),
      dialogClass: 'icon-picker-dialog',
      width: '800px',
      // Disable autoResize so we can control height precisely in open().
      autoResize: false,
      // No jQuery UI buttons array; footer buttons live inside the content.
      buttons: [],
      open: function () {
        // jQuery UI adds .ui-dialog-content to our element and sets
        // overflow:auto on it via inline style, which would cause the whole
        // content area to scroll. Override height and overflow so only the
        // results grid scrolls while the filter and footer stay visible.
        var $this    = $(this);
        var $dialog  = $this.closest('.ui-dialog');
        var titlebar = $dialog.find('.ui-dialog-titlebar').outerHeight(true) || 0;
        var height   = Math.max(300, Math.round(window.innerHeight * 0.72) - titlebar);
        $this.css({height: height + 'px', overflow: 'hidden'});
        // Position near the top so the dialog stays fully within the viewport.
        $dialog.css({top: '5%', transform: 'none', marginTop: 0});
      }
    });

    // Live search: fetch as the user types.
    $content.on('input', '.icon-picker-search', Backdrop.debounce(function () {
      picker.currentPage = 0;
      fetchIcons();
    }, 200));

    // Filter button: immediate fetch.
    $content.on('click', '.icon-picker-filter-btn', function () {
      picker.currentPage = 0;
      fetchIcons();
    });

    // Single click: highlight the chosen icon.
    $content.on('click', '.icon-picker-result', function () {
      $content.find('.icon-picker-result').removeClass('is-selected');
      $(this).addClass('is-selected');
      picker.selectedIcon = $(this).data('icon');
    });

    // Double-click: select and close immediately.
    $content.on('dblclick', '.icon-picker-result', function () {
      picker.selectedIcon = $(this).data('icon');
      confirmSelection();
    });

    $content.on('click', '.icon-picker-select', confirmSelection);
    $content.on('click', '.icon-picker-cancel', function () {
      picker.dialog.close();
    });
  }

  /**
   * Populates the active field with the selected icon name and closes the
   * dialog.
   */
  function confirmSelection() {
    if (picker.selectedIcon && picker.activeField) {
      picker.activeField.val(picker.selectedIcon).trigger('change');
    }
    picker.dialog.close();
  }

  /**
   * Opens the dialog targeting the given input field.
   *
   * @param {jQuery} $field
   *   The icon_select text input to populate on confirmation.
   */
  function openDialog($field) {
    picker.activeField  = $field;
    picker.selectedIcon = $field.val() || null;
    picker.$search.val('');
    picker.currentPage = 0;
    picker.dialog.showModal();
    fetchIcons();
  }

  /**
   * Fetches a page of icons from the JSON endpoint and renders results.
   *
   * @param {number} [page]
   *   Zero-based page number. Defaults to picker.currentPage.
   */
  function fetchIcons(page) {
    if (picker.loading) {
      return;
    }
    picker.loading = true;
    picker.$message.text(Backdrop.t('Loading\u2026'));
    picker.$results.empty();

    if (typeof page === 'number') {
      picker.currentPage = page;
    }

    $.getJSON(picker.endpoint, {
      search: picker.$search.val() || '',
      page: picker.currentPage,
      limit: picker.limit
    }).done(function (data) {
      renderResults(data);
    }).fail(function (xhr) {
      var status = xhr && xhr.status ? xhr.status : 'error';
      picker.$message.text(
        Backdrop.t('Could not load icons (@status).', {'@status': status})
      );
    }).always(function () {
      picker.loading = false;
    });
  }

  /**
   * Renders a page of icon results into the dialog grid.
   *
   * Icon markup is sanitized server-side by theme_icon() before being included
   * in the JSON response, so it is safe to inject here directly.
   *
   * @param {Object} data  Response from the JSON endpoint.
   */
  function renderResults(data) {
    picker.$results.empty();
    picker.$message.text('');

    if (!data || !data.results || data.results.length === 0) {
      picker.$message.text(Backdrop.t('No icons found.'));
      picker.$pager.empty();
      return;
    }

    data.results.forEach(function (item) {
      var $btn = $(
        '<button type="button" class="icon-picker-result" role="option"></button>'
      );
      $btn.attr({
        'data-icon': item.name,
        'aria-label': item.name,
        'aria-selected': picker.selectedIcon === item.name ? 'true' : 'false'
      });
      if (picker.selectedIcon === item.name) {
        $btn.addClass('is-selected');
      }
      $btn.append('<span class="icon-picker-swatch">' + item.markup + '</span>');
      $btn.append(
        '<span class="icon-picker-name">' + Backdrop.checkPlain(item.name) + '</span>'
      );
      picker.$results.append($btn);
    });

    renderPager(data.total, data.page, data.limit);
  }

  /**
   * Renders pagination controls below the results grid.
   *
   * @param {number} total  Total number of matching icons.
   * @param {number} page   Current zero-based page number.
   * @param {number} limit  Results per page.
   */
  function renderPager(total, page, limit) {
    picker.$pager.empty();
    var totalPages = Math.ceil(total / limit);
    if (totalPages <= 1) {
      return;
    }

    var $prev = $('<button type="button" class="button button-small">'
      + Backdrop.t('Prev') + '</button>');
    var $next = $('<button type="button" class="button button-small">'
      + Backdrop.t('Next') + '</button>');

    $prev.prop('disabled', page <= 0);
    $next.prop('disabled', page >= totalPages - 1);
    $prev.on('click', function () { fetchIcons(page - 1); });
    $next.on('click', function () { fetchIcons(page + 1); });

    var $status = $('<span class="icon-picker-page-status"></span>').text(
      Backdrop.t('Page @page of @total', {'@page': page + 1, '@total': totalPages})
    );
    picker.$pager.append($prev, $status, $next);
  }

  /**
   * Attaches the icon picker dialog behavior to icon_select launch buttons.
   *
   * @type {Backdrop~behavior}
   */
  Backdrop.behaviors.iconPicker = {
    attach: function (context, settings) {
      if (!settings.iconPicker || !settings.iconPicker.endpoint) {
        return;
      }

      picker.endpoint = settings.iconPicker.endpoint;
      picker.limit    = settings.iconPicker.limit || 48;

      buildDialog();

      // Wire each Browse button to open the dialog targeting its paired input.
      $('.icon-picker-launch', context).once('icon-picker').each(function () {
        var $btn   = $(this);
        var target = $btn.data('target');
        var $field = $btn.closest('.form-item').find('#' + target);

        $btn.on('click', function (e) {
          e.preventDefault();
          openDialog($field);
        });
      });
    },

    detach: function (context, settings, trigger) {
      // If the element containing the active field is being removed from the
      // page, close the dialog and clear any stale field reference.
      if (trigger === 'unload' && picker.activeField
          && $.contains(context, picker.activeField[0])) {
        picker.dialog.close();
        picker.activeField  = null;
        picker.selectedIcon = null;
      }
    }
  };

})(jQuery, Backdrop);
