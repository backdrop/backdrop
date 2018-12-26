<?php

  /**
   * @file
   *
   * Administrative block for displaying available menus on the site.
   */


class DashboardMenuBlock extends Block {


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
    return t('Manage Menus');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Links to administer menus.');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    if (!module_exists('menu')) {
      return;
    }
    $items = array();

    $menus = menu_get_menus();
    $options = array('destination' => 'admin/dashboard');
    $header = array(
      array('data' => t('Menu')),
      array(
        'data' => t('Operations'),
      ),
    );
    $rows = array();

    if (user_access('administer menu')) {
      foreach ($menus as $menu_name => $menu) {
        if (array_key_exists($menu_name, $this->settings['menus'])) {
          if ($this->settings['menus'][$menu_name] === $menu_name) {
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
                t($menu),
                backdrop_render($operations),
              ),
            );
          }
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
      $link = l(t('Menu administration'), 'admin/structure/menu');
    }

    $content = theme('dashboard_admin_table', array('header' => $header, 'rows' => $rows, 'link' => $link));

    return $content;
  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    if (!module_exists('menu')) {
      return $form;
    }

    $menus = menu_get_menus();
    foreach ($menus as $machine => $name) {
      if (!in_array($machine, array('management', 'user-menu', 'devel'))) {
        $menu_defaults[$machine] = $machine;
      }
    }
    $form['menus'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Show links for the following menus on the dashboard'),
      '#options' => $menus,
      '#default_value' => ($this->settings['menus']) ? $this->settings['menus'] : $menu_defaults,
    );
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    foreach (array_keys($form_state['values']['menus']) as $key) {
      $this->settings['menus'][$key] = $form_state['values']['menus'][$key];
    }
  }
}
