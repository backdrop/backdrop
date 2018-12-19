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
    $options['type'] = array('default' => 'view');
    $options['url'] = array('default' => FALSE, 'bool' => TRUE);
    $options['absolute'] = array('default' => FALSE, 'bool' => TRUE);
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Link type'),
      '#default_value' => $this->options['type'],
      '#options' => array(
        'view' => t('View'),
        'edit' => t('Edit'),
        'delete' => t('Delete'),
      ),
    );
    $form['url'] = array(
      '#type' => 'checkbox',
      '#title' => t('Output URL as text'),
      '#default_value' => $this->options['url'],
      '#description' => t('Enable this option to output just the URL (i.e. not a clickable link).'),
    );
    $form['absolute'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use absolute link (begins with "http(s)://")'),
      '#default_value' => $this->options['absolute'],
      '#description' => t('Enable this option to output an absolute link.'),
    );
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
      '#states' => array(
        'visible' => array(
          ':input[name="options[url]"]' => array('checked' => FALSE),
        ),
      ),
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

    if ($options['type'] == 'view' && node_access('view', $node)) {
      if ($options['url']) {
        return url("node/$node->nid", array('absolute' => $options['absolute']));
      }
      else {
        $options['alter']['make_link'] = TRUE;
        $options['alter']['absolute'] = $options['absolute'];
        $options['alter']['path'] = "node/$node->nid";
        $text = !empty($options['text']) ? $options['text'] : t('View');
        return $text;
      }
    }
    elseif ($options['type'] == 'edit' && node_access('update', $node)) {
      if ($options['url']) {
        return url("node/$node->nid/edit", array('absolute' => $options['absolute']));
      }
      else {
        $options['alter']['make_link'] = TRUE;
        $options['alter']['absolute'] = $options['absolute'];
        $options['alter']['path'] = "node/$node->nid/edit";
        $options['alter']['query'] = backdrop_get_destination();
        $text = !empty($options['text']) ? $options['text'] : t('Edit');
        return $text;
      }
    }
    elseif ($options['type'] == 'delete' && node_access('delete', $node)) {
      if ($options['url']) {
        return url("node/$node->nid/delete", array('absolute' => $options['absolute']));
      }
      else {
        $options['alter']['make_link'] = TRUE;
        $options['alter']['absolute'] = $options['absolute'];
        $options['alter']['path'] = "node/$node->nid/delete";
        $options['alter']['query'] = backdrop_get_destination();
        $text = !empty($options['text']) ? $options['text'] : t('Delete');
        return $text;
      }
    }
  }
}
