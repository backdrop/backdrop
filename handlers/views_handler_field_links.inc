<?php

/**
 * @file
 * Definition of views_handler_field_links.
 */

/**
 * A abstract handler which provides a collection of links.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_links extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();

    $options['fields'] = array('default' => array());
    $options['destination'] = array('default' => TRUE, 'bool' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $all_fields = $this->view->display_handler->get_field_labels();
    // Offer to include only those fields that follow this one.
    $field_options = array_slice($all_fields, 0, array_search($this->options['id'], array_keys($all_fields)));
    $form['fields'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Fields'),
      '#description' => t('Fields to be included as links.'),
      '#options' => $field_options,
      '#default_value' => $this->options['fields'],
    );
    $form['destination'] = array(
      '#type' => 'checkbox',
      '#title' => t('Include destination'),
      '#description' => t('Include a "destination" parameter in the link to return the user to the original view upon completing the link action.'),
      '#default_value' => $this->options['destination'],
    );
  }

  /**
   * Return the list of links of this field.
   *
   * @return array
   *   The links which are used by the render function.
   */
  function get_links() {
    $links = array();
    foreach ($this->options['fields'] as $field) {
      if (empty($this->view->field[$field]->last_render_text)) {
        continue;
      }
      $title = $this->view->field[$field]->last_render_text;
      $path = '';
      if (!empty($this->view->field[$field]->options['alter']['path'])) {
        $path = $this->view->field[$field]->options['alter']['path'];
      }
      // Make sure that tokens are replaced for this paths as well.
      $tokens = $this->get_render_tokens(array());
      $path = strip_tags(decode_entities(strtr($path, $tokens)));

      $links[$field] = array(
        'href' => $path,
        'title' => $title,
      );
      if (!empty($this->options['destination'])) {
        $links[$field]['query'] = drupal_get_destination();
      }
    }

    return $links;
  }

  function query() { }
}
