<?php
/**
 * @file
 * Dashboard block displaying information about content types.
 */
class DashboardContentTypesBlock extends Block {
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
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Content types');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Manage content types');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Links to administer content types.');
  }

  /**
   * {@inheritdoc}
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
      foreach ($node_types as $machine => $node_type) {
        // If no types are specified, show all types.
        if (empty($this->settings['types']) || in_array($machine, $this->settings['types'])) {
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

    $panel = array(
      '#theme' => 'dashboard_panel__node_types',
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no content types to display.'),
    );

    if ($access) {
      $panel['link'] = array(
        '#theme' => 'link',
        '#path' => 'admin/structure/types',
        '#text' => t('Manage content types'),
      );
    }

    return $panel;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $node_types = node_type_get_types();
    $type_options = array();
    $type_defaults = array();

    foreach ($node_types as $machine => $node_type) {
      $type_options[$machine] = check_plain(t($node_type->name));
      if (empty($this->settings['types']) || in_array($machine, $this->settings['types'])) {
        $type_defaults[] = $machine;
      }
    }

    $form['types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links for the following content types'),
      '#options' => $type_options,
      '#default_value' => $type_defaults,
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
