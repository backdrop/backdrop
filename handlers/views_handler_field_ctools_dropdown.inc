<?php

/**
 * @file
 * Definition of views_handler_field_ctools_dropdown.
 */

/**
 * Field handler which displays some amount of links as ctools dropdown button.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_ctools_dropdown extends views_handler_field_links {
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['fields']['#destination'] = t('Fields to be included as ctools dropdown button.');
    $form['destination']['#description'] = t('Include a "destination" parameter in the link to return the user to the original view upon completing a link action.');
  }

  /**
   * Render the dropdown button.
   */
  function render($values) {
    $links = $this->get_links();
    if (!empty($links)) {
      return theme('links__ctools_dropbutton', array('links' => $links, 'attributes' => array('class' => array('links', 'inline'))));
    }
    else {
      return '';
    }
  }
}
