<?php

  /**
   * @file
   *
   * "Create content" dashboard block. It shows users links to
   * create new content of all types for which they have permissions,
   * and users with "administer content types" permission an optional
   * "Configure" link as well.
   *
   */

class DashboardCreateBlock extends DashboardBlock {

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
   * Run-time rendering of the body of the pane.
   */
  function getContent() {
    $types = node_type_get_types();
    $create = array();

    foreach ($types as $type => $object) {
        // Check against pane this->settings for type.
        if ((!array_key_exists($type, $this->settings['types'])) || (isset($this->settings['types']) && $this->settings['types'][$type])) {
          // Check access, then add a link to create content.
          if (node_access('create', $type)) {
            $type_url_str = str_replace('_', '-', $object->type);
            $create[] = l(t('Add new  ' . $object->name), 'node/add/' . $type_url_str);
          }
        }
      }

    $content =  theme('dashboard_create', array('create' => $create));

    return $content;
  }


  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $types = node_type_get_types();
      $type_options = array();
      $type_defaults = array();

    if (isset($this->settings['types'])) {
        $type_defaults = $this->settings['types'];
      }

    foreach ($types as $type => $object) {
        $type_options[$type] = $object->name;
        if (!array_key_exists($type, $type_defaults)) {
          $type_defaults[$type] = $type;
        }
      }

    $form['types'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Include Create links for Content Types'),
        '#options' => $type_options,
        '#default_value' => $type_defaults,
      );

  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    foreach (array_keys($form_state['values']['types']) as $key) {
      $this->settings['types'][$key] = $form_state['values']['types'][$key];
    }
  }
}
