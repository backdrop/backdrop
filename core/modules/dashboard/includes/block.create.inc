<?php
/**
 * @file
 * Dashboard block providing links to create new content.
 */
class DashboardCreateBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $this->settings += array(
      'types' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Create content');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Content creation links');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Links to create new content.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    $node_types = node_type_get_types();
    $no_access = TRUE;
    $create = array();

    foreach ($node_types as $machine => $node_type) {
      // If enabled or all types are enabled.
      if (in_array($machine, $this->settings['types']) || empty($this->settings['types'])) {
        // Check access, then add a link to create content.
        if (node_access('create', $machine)) {
          $type_url_str = str_replace('_', '-', $node_type->type);
          $create[] = l(t('Add new @type', array('@type' => t($node_type->name))), 'node/add/' . $type_url_str, array('html' => TRUE));
          $no_access = FALSE;
        }
      }
    }

    // If there are existing content types, but user has no access to create any
    // of them, hide the block completely.
    if ($no_access) {
      return array();
    }

    $build = array(
      '#theme' => 'dashboard_panel__create',
    );
    if (empty($create)) {
      $build['list'] = array(
        '#type' => 'markup',
        '#markup' => t('No content types have been configured yet.'),
      );
    }
    else {
      $build['list'] = array(
        '#theme' => 'item_list',
        '#items' => $create,
      );
    }

    return $build;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $options = node_type_get_names();
    foreach ($options as $machine_name => $type_label) {
      $options[$machine_name] = check_plain(t($type_label));
    }

    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links to create content of the following types'),
      '#options' => $options,
      // All types are enabled if types is an empty array.
      '#default_value' => $this->settings['types'] ? $this->settings['types'] : array_keys($options),
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    // If all types are checked, save nothing, indicating all types are allowed.
    $checked_types = array_filter($form_state['values']['types']);
    if (count($form['types']['#options']) === count($checked_types)) {
      $this->settings['types'] = array();
    }
    else {
      $this->settings['types'] = array_values($checked_types);
    }
  }
}
