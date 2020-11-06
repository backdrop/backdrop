<?php
/**
 * @file
 * Dashboard block providing a welcome message, and links to get people started
 * using Backdrop.
 */
class DashboardWelcomeBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Welcome to Backdrop CMS!');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Welcome to Backdrop CMS!');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Welcome message, and links to getting-started tasks.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    $build = array(
      '#theme' => 'dashboard_panel__welcome',
    );
    $build['welcome'] = array(
      '#type' => 'help',
      '#markup' => t('Here are some links to help get you started:'),
    );

    $build['steps'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('dashboard-welcome-steps', 'row'),
      ),
    );

    // Add the first column of links.
    $links_col1 = array();
    $links_col1['<front>'] = t('View the home page');
    $links_col1['admin/config/system/site-information'] = t('Add a logo or change the site name');
    $default_theme = config_get('system.core', 'theme_default');
    if (theme_has_settings($default_theme) || (module_exists('color') && theme_has_color_support($default_theme))) {
      $links_col1['admin/appearance/settings/' . $default_theme] = t('Customize the current theme');
    }
    if (module_exists('installer')) {
      $links_col1['admin/appearance/install'] = t('Find a new theme for your site');
    }
    $links_col1 = $this->filterAvailableLinks($links_col1);

    if ($links_col1) {
      $welcome_start_content = theme('item_list', array('items' => $links_col1));
      $build['steps']['start'] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('dashboard-welcome-step', 'dashboard-welcome-start', 'col-md-4'),
        ),
      );
      $build['steps']['start']['content'] = array(
        '#type' => 'markup',
        '#prefix' => '<h4>' . t('Get started') . '</h4>',
        '#markup' => $welcome_start_content,
      );
    }

    // Add the second column of links.
    $links_col2 = array();
    $about_node = node_load(2);
    if ($about_node) {
      $links_col2['node/2/edit'] = t('Edit the %title page', array('%title' => $about_node->title));
    }
    $node_types = node_type_get_types();
    if (isset($node_types['post'])) {
      $links_col2['node/add/post'] = t('Create a new @post_label', array('@post_label' => $node_types['post']->name));
    }
    $system_menus = menu_list_system_menus();
    if (isset($system_menus['main-menu'])) {
      $links_col2['admin/structure/menu/manage/main-menu'] = t('Update the %menu_name menu', array('%menu_name' => $system_menus['main-menu']));
    }
    $home_page_path = config_get('system.core', 'site_frontpage');
    $home_layout = layout_load('home');
    if ($home_layout && !$home_layout->disabled && $home_page_path == 'home') {
      $links_col2['admin/structure/layouts/manage/home'] = t('Modify the layout for your home page');
    }
    $links_col2 = $this->filterAvailableLinks($links_col2);
    if ($links_col2) {
      $welcome_next_content = theme('item_list', array('items' => $links_col2));
      $build['steps']['next'] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('dashboard-welcome-step', 'dashboard-welcome-next', 'col-md-4'),
        ),
      );
      $build['steps']['next']['content'] = array(
        '#type' => 'markup',
        '#prefix' => '<h4>' . t('Next steps') . '</h4>',
        '#markup' => $welcome_next_content,
      );
    }

    // Add the third column of links.
    $links_col3 = array();
    $links_col3['admin/modules'] = t('Turn existing modules on or off');
    if (module_exists('installer')) {
      $links_col3['admin/modules/install'] = t('Add new modules for more functionality');
    }
    $links_col3['https://backdropcms.org/user-guide'] = t('Read the online user guide');
    $links_col3['https://forum.backdropcms.org'] = t('Visit the Backdrop CMS Forum');
    $links_col3 = $this->filterAvailableLinks($links_col3);

    if ($links_col3) {
      $welcome_more_content = theme('item_list', array('items' => $links_col3));
      $build['steps']['more'] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('dashboard-welcome-step', 'dashboard-welcome-more', 'col-md-4'),
        ),
      );
      $build['steps']['more']['content'] = array(
        '#type' => 'markup',
        '#prefix' => '<h4>' . t('More actions') . '</h4>',
        '#markup' => $welcome_more_content,
      );
    }

    return $build;
  }

  /**
   * Checks access to paths and returns an HTML list of links.
   *
   * @param array $links
   *   An array with each key being a path and each value being a label.
   *
   * @return array
   *   An array of HTML anchor tags for items the user may access.
   */
  private function filterAvailableLinks(array $links) {
    $html_links = array();
    foreach ($links as $path => $label) {
      if (url_is_external($path) || $path === '<front>') {
        if ($path !== '<front>') {
          $label .= '<span class="dashboard-external"></span>';
        }
        $access = TRUE;
      }
      else {
        $router_item = menu_get_item($path);
        $access = $router_item && $router_item['access'];
      }
      if ($access) {
        $html_links[] = l($label, $path, array('html' => TRUE));
      }
    }
    return $html_links;
  }
}
