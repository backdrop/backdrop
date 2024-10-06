<?php
/**
 * @file
 * Dashboard block displaying menus on the site.
 */
class DashboardMenuBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $all_menus = module_exists('menu') ? array_keys(menu_get_menus()) : array();
    $excluded_menus = array('management', 'user-menu', 'devel');
    $this->settings += array(
      'menus' => array_diff($all_menus, $excluded_menus),
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Menus');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Manage menus');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Links to manage menus.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!module_exists('menu')) {
      return;
    }

    $menus = menu_get_menus();
    $current_path = current_path();
    $options = array('destination' => $current_path);
    $header = array(
      array('data' => t('Menu')),
      array('data' => t('Operations')),
    );
    $rows = array();

    if (user_access('administer menu')) {
      foreach ($menus as $machine => $menu_label) {
        if (in_array($machine, $this->settings['menus'])) {
          $links['manage'] = array(
            'title' => t('Edit links'),
            'href' => 'admin/structure/menu/manage/' . $machine,
            'query' => $options,
          );
          $links['add'] = array(
            'title' => t('Add new link'),
            'href' => 'admin/structure/menu/manage/' . $machine . '/add',
            'query' => $options,
          );
          $links['configure'] = array(
            'title' => t('Configure'),
            'href' => 'admin/structure/menu/manage/' . $machine . '/edit',
            'query' => $options,
          );
          $operations = array(
            '#type' => 'dropbutton',
            '#links' => $links,
          );
          $rows[] = array(
            'data' => array(
              check_plain(t($menu_label)),
              backdrop_render($operations),
            ),
          );
        }
      }
    }

    if (empty($rows) && !empty($menus)) {
      // If there are existing menus, but user has no access to manage any of
      // them, hide the block completely.
      return;
    }

    $panel = array(
      '#theme' => 'dashboard_panel__menus',
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no menus to display.'),
    );

    if (user_access('administer menu')) {
      $panel['link'] = array(
        '#theme' => 'link',
        '#path' => 'admin/structure/menu',
        '#text' => t('Manage menus'),
      );
    }

    return $panel;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    if (!module_exists('menu')) {
      return $form;
    }


    $menus = menu_get_menus();
    foreach ($menus as $menu_key => $menu_label) {
      $menus[$menu_key] = check_plain(t($menu_label));
    }

    $form['menus'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links for the following menus'),
      '#options' => $menus,
      '#default_value' => $this->settings['menus'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['menus'] = array_keys(array_filter($form_state['values']['menus']));
  }
}
