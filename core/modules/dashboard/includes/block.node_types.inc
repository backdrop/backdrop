<?php
/**
 * @file
 * Dashboard block displaying informtion about content types.
 */
class DashboardContentTypesBlock extends Block {

  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
    // Set defaults.
    $this->settings += array(
      'types' => array(),
    );
  }

  /**
   * Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Content types');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Manage content types');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Links to administer content types.');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    $node_types = node_type_get_types();
    $access = user_access('administer content types');
    $options = array('destination' => current_path());
    $header = array(
      array('data' => t('Content type')),
      array('data' => t('Operations')),
    );
    $rows = array();

    // Check access, then add a link to create content.
    if ($access) {
      foreach ($node_types as $node_type_key => $node_type) {
        // If no types are specified, show all types.
        if (empty($this->settings['types']) || in_array($node_type_key, $this->settings['types'])) {
          $type_url_string = str_replace('_', '-', $node_type->type);
          $links['configure'] = array(
            'title' => t('Configure'),
            'href' => 'admin/structure/types/manage/' . $type_url_string,
            'query' => $options,
          );
          $links['manage'] = array(
            'title' => t('Manage fields'),
            'href' => 'admin/structure/types/manage/' . $type_url_string . '/fields',
            'query' => $options,
          );
          $links['add'] = array(
            'title' => t('Manage display'),
            'href' => 'admin/structure/types/manage/' . $type_url_string . '/display',
            'query' => $options,
          );
          $operations = array(
            '#type' => 'dropbutton',
            '#links' => $links,
          );
          $rows[] = array(
            'data' => array(
              check_plain(t($node_type->name)),
              backdrop_render($operations),
            ),
          );
        }
      }
    }
    else {
      return array();
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
    if ($access) {
      $link = l(t('Manage content types'), 'admin/structure/types');
    }

    return theme('dashboard_admin_table', array('header' => $header, 'rows' => $rows, 'link' => $link));

  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $node_types = node_type_get_types();
    $type_options = array();
    $type_defaults = array();

    foreach ($node_types as $node_type_key => $node_type) {
      $type_options[$node_type_key] = t($node_type->name);
      if (empty($this->settings['types']) || in_array($node_type_key, $this->settings['types'])) {
        $type_defaults[] = $node_type_key;
      }
    }

    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links for the following content types:'),
      '#options' => $type_options,
      '#default_value' => $type_defaults,
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
