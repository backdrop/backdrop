<?php
/**
 * @file
 * Definition of views_handler_field_node_link.
 */

/**
 * Field handler to present a link to the node.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_node_link extends views_handler_field_entity {

  function option_definition() {
    $options = parent::option_definition();
    $options['url'] = array('default' => FALSE, 'bool' => TRUE);
    $options['absolute'] = array('default' => FALSE, 'bool' => TRUE);
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['url'] = array(
      '#type' => 'checkbox',
      '#title' => t('Output URL as text'),
      '#default_value' => $this->options['url'],
      '#description' => t('The resulting URL will be plain text and not a clickable link.'),
    );
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
      '#indentation' => 1,
      '#states' => array(
        'visible' => array(
          ':input[name="options[url]"]' => array('checked' => FALSE),
        ),
      ),
    );
    $form['absolute'] = array(
      '#type' => 'checkbox',
      '#title' => t('Output an absolute link starting with "http(s)://"'),
      '#default_value' => $this->options['absolute'],
    );
    parent::options_form($form, $form_state);

    // The path is set by render_link function so don't allow to set it.
    $form['alter']['path'] = array('#access' => FALSE);
    $form['alter']['external'] = array('#access' => FALSE);
    $form['alter']['absolute'] = array('#access' => FALSE);
  }

  function render($values) {
    if ($entity = $this->get_value($values)) {
      return $this->render_link($entity, $values);
    }
  }

  function render_link($node, $values) {
    $options = &$this->options;

    if (node_access('view', $node)) {
      if ($options['url']) {
        return url("node/$node->nid", array('absolute' => $options['absolute']));
      }
      else {
        $options['alter']['make_link'] = TRUE;
        $options['alter']['absolute'] = $options['absolute'];
        $options['alter']['path'] = "node/$node->nid";
        $text = !empty($options['text']) ? $options['text'] : t('view');
        return $text;
      }
    }
  }
}
