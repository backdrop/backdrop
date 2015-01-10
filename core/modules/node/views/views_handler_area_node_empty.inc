<?php

/**
 * @file
 * Definition of views_handler_area_node_empty.
 */

/**
 * Views area handler to provide a link to the node add overview page.
 *
 * @ingroup views_area_handlers
 */
class views_handler_area_node_empty extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    unset($form['label']);
  }

  function render($empty = FALSE) {
    $link = array(
      'items' => array(
        l(t('Add new content'), 'node/add'),
      ),
    );
    return theme('item_list', $link);
  }
}
