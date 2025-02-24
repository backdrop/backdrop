<?php
/**
 * @file
 * Definition of views_plugin_pager_mini.
 */

/**
 * The plugin to handle mini pager.
 *
 * @ingroup views_pager_plugins
 */
class views_plugin_pager_mini extends views_plugin_pager_full {
  function summary_title() {
    if (!empty($this->options['offset'])) {
      return format_plural($this->options['items_per_page'], 'Mini pager, @count item, skip @skip', 'Mini pager, @count items, skip @skip', array('@count' => $this->options['items_per_page'], '@skip' => $this->options['offset']));
    }
      return format_plural($this->options['items_per_page'], 'Mini pager, @count item', 'Mini pager, @count items', array('@count' => $this->options['items_per_page']));
  }

  /**
   * Overrides views_plugin_pager_full::option_definition().
   *
   * Overrides the full pager options form by deleting unused settings.
   */
  function option_definition() {
    $options = parent::option_definition();

    // Intentionally setting $options['quantity'] to NULL here instead of using
    // unset(), otherwise it throws "undefined index" notices.
    $options['quantity'] = NULL;
    unset($options['tags']['first']);
    unset($options['tags']['last']);
    $options['tags']['previous']['default'] = '‹‹';
    $options['tags']['next']['default'] = '››';

    $options['no_count_query'] = array('default' => TRUE);

    return $options;
  }

  /**
   * Overrides views_plugin_pager_full::options_form().
   *
   * Overrides the full pager options form by deleting unused settings.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    unset($form['quantity']);
    unset($form['tags']['first']);
    unset($form['tags']['last']);

    $form['no_count_query'] = array(
      '#type' => 'checkbox',
      '#title' => t('No total count'),
      '#description' => t('When checked, there will be no total count to prevent slow queries on large datasets.'),
      '#default_value' => $this->options['no_count_query'],
    );
  }

  function use_count_query() {
    return empty($this->options['no_count_query']);
  }

  function query() {
    parent::query();

    if (empty($this->options['no_count_query'])) {
      return;
    }

    $next_page = TRUE;
    if (!empty($this->options['total_pages'])) {
      if (($this->current_page + 1) >= $this->options['total_pages']) {
        $next_page = FALSE;
      }
    }
    if ($next_page) {
      $limit = $this->options['items_per_page'];
      // Fetch one extra to have a peek one page ahead.
      // Useful to determine if there are more pages without doing a total
      // count.
      $this->view->query->set_limit($limit + 1);
    }
  }

  function pre_render(&$result) {
    global $pager_page_array, $pager_total;

    if (empty($this->options['no_count_query'])) {
      return;
    }

    if (count($result) > $this->options['items_per_page']) {
      // query() sets limit 1 record past this page. If there is an extra record
      // remove it before rendering the result.
      array_pop($result);
      $this->next_page = TRUE;
      if ($pager_page_array[$this->get_pager_id()] == -1) {
        $this->set_current_page();
      }
      $pager_total[$this->get_pager_id()] = $pager_page_array[$this->get_pager_id()] + 1;
    }
    else {
      $this->set_current_page();
    }
  }

  /**
   * Overrides views_plugin_pager_full::render().
   *
   * Overrides the full pager renderer by changing the theme function
   * and leaving out variables that are not used in the mini pager.
   */
  function render($input) {
    global $pager_page_array;

    $next_page = !isset($this->next_page) || $this->next_page != TRUE;

    if (!empty($this->options['no_count_query']) && empty($pager_page_array[$this->get_pager_id()]) && $next_page) {
      return '';
    }

    $pager_theme = views_theme_functions('views_mini_pager', $this->view, $this->display);
    // The 1, 3 index are correct.
    // @see theme_pager().
    $tags = array(
      1 => $this->options['tags']['previous'],
      3 => $this->options['tags']['next'],
    );
    return theme($pager_theme, array(
      'tags' => $tags,
      'element' => $this->get_pager_id(),
      'parameters' => $input,
      'no_count_query' => !empty($this->options['no_count_query']),
    ));
  }
}
