<?php

  /**
   * @file
   *
   * "Create content" panels content type. It shows users links to
   * create new content of all types for which they have permissions,
   * and users with "administer content types" permission an optional
   * "Configure" link as well.
   *
   */

class DashboardContentTypesBlock extends DashboardBlock {

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Manage Content types');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Links to administer content types.');
  }

  /**
   * Run-time rendering of the body of the pane.
   */
  function getContent() {
    $types = node_type_get_types();
    $access = user_access('administer content types');
    $options = array('destination' => 'admin/dashboard');
    $header = array(
      array('data' => t('Content type')),
      array(
        'data' => t('Operations'),
      ),
    );
    $rows = array();

    foreach ($types as $type => $object) {
        // this->settingsig data says to include the type.
        if ((!array_key_exists($type, $this->settings['types'])) || (isset($this->settings['types']) && $this->settings['types'][$type]) == $type) {
          // Check access, then add a link to create content.
          if ($access) {
            $type_url_str = str_replace('_', '-', $object->type);
            $links['configure'] = array(
              'title' => t('Configure'),
              'href' => 'admin/structure/types/manage/' . $type_url_str,
              'query' => $options,
            );
            $links['manage'] = array(
              'title' => t('Manage fields'),
              'href' => 'admin/structure/types/manage/' . $type_url_str . '/fields',
              'query' => $options,
            );
            $links['add'] = array(
              'title' => t('Manage display'),
              'href' => 'admin/structure/types/manage/' . $type_url_str . '/display',
              'query' => $options,
            );
            $operations = array(
              '#type' => 'dropbutton',
              '#links' => $links,
            );
            $rows[] = array(
              'data' => array(
                t($object->name),
                backdrop_render($operations),
              ),
            );
          }
        }
      }

    if (empty($rows)) {
        $rows[] = array(
          array(
            'data' => t('There are no content types to display.'),
            'colspan' => 4,
          ),
        );
      }

    $link = '';
      if (user_access('administer content types')) {
        $link = l(t('Content type administration'), 'admin/structure/types');
      }

    return theme('dashboard_admin_table', array('header' => $header, 'rows' => $rows, 'link' => $link));

  }

  /**
   * 'Edit form' callback for the content type.
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
