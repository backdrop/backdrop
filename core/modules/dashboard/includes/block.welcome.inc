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
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Welcome to Backdrop!');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    return t('Welcome to Backdrop!');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
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
      '#markup' => t('Here are some links to get you started:'),
    );

    $build['steps'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('dashboard-welcome-steps', 'row'),
      ),
    );

    $get_started_links = array();
    $get_started_links['<front>'] = t('View your site');
    $get_started_links['admin/appearance/settings/basis'] = t('Customize the look of your site');
    $get_started_links['admin/structure/layouts/manage/home'] = t('Configure the layout of your home page');
    $get_started_links['admin/appearance'] = t('Change the theme used by your site');
    $get_started_links['admin/appearance/install'] = t('Find more themes to install');
    $get_started_links = $this->filterAvailableLinks($get_started_links);

    if ($get_started_links) {
      $welcome_start_content = theme('item_list', array('items' => $get_started_links));
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

    $next_steps_links = array();
    $next_steps_links['node/add/post'] = t('Create a new post');
    $about_node = node_load(2);
    if ($about_node) {
      $next_steps_links['node/2/edit'] = t('Edit the %title page', array('%title' => $about_node->title));
    }
    $next_steps_links['admin/structure/menu/manage/main-menu'] = t('Manage the main menu');
    $next_steps_links['admin/config/system/site-information'] = t('Update your site information');
    $next_steps_links = $this->filterAvailableLinks($next_steps_links);

    if ($next_steps_links) {
      $welcome_next_content = theme('item_list', array('items' => $next_steps_links));
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

    $more_actions_links = array();
    $more_actions_links['admin/modules'] = t('Turn existing features on or off');
    $more_actions_links['admin/modules/install'] = t('Add more functionality');
    $more_actions_links['https://backdropcms.org/user-guide'] = t('Read the online user guide');
    $more_actions_links['https://forum.backdropcms.org'] = t('Visit the Backdrop CMS Forum');
    $more_actions_links = $this->filterAvailableLinks($more_actions_links);

    if ($more_actions_links) {
      $welcome_more_content = theme('item_list', array('items' => $more_actions_links));
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
