<?php
/**
 * @file
 * Dashboard block displaying menus on the site.
 */
class DashboardMenuBlock extends Block {

  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $all_menus = array_keys(menu_get_menus());
    $excluded_menus = array('management', 'user-menu', 'devel');
    $this->settings += array(
      'menus' => array_diff($all_menus, $excluded_menus),
    );
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Menus');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Manage menus');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Links to manage menus.');
  }

  /**
   * Run-time rendering of the body of the block.
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
      array(
        'data' => t('Operations'),
      ),
    );
    $rows = array();

    if (user_access('administer menu')) {
      foreach ($menus as $menu_name => $menu_label) {
        if (in_array($menu_name, $this->settings['menus'])) {
          $links['configure'] = array(
            'title' => t('Configure'),
            'href' => 'admin/structure/menu/manage/' . $menu_name . '/edit',
            'query' => $options,
          );
          $links['manage'] = array(
            'title' => t('Manage links'),
            'href' => 'admin/structure/menu/manage/' . $menu_name,
            'query' => $options,
          );
          $links['add'] = array(
            'title' => t('Add new link'),
            'href' => 'admin/structure/menu/manage/' . $menu_name . '/add',
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

    if (empty($rows)) {
      // If there are existing menuse, but user has no access to manage any
      // of them, hide the block completely.
      if (!empty($menus)) {
        return array();
      }
      $rows[] = array(
        array(
          'data' => t('There are no menus to display.'),
          'colspan' => 4,
        ),
      );
    }

    // Build a link to the menu admin UI.
    $link = '';
    if (user_access('administer menu')) {
      $link = l(t('Manage menus'), 'admin/structure/menu');
    }

    $content = theme('dashboard_admin_table', array('header' => $header, 'rows' => $rows, 'link' => $link));

    return $content;
  }

  /**
   * 'Edit form' callback for the block.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    if (!module_exists('menu')) {
      return $form;
    }

    $form['menus'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display links for the following menus:'),
      '#options' => menu_get_menus(),
      '#default_value' => $this->settings['menus'],
    );
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['menus'] = array_keys(array_filter($form_state['values']['menus']));
  }
}
