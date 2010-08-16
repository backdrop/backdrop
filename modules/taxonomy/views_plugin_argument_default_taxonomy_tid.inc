<?php
// $Id$
/**
 * @file
 *   Taxonomy tid default argument.
 */

class views_plugin_argument_default_taxonomy_tid extends views_plugin_argument_default {
  function option_definition() {
    $options = parent::option_definition();

    $options['term_page'] = array('default' => TRUE);
    $options['node'] = array('default' => FALSE);
    $options['limit'] = array('default' => FALSE);
    $options['vids'] = array('default' => array());

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['term_page'] = array(
      '#type' => 'checkbox',
      '#title' => t('Load default argument from term page'),
      '#default_value' => $this->options['term_page'],
    );
    $form['node'] = array(
      '#type' => 'checkbox',
      '#title' => t('Load default argument from node page, thats good for related taxonomy blocks.'),
      '#default_value' => $this->options['node'],
    );

    $form['limit'] = array(
      '#type' => 'checkbox',
      '#title' => t('Limit terms by vocabulary'),
      '#default_value'=> $this->options['limit'],
      '#process' => array('form_process_checkboxes', 'views_process_dependency'),
      '#dependency' => array(
        'edit-options-argument-default-taxonomy-tid-node' => array(1),
      ),
    );

    $options = array();
    $vocabularies = taxonomy_get_vocabularies();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = check_plain($voc->name);
    }

    $form['vids'] = array(
      '#prefix' => '<div><div id="edit-options-vids">',
      '#suffix' => '</div></div>',
      '#type' => 'checkboxes',
      '#title' => t('Vocabularies'),
      '#options' => $options,
      '#default_value' => $this->options['vids'],
      '#process' => array('form_process_checkboxes', 'views_process_dependency'),
      '#dependency' => array(
        //'edit-options-argument-default-taxonomy-tid-limit' => array(1),
        'edit-options-argument-default-taxonomy-tid-node' => array(1),
      ),
    );
  }

  function options_submit(&$form, &$form_state, &$options) {
    // Clear checkbox values.
    $options['vids'] = array_filter($options['vids']);
  }

  function get_argument() {
    // Load default argument from taxonomy page.
    if (!empty($this->options['term_page'])) {
      if (arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2))) {
        return arg(2);
      }
    }
    // Load default argument from node.
    if (!empty($this->options['node'])) {
      foreach (range(1, 3) as $i) {
        $node = menu_get_object('node', $i);
        if (!empty($node)) {
          break;
        }
      }
      // Just check, if a node could be detected.
      if ($node) {
        $taxonomy = array();
        $fields = field_info_instances('node', $node->type);
        foreach ($fields as $name => $info) {
          $field_info = field_info_field($name);
          if ($field_info['type'] == 'taxonomy_term_reference') {
            $items = field_get_items('node', $node, $name);
            foreach ($items as $item) {
              $taxonomy[$item['tid']] = $item['taxonomy_term'];
            }
          }
        }
        if (!empty($this->options['limit'])) {
          $tids = array();
          // filter by vid
          foreach ($taxonomy as $tid => $term) {
            if (!empty($this->options['vids'][$term->vid])) {
              $tids[] = $tid;
            }
          }
          return implode(",", $tids);
        }
        // Return all tids.
        else {
          return implode(",", array_keys($taxonomy));
        }
      }
    }
  }
}

