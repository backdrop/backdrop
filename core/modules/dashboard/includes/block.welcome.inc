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
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);
  }

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
    $get_started_links[] = l(t('View your site'), '<front>');
    $get_started_links[] = l(t('Customize the look of your site'), 'admin/appearance/settings/basis');
    $get_started_links[] = l(t('Configure the layout of your home page'), 'admin/structure/layouts/manage/home');
    $get_started_links[] = l(t('Change the theme used by your site'), 'admin/appearance');
    $get_started_links[] = l(t('Find more themes to install'), 'admin/appearance/install');

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

    $next_steps_links = array();
    $next_steps_links[] = l(t('Write your first article'), 'node/add/post');
    $next_steps_links[] = l(t('Edit the <em>About</em> page'), 'node/2/edit', array('html' => TRUE));
    $next_steps_links[] = l(t('Manage the main menu'), 'admin/structure/menu/manage/main-menu');
    $next_steps_links[] = l(t('Update your site information'), 'admin/config/system/site-information');

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

    $more_actions_links = array();
    $more_actions_links[] = l(t('Turn existing features on or off'), 'admin/modules');
    $more_actions_links[] = l(t('Add more functionality'), 'admin/modules/install');
    $more_actions_links[] = l(t('Read the online user guide'), 'https://backdropcms.org/user-guide');
    $more_actions_links[] = l(t('Visit the Backdrop CMS Forum'), 'https://forum.backdropcms.org');

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

    return $build;
  }

}
