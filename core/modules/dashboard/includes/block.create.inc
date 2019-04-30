<?php
/**
 * @file
 *
 * Dashboard block providing links to create new content.
 */
class DashboardCreateBlock extends Block {

  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $this->settings += array(
      'types' => array(),
    );
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Create content');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Content creation links');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Links to create new content.');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    $types = node_type_get_types();
    $no_access = FALSE;
    $create = array();

    foreach ($types as $type => $object) {
      // If enabled or all types are enabled.
      if (in_array($type, $this->settings['types']) || empty($this->settings['types'])) {
        // Check access, then add a link to create content.
        if (node_access('create', $type)) {
          $type_url_str = str_replace('_', '-', $object->type);
          $create[] = l(t('Add new @type', array('@type' => t($object->name))), 'node/add/' . $type_url_str);
        }
        else {
          $no_access = TRUE;
        }
      }
    }

    // If there are existing content types, but user has no access to create any
    // of them, hide the block completely.
    if (empty($create)) {
      if ($no_access) {
        return array();
      }
      return t('No content types have been configured');
    }
    $content =  theme('dashboard_create', array('create' => $create));

    return $content;
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $type_options = node_type_get_names();
    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Include "Create" links for content types'),
      '#options' => array_map('t', $type_options),
      // All types are enabled if types is an empty array.
      '#default_value' => $this->settings['types'] ? $this->settings['types'] : array_keys($type_options),
    );
  }

  /**
   * Submit handler to save the form settings.
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
