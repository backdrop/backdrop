<?php
// $Id: views_plugin_pager_mini.inc,v 1.1.4.2 2010/02/06 13:29:48 dereine Exp $

/**
 * The plugin to handle full pager.
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

  function render($input) {
    $pager_theme = views_theme_functions('views_mini_pager', $this->view, $this->display);
    return theme($pager_theme, array(
      'tags' => $input, 'quantity' => $this->options['items_per_page'], 'element' => $this->options['id']));
  }
}
