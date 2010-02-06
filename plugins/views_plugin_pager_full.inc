<?php
// $Id$

/**
 * The plugin to handle full pager.
 *
 * @ingroup views_pager_plugins
 */
class views_plugin_pager_full extends views_plugin_pager {
  function summary_title() {
    if (!empty($this->options['offset'])) {
      return format_plural($this->options['items_per_page'], 'Paged, @count item, skip @skip', 'Paged, @count items, skip @skip', array('@count' => $this->options['items_per_page'], '@skip' => $this->options['offset']));
    }
      return format_plural($this->options['items_per_page'], 'Paged, @count item', 'Paged, @count items', array('@count' => $this->options['items_per_page']));
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['items_per_page'] = array('default' => 10);
    $options['offset'] = array('default' => 0);
    $options['id'] = array('default' => 0);

    return $options;
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    $form['items_per_page'] = array(
      '#title' => t('Items per page'),
      '#type' => 'textfield',
      '#description' => t('The number of items to display per page. Enter 0 for no limit.'),
      '#default_value' => $this->options['items_per_page'],
    );

    $form['offset'] = array(
      '#type' => 'textfield',
      '#title' => t('Offset'),
      '#description' => t('The number of items to skip. For example, if this field is 3, the first 3 items will be skipped and not displayed.'),
      '#default_value' => $this->options['offset'],
    );

    $form['id'] = array(
      '#type' => 'textfield',
      '#title' => t('Pager ID'),
      '#description' => t("Unless you're experiencing problems with pagers related to this view, you should leave this at 0. If using multiple pagers on one page you may need to set this number to a higher value so as not to conflict within the ?page= array. Large values will add a lot of commas to your URLs, so avoid if possible."),
      '#default_value' => $this->options['id'],
    );
  }

  function query() {
    $this->view->query->set_limit($this->options['items_per_page']);
    $this->view->query->set_offset($this->current_page * $this->options['items_per_page'] + $this->options['offset']);
  }

  function render($input) {
    $pager_theme = views_theme_functions('pager', $this->view, $this->display);
    $output = theme($pager_theme, array(
      'tags' => $input, 'quantity' => $this->options['items_per_page'], 'element' => $this->options['id']));
    return $output;
  }

  /**
   * Set the current page.
   *
   * @param $number
   *   If provided, the page number will be set to this. If NOT provided,
   *   the page number will be set from the global page array.
   */
  function set_current_page($number = NULL) {
    if (isset($number)) {
      $this->current_page = $number;
      return;
    }

    // If the current page number was not prespecified, default to pulling it from 'page'
    // based upon
    global $pager_page_array;
    // Extract the ['page'] info.
    $pager_page_array = isset($_GET['page']) ? explode(',', $_GET['page']) : array();

    $this->current_page = 0;
    if (!empty($pager_page_array[$this->options['id']])) {
      $this->current_page = intval($pager_page_array[$this->options['id']]);
    }
  }

  /**
   * Update global paging info.
   *
   * This is called after the count query has been run to set the total
   * items available and to update the current page if the requested
   * page is out of range.
   */
  function update_page_info() {
    // Dump information about what we already know into the globals.
    global $pager_page_array, $pager_total, $pager_total_items;

    // Set the item count for the pager.
    $pager_total_items[$this->options['id']] = $this->total_items;
    // Calculate and set the count of available pages.
    $pager_total[$this->options['id']] = ceil($pager_total_items[$this->options['id']] / $this->get_items_per_page());

    // See if the requested page was within range:
    if ($this->current_page < 0) {
      $this->current_page = 0;
    }
    else if ($this->current_page >= $pager_total[$this->options['id']]) {
      // Pages are numbered from 0 so if there are 10 pages, the last page is 9.
      $this->current_page = $pager_total[$this->options['id']] - 1;
    }

    // Put this number in to guarantee that we do not generate notices when the pager
    // goes to look for it later.
    $pager_page_array[$this->options['id']] = $this->current_page;
  }

}
