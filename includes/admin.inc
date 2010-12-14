<?php
// $Id: admin.inc,v 1.161.4.63 2010/11/21 00:27:59 dereine Exp $
/**
 * @file admin.inc
 * Provides the Views' administrative interface.
 */

/**
 * Page callback to list available views.
 *
 * @return
 *   A renderable page build array.
 */
function views_ui_list_views() {
  // @todo Find a way to load the views-admin.seven.css only when Seven is the
  // admin theme
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.css');
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/ie/views-admin.ie7.css', 
    array(
      'group' => CSS_DEFAULT,
      'browsers' => array(
        'IE' => 'lte IE 7',
        '!IE' => FALSE
      ),
      'preprocess' => FALSE,
    )
  );
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.theme.css', 
    array(
      'group' => CSS_DEFAULT,
      'preprocess' => TRUE,
    )
  );
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.seven.css', 
    array(
      'group' => CSS_THEME,
      'preprocess' => TRUE,
    )
  );

  $views = views_get_all_views();
  foreach ($views as $view) {
    // Exclude code-based and disabled views from the listing, until we have
    // a UI to deal with them.
    if ($view->type == t('Default') || !empty($view->disabled)) {
      continue;
    }

    // Add a row to the table listing.
    $rows[] = array(
      array(
        'data' => array(
          '#theme' => 'views_ui_view_info',
          '#title' => check_plain($view->name),
          '#paths' => _views_ui_get_paths($view),
          '#displays' => _views_ui_get_displays_list($view),
        ),
      ),
      array(
        'data' => '<div class="views-ui-view-description">' . check_plain($view->description) . '</div>',
      ),
      array(
        'data' => array(
          '#theme' => 'links',
          '#links' => _views_ui_get_operation_links($view),
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      )
    );
  }

  $build = array(
    '#theme' => 'table',
    '#header' => array(
      t('View name'),
      t('Description'),
      t('Operations'),
    ),
    '#rows' => $rows,
    '#empty' => t('No views found.'),
  );
  return $build;
}

/**
 * Check to see if the advanced help module is installed, and if not put up
 * a message.
 *
 * Only call this function if the user is already in a position for this to
 * be useful.
 */
function views_ui_check_advanced_help() {
  if (variable_get('views_hide_help_message', FALSE)) {
    return;
  }

  if (!module_exists('advanced_help')) {
    $filename = db_query_range("SELECT filename FROM {system} WHERE type = 'module' AND name = 'advanced_help'", 0, 1)
      ->fetchField();
    if ($filename && file_exists($filename)) {
      drupal_set_message(t('If you <a href="@modules">enable the advanced help module</a>, Views will provide more and better help. <a href="@hide">Hide this message.</a>', array('@modules' => url('admin/modules'),'@hide' => url('admin/structure/views/tools'))));
    }
    else {
      drupal_set_message(t('If you install the advanced help module from !href, Views will provide more and better help. <a href="@hide">Hide this message.</a>', array('!href' => l('http://drupal.org/project/advanced_help', 'http://drupal.org/project/advanced_help'), '@hide' => url('admin/structure/views/tools'))));
    }
  }
}

/**
 * Page callback for the live preview.
 *
 * @todo make this use a template
 */
function views_ui_preview($js, $view) {
  // Take off the items we know so that we can have just the args passed
  // in for later use.
  $func_args = func_get_args();
  array_shift($func_args); // $js
  array_shift($func_args); // $view
  $display_id = (count($func_args)) ? array_shift($func_args) : 'default';

  $form_state = array(
    'display_id' => $display_id,
    'view_args' => $func_args ? implode('/', $func_args) : '',
    'rerender' => TRUE,
    'no_redirect' => TRUE,
    'view' => &$view,
    'ajax' => $js,
    'build_info' => array(
      'args' => array(),
    ),
  );

  $output = drupal_render(drupal_build_form('views_ui_preview_form', $form_state));
  $args = array();
  if (isset($form_state['view_args']) && $form_state['view_args'] !== '') {
    $args = explode('/', $form_state['view_args']);
  }

  $errors = $view->validate();
  if ($errors === TRUE) {
    $view->ajax = $js;
    $view->live_preview = TRUE;
    $view->set_exposed_input($_POST);

    // Store the current view URL for later use:
    $view->set_display($form_state['display_id']);
    $view->set_arguments($args);

    if ($view->display_handler->get_option('path')) {
      $path = $view->get_url();
    }

    // Make view links come back to preview.
    $view->override_path = 'admin/structure/views/nojs/preview/' . $view->name . '/' . $form_state['display_id'];

    // also override $_GET['q'] so we get the pager
    $_GET['q'] = $view->override_path;
    if ($form_state['view_args']) {
      $_GET['q'] .= '/' . $form_state['view_args'];
    }

    $preview = $view->preview($form_state['display_id'], $args);

    // Get information from the preview for display.
    if (!empty($view->build_info['query'])) {
      $rows = array();
      $query = $view->build_info['query'];
      $rows[] = array('<strong>' . t('Query') . '</strong>', '<pre>' . check_plain(strtr($query, $query->getArguments())) . '</pre>');
      if (!empty($view->additional_queries)) {
        $queries = '<strong>' . t('These queries were run during view rendering:') . '</strong>';
        foreach ($view->additional_queries as $query) {
          if ($queries) {
            $queries .= "\n";
          }
          $queries .= t('[@time ms]', array('@time' => intval($query[1] * 100000) / 100)) . ' ' . $query[0];
        }

        $rows[] = array('<strong>' . t('Other queries') . '</strong>', '<pre>' . $queries . '</pre>');
      }

      $rows[] = array('<strong>' . t('Title') . '</strong>', filter_xss_admin($view->get_title()));
      if (isset($path)) {
        $path = l($path, $path);
      }
      else {
        $path = t('This display has no path.');
      }

      $rows[] = array('<strong>' . t('Path') . '</strong>', $path);

      $rows[] = array('<strong>' . t('Query build time') . '</strong>', t('@time ms', array('@time' => intval($view->build_time * 100000) / 100)));
      $rows[] = array('<strong>' . t('Query execute time') . '</strong>', t('@time ms', array('@time' => intval($view->execute_time * 100000) / 100)));
      $rows[] = array('<strong>' . t('View render time') . '</strong>', t('@time ms', array('@time' => intval($view->render_time * 100000) / 100)));
      drupal_alter('views_preview_info', $rows, $view);

      $info = theme('table', array('rows' => $rows));
    }
    else {
      $info = theme('table', array('rows' => array(array('<strong>' . t('Query') . '</strong>', t('No query was run')))));
    }
  }
  else {
    foreach ($errors as $error) {
      drupal_set_message($error, 'error');
    }
    $preview = t('Unable to preview due to validation errors.');
    $info = '';
  }

  $info = '<div class="views-query-info">' . $info . '</div>';

  if (variable_get('views_ui_query_on_top', FALSE)) {
    $output .= $info . $preview;
  }
  else {
    $output .= $preview . $info;
  }

  if (!$js) {
    views_add_css('views-admin');
    drupal_set_title($view->get_title());
    return $output;
  }
  else {
    $commands = array();
    if (!empty($view->js_settings)) {
      $commands[] = ajax_command_settings($view->js_settings);
    }
    $display = '';
    if ($messages = theme('status_messages')) {
      $display = '<div class="views-messages">' . $messages . '</div>';
    }
    $display .= $output;
    if ($display) {
      $commands[] = ajax_command_html('div#views-live-preview', $display);
    }
    return $js ? array('#type' => 'ajax', '#commands' => $commands) : $commands;
  }
}

/**
 * Form for generating argument information for the live preview.
 */
function views_ui_preview_form($form, &$form_state) {
  $view = &$form_state['view'];
  $view->init_display();
  $options = array();
  foreach ($view->display as $id => $display) {
    $options[$id] = $display->display_title;
  }

  $form['#attributes'] = array(
    'class' => array('clearfix'),
  );

  $form['display_id'] = array(
    '#type' => 'select',
    '#title' => t('Display'),
    '#options' => $options,
    '#default_value' => $form_state['display_id'],
    '#id' => 'preview-display-id',
  );

  $form['args'] = array(
    '#type' => 'textfield',
    '#title' => t('Arguments'),
    '#default_value' => $form_state['view_args'],
    '#description' => t('Separate arguments with a / as though they were a URL path.'),
    '#id' => 'preview-args',
  );

  $form['preview'] = array(
    '#type' => 'submit',
    '#value' => t('Preview'),
    '#id' => 'preview-submit',
  );

  $form['live_preview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatic live preview'),
    '#default_value' => !variable_get('views_ui_disable_live_preview', 0),
  );

  $form['#action'] = url("admin/structure/views/nojs/preview/$view->name");
  return $form;
}

/**
 * Submit the preview form.
 *
 * This just takes the data and stores it on the form state in a
 * known location. The caller will be responsible for using it.
 */
function views_ui_preview_form_submit(&$form, &$form_state) {
  $form_state['display_id'] = $form_state['values']['display_id'];
  $form_state['view_args'] = $form_state['values']['args'];
}

/**
 * Page callback to add a new view.
 */
function views_ui_add_page() {
  // @todo Find a way to load the views-admin.seven.css only when Seven is the
  // admin theme
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.css');
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/ie/views-admin.ie7.css', 
    array(
      'group' => CSS_DEFAULT,
      'browsers' => array(
        'IE' => 'lte IE 7',
        '!IE' => FALSE
      ),
      'preprocess' => FALSE,
    )
  );
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.theme.css', 
    array(
      'group' => CSS_DEFAULT,
      'preprocess' => TRUE,
    )
  );
  drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.seven.css', 
    array(
      'group' => CSS_THEME,
      'preprocess' => TRUE,
    )
  );
  return drupal_get_form('views_ui_add_form');
}

/**
 * Page callback to add a new view.
 */
function views_ui_clone_page($view) {
  $form_state = array(
    'view' => $view->copy(),
    'build_info' => array(
      'args' => array(),
    ),
  );

  drupal_set_title(t('Clone view %view', array('%view' => $view->name)), PASS_THROUGH);
  return drupal_render(drupal_build_form('views_ui_add_form', $form_state));
}

/**
 * Form builder for the "add new view" page.
 */
function views_ui_add_form($form, &$form_state) {
  $form['#attached']['js'][] = drupal_get_path('module', 'views_ui') . '/js/views-admin.js';
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('View name'),
    '#required' => TRUE,
    '#size' => 32,
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
    '#default_value' => '',
    '#maxlength' => 255,
  );
  $form['name'] = array(
    '#type' => 'machine_name',
    '#maxlength' => 32,
    '#machine_name' => array(
      'exists' => 'views_get_view',
      'source' => array('description'),
    ),
    '#description' => t('A unique machine-readable name for this View. It must only contain lowercase letters, numbers, and underscores.'),
  );
  $form['show'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#attributes' => array('class' => array('inline')),
  );
  
  $wizard_plugins = views_ui_get_wizards();
  $options = array();
  foreach ($wizard_plugins as $key => $w) {
    $options[$key] = $w['title'];
  }
  $default = isset($options['node:blog']) ? 'node:blog' : 'node';
  $form['show']['wizard_key'] = array(
    '#type' => 'select',
    '#title' => t('Show'),
    '#options' => $options,
    '#default_value' => $default,
    '#attributes' => array('class' => array('inline')),
    '#ajax' => array(
      'callback' => 'views_ui_add_form_update',
      'wrapper' => 'edit-view-displays-wrapper',
    ),
  );

  // Everything that can be updated by AJAX goes in here. It is triggered by
  // the "Show" dropdown above.
  $form['displays'] = array(
    '#prefix' => '<div id="edit-view-displays-wrapper">',
    '#suffix' => '</div>',
  );

  // Since the "Show" dropdown can't trigger a form submission when
  // JavaScript is disabled, add a submit button to do that. CSS hides
  // this button when JavaScript is enabled.
  $form['show']['update_wizard_key'] = array(
    '#type' => 'submit',
    '#value' => t('Update displays'),
    '#limit_validation_errors' => array(
      array('show'),
    ),
    '#submit' => array('views_ui_show_nojs_submit'),
  );

  $wizard_key = isset($form_state['values']['show']['wizard_key']) ? $form_state['values']['show']['wizard_key'] : $form['show']['wizard_key']['#default_value'];
  $get_instance = $wizard_plugins[$wizard_key]['get_instance'];
  $wizard_instance = $get_instance($wizard_plugins[$wizard_key]);
  $form = $wizard_instance->build_form($form, $form_state);

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('views_ui_wizard_form_validate'),
    '#submit' => array('views_ui_add_form_save_submit'),
  );
  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#validate' => array('views_ui_wizard_form_validate'),
    '#submit' => array('views_ui_add_form_store_edit_submit'),
  );
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('views_ui_add_form_cancel_submit'),
  );

  return $form;
}

/**
 * Update the add view form.
 */
function views_ui_show_nojs_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Update the add view for via AJAX.
 *
 * @return
 *   The part of the form that has changed.
 */
function views_ui_add_form_update($form, $form_state) {
  return $form['displays'];
}

/**
 * Validate the add view form.
 */
function views_ui_wizard_form_validate($form, &$form_state) {
  $w = views_ui_get_wizard($form_state['values']['show']['wizard_key']);
  $form_state['wizard'] = $w;
  $get_instance = $w['get_instance'];
  $form_state['wizard_instance'] = $get_instance($w);
  $errors = $form_state['wizard_instance']->validate($form, $form_state);
  foreach ($errors as $name => $message) {
    form_set_error($name, $message);
  }
}

/**
 * Process the add view form, 'save'.
 */
function views_ui_add_form_save_submit($form, &$form_state) {
  try {
    $view = $form_state['wizard_instance']->create_view($form, $form_state);
  }
  catch (ViewsWizardException $e) {
    drupal_set_message($e->getMessage(), 'error');
    $form_state['redirect'] = 'admin/structure/views';
  }
  $view->save();
  menu_rebuild();
  cache_clear_all('*', 'cache_views', TRUE);
  cache_clear_all();
  $form_state['redirect'] = 'admin/structure/views';
  if (!empty($view->display['page'])) {
    $display = $view->display['page'];
    if ($display->handler->has_path()) {
      $one_path = $display->handler->get_option('path');
      if (strpos($one_path, '%') === FALSE) {
        $form_state['redirect'] = $one_path;  // PATH TO THE VIEW IF IT HAS ONE
        return;
      }
    }
  }
  drupal_set_message(t('Your view was saved. You may edit it from the list below.'));
}

/**
 * Process the add view form, 'continue'.
 */
function views_ui_add_form_store_edit_submit($form, &$form_state) {
  try {
    $view = $form_state['wizard_instance']->create_view($form, $form_state);
  }
  catch (ViewsWizardException $e) {
    drupal_set_message($e->getMessage(), 'error');
    $form_state['redirect'] = 'admin/structure/views';
  }
  // Just cache it temporarily to edit it.
  views_ui_cache_set($view);
  $form_state['redirect'] = 'admin/structure/views/view/' . $view->name;
}

/**
 * Cancel the add view form.
 */
function views_ui_add_form_cancel_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/views';
}

/**
 * Helper function to get a list of available operation links for a given view.
 *
 * @param $view
 *   The view.
 *
 * @return
 *   An array of links as expected by theme_links().
 */
function _views_ui_get_operation_links($view) {
  $links = array();

  if ($view->type != t('Default')) {
    $text = $view->type == t('Overridden') ? t('revert') : t('delete');
    $links[] = array('title' => $text, 'href' => "admin/structure/views/view/$view->name/delete");
  }
  else {
    if (empty($view->disabled)) {
      $links[] = array('title' => t('disable'), 'href' => "admin/structure/views/view/$view->name/disable", 'query' => drupal_get_destination());
    }
    else {
      $links[] = array('title' => t('enable'), 'href' => "admin/structure/views/view/$view->name/enable", 'query' => drupal_get_destination());
    }
  }

  if (empty($view->disabled)) {
    $links[] = array('title' => t('duplicate'), 'href' => "admin/structure/views/view/$view->name/duplicate");
    $links[] = array('title' => t('edit'), 'href' => "admin/structure/views/view/$view->name/edit");
  }

  return $links;
}

/**
 * Helper function to get a list of paths assigned to a view.
 *
 * @param $view
 *   The view.
 *
 * @return
 *   An array of links to this view's display paths.
 */
function _views_ui_get_paths($view) {
  $all_paths = array();
  if (empty($view->display)) {
    $all_paths[] = t('Edit this view to add a display.');
  }
  else {
    $view->init_display();   // Make sure all the handlers are set up
    foreach ($view->display as $display) {
      if (!empty($display->handler) && $display->handler->has_path()) {
        $one_path = $display->handler->get_option('path');
        if (empty($path_sort)) {
          $path_sort = strtolower($one_path);
        }
        if (empty($view->disabled) && strpos($one_path, '%') === FALSE) {
          $all_paths[] = l($one_path, $one_path);
        }
        else {
          $all_paths[] = check_plain($one_path);
        }
      }
    }
  }

  return array_unique($all_paths);
}

/**
 * Helper function to get a list of displays included in a view.
 *
 * @param $view
 *   The view.
 *
 * @return
 *   An array of display types that this view includes.
 */
function _views_ui_get_displays_list($view) {
  $displays = array();
  foreach ($view->display as $display) {
    if (!empty($display->handler->definition['admin'])) {
      $displays[$display->handler->definition['admin']] = TRUE;
    }
  }

  if ($displays) {
    ksort($displays);
    $displays = array_keys($displays);
  }
  return $displays;
}

/**
 * Theme function; returns basic administrative information about a view.
 *
 * TODO: template + preprocess
 */
function theme_views_ui_view_info($variables) {
  $title = empty($variables['title']) ? '<em>' . t('No title') . '</em>' : $variables['title'];
  $paths = empty($variables['paths']) ? t('None') : format_plural(count($variables['paths']), 'Path', 'Paths') . ": " . implode(", ", $variables['paths']);
  $displays = empty($variables['displays']) ? t('None') : format_plural(count($variables['displays']), 'Display', 'Displays') . ': ' . '<em>' . implode(', ', $variables['displays']) . '</em>';

  $output = '';
  $output .= '<div class="views-ui-view-title">' . $title . "</div>\n";
  $output .= '<div class="views-ui-view-displays">' . $displays . "</div>\n";
  $output .= '<div class="views-ui-view-paths">' . $paths . "</div>\n";
  return $output;
}
/**
 * Page to delete a view.
 */
function views_ui_delete_confirm($form, &$form_state, $view) {
  $form_state['view'] = &$view;
  $form = array();

  $cancel = 'admin/structure/views';
  if (!empty($_REQUEST['cancel'])) {
    $cancel = $_REQUEST['cancel'];
  }

  if ($view->type == t('Overridden')) {
    $title = t('Are you sure you want to revert the view %name?', array('%name' => $view->name));
    $desc = t('Reverting the view will delete the view that is in the database, reverting it to the original default view. Any changes you have made will be lost and cannot be recovered.');
    $button = t('Revert');
  }
  else {
    $title = t('Are you sure you want to delete the view %name?', array('%name' => $view->name));
    $desc = t('Deleting a view cannot be undone.');
    $button = t('Delete');
  }

  return confirm_form($form,
                  $title,
                  $cancel,
                  $desc,
                  $button,
                  t('Cancel'));
}

/**
 * Submit handler to delete a view.
 */
function views_ui_delete_confirm_submit(&$form, &$form_state) {
  $form_state['view']->delete();
  ctools_object_cache_clear('view', $form_state['view']->name);
  drupal_set_message(t('The view has been deleted.'));
  $form_state['redirect'] = 'admin/structure/views';
}

/**
 * Page to delete a view.
 */
function views_ui_break_lock_confirm($form, &$form_state, $view) {
  $form_state['view'] = &$view;
  $form = array();

  if (empty($view->locked)) {
    return t('There is no lock on view %view to break.', array('%name' => $view->name));
  }

  $cancel = 'admin/structure/views/view/' . $view->name . '/edit';
  if (!empty($_REQUEST['cancel'])) {
    $cancel = $_REQUEST['cancel'];
  }

  $account = user_load($view->locked->uid);
  return confirm_form($form,
                  t('Are you sure you want to break the lock on view %name?',
                  array('%name' => $view->name)),
                  $cancel,
                  t('By breaking this lock, any unsaved changes made by !user will be lost!', array('!user' => theme('username', array('account' => $account)))),
                  t('Break lock'),
                  t('Cancel'));
}

/**
 * Submit handler to break_lock a view.
 */
function views_ui_break_lock_confirm_submit(&$form, &$form_state) {
  ctools_object_cache_clear_all('view', $form_state['view']->name);
  $form_state['redirect'] = 'admin/structure/views/view/' . $form_state['view']->name . '/edit';
  drupal_set_message(t('The lock has been broken and you may now edit this view.'));
}

/**
 * Page title callback for the Edit View page.
 */
function views_ui_edit_page_title($view) {
  return $view->get_title();
}

/**
 * Form builder callback for editing a View.
 *
 * @todo Remove as many #prefix/#suffix lines as possible. Use #theme_wrappers
 *   instead.
 *
 * @todo Rename to views_ui_edit_view_form(). See that function for the "old"
 *   version.
 */
function views_ui_edit_form($form, &$form_state, $view, $display_id = NULL) {
  $form['#tree'] = TRUE;
  // @todo When more functionality is added to this form, cloning here may be
  //   too soon. But some of what we do with $view later in this function
  //   results in making it unserializable due to PDO limitations.
  $form_state['view'] = clone($view);

  $form['#attached']['library'][] = array('system', 'ui.tabs');
  $form['#attached']['library'][] = array('system', 'ui.dialog');
  $form['#attached']['library'][] = array('system', 'drupal.ajax');
  $form['#attached']['library'][] = array('system', 'jquery.form');

  $form['#attached']['css'][] = drupal_get_path('module', 'views_ui') . '/css/views-admin.css';
  $form['#attached']['css'][] = drupal_get_path('module', 'views_ui') . '/css/views-admin.theme.css';
  $form['#attached']['css'][] = array(
    'data' => drupal_get_path('module', 'views_ui') . '/css/ie/views-admin.ie7.css',
    'group' => CSS_DEFAULT,
    'browsers' => array(
      'IE' => 'lte IE 7',
      '!IE' => FALSE
    ),
    'preprocess' => FALSE,
  );

  // Add CSS specific to the Seven theme.
  if ($GLOBALS['theme'] === 'seven') {
    $form['#attached']['css'][drupal_get_path('module', 'views_ui') . '/css/views-admin.seven.css'] = array(
      'group' => CSS_THEME,
      'preprocess' => TRUE,
    );
  }

  $form['#attached']['js'][] = drupal_get_path('module', 'views_ui') . '/js/views-admin.js';
  $form['#attached']['js'][] = array(
    'data' => array('views' => array('ajax' => array(
      'id' => '#views-ajax-pad',
      'title' => '#views-ajax-title',
      'defaultForm' => views_ui_get_default_ajax_message(),
    ))),
    'type' => 'setting',
  );

  $form += array(
    '#prefix' => '',
    '#suffix' => '',
  );
  $form['#prefix'] = $form['#prefix'] . '<div class="views-edit-view">';
  $form['#suffix'] = '</div>' . $form['#suffix'];

  if (isset($view->locked) && is_object($view->locked)) {
    $form['locked'] = array(
      '#theme' => 'views_ui_break_lock',
      '#user' => theme('username', array('account' => user_load($view->locked->uid))),
      '#lock_age' => format_interval(REQUEST_TIME - $view->locked->updated),
      '#break_link' => url('admin/structure/views/view/' . $view->name . '/break-lock'),
    );
  }

  $form['actions'] = array('#type' => 'actions', '#weight' => -10);
  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    // Taken from the "old" UI.
    '#validate' => array('views_ui_edit_view_form_validate'),
    '#submit' => array('views_ui_edit_view_form_submit'),
  );
  $form['displays'] = array(
    '#prefix' => '<h3 class="clearfix">' . t('Queries/displays') . '</h3>' . "\n" . '<div class="views-displays">',
    '#suffix' => '</div>',
  );

  // Determine the displays available for editing.
  if ($tabs = views_ui_edit_page_display_tabs($view)) {
    // If a display isn't specified, use the first one.
    if (empty($display_id)) {
      foreach ($tabs as $id => $tab) {
        if (!isset($tab['#access']) || $tab['#access']) {
          $display_id = $id;
          break;
        }
      }
    }
    // If a display is specified, but we don't have access to it, return
    // an access denied page.
    if ($display_id && (!isset($tabs[$display_id]) || (isset($tabs[$display_id]['#access']) && !$tabs[$display_id]['#access']))) {
      // TODO: This doesn't work inside a form.
      return MENU_ACCESS_DENIED;
    }

    if ($display_id) {
      $tabs[$display_id]['#active'] = TRUE;
    }
    $tabs['#prefix'] = '<h2 class="element-invisible">' . t('Secondary tabs') . '</h2><ul class="tabs secondary">';
    $tabs['#suffix'] = '</ul>';
    $form['displays']['tabs'] = $tabs;
  }
  elseif ($display_id) {
    return MENU_ACCESS_DENIED;
  }
  else {
    $display_id = NULL;
  }

  // Buttons for adding a new display.
  foreach (views_fetch_plugin_names('display') as $type => $label) {
    $form['displays']['add_display'][$type] = array(
      '#type' => 'submit',
      '#value' => t('Add !display', array('!display' => $label)),
      '#limit_validation_errors' => array(),
      '#submit' => array('views_ui_edit_form_submit_add_display'),
    );
  }

  // The rest requires a display to be selected.
  if ($display_id) {
    $form_state['display_id'] = $display_id;

    // The part of the page where editing will take place.
    $view->set_display($display_id);
    $form['displays']['tab_content'] = views_ui_get_display_tab($view, $display_id);
    $form['displays']['tab_content']['#theme_wrappers'] = array('container');
    $form['displays']['tab_content']['#attributes'] = array('class' => array('views-display-tab'));
    $form['displays']['tab_content']['#id'] = 'views-tab-' . $display_id;

    // The live preview.
    $view->views_ui_context = TRUE;
    $form['displays']['preview'] = array(
      '#markup' => $view->preview($display_id, array()),
    );

    // The content of the popup dialog.
    $form['ajax-area'] = array(
      '#theme' => 'container',
      '#id' => 'views-ajax-pad',
      'message' => array('#markup' => views_ui_get_default_ajax_message()),
    );
  }

  return $form;
}

function views_ui_get_default_ajax_message() {
  return '<div class="message">' . t("Click on an item to edit that item's details.") . '</div>';
}

/**
 * Submit handler to add a display to a view.
 */
function views_ui_edit_form_submit_add_display($form, &$form_state) {
  $view = $form_state['view'];

  // Create the new display.
  $parents = $form_state['triggering_element']['#parents'];
  $display_type = array_pop($parents);
  $display_id = $view->add_display($display_type);
  views_ui_cache_set($view);

  // Redirect to the new display's edit page.
  $form_state['redirect'] = 'admin/structure/views/view/' . $view->name . '/edit/' . $display_id;
}

/**
 * Submit handler to duplicate a display for a view.
 */
function views_ui_edit_form_submit_duplicate_display($form, &$form_state) {
  $view = $form_state['view'];
  $display_id = $form_state['display_id'];

  // Create the new display.
  $display = $view->display[$display_id];
  $new_display_id = $view->add_display($display->display_plugin);
  $view->display[$new_display_id] = clone $display;
  $view->display[$new_display_id]->id = $new_display_id;
  views_ui_cache_set($view);

  // Redirect to the new display's edit page.
  $form_state['redirect'] = 'admin/structure/views/view/' . $view->name . '/edit/' . $new_display_id;
}

/**
 * Submit handler to delete a display from a view.
 *
 * @todo Do we want a confirmation step?
 */
function views_ui_edit_form_submit_delete_display($form, &$form_state) {
  $view = $form_state['view'];
  $display_id = $form_state['display_id'];

  // Mark the display for deletion.
  $view->display[$display_id]->deleted = TRUE;
  views_ui_cache_set($view);

  // Redirect to the top-level edit page. The first remaining display will
  // become the active display.
  $form_state['redirect'] = 'admin/structure/views/view/' . $view->name;
}

/**
 * Adds tabs for navigating across Displays when editing a View.
 *
 * This function can be called from hook_menu_local_tasks_alter() to implement
 * these tabs as secondary local tasks, or it can be called from elsewhere if
 * having them as secondary local tasks isn't desired. The caller is responsible
 * for setting the active tab's #active property to TRUE.
 */
function views_ui_edit_page_display_tabs($view) {
  $tabs = array();

  // Create a tab for each display.
  foreach ($view->display as $id => $display) {
    if (!empty($display->deleted)) {
      continue;
    }
    $tabs[$id] = array(
      '#theme' => 'menu_local_task',
      '#link' => array(
        'title' => views_ui_get_display_label($view, $id),
        'href' => 'admin/structure/views/view/' . $view->name . '/edit/' . $id,
        'localized_options' => array(),
      ),
    );
  }

  // If the default display isn't supposed to be shown, don't display its tab.
  if (!views_ui_show_default_display($view)) {
    $tabs['default']['#access'] = FALSE;
  }

  return $tabs;
}

/**
 * Controls whether or not the default display should have its own tab on edit.
 */
function views_ui_show_default_display($view) {
  // @todo For now, show the default display only if there are no others, and
  //   hide it if there's at least one "real" one. Later, we may add a setting.
  return (count($view->display) == 1);
}

/**
 * Returns a renderable array representing the edit page for one display.
 */
function views_ui_get_display_tab($view, $display_id) {
  $build = array();
  $display = $view->display[$display_id];
  $plugin = $display->handler->definition;
  // If the plugin doesn't exist, display an error message instead of an edit
  // page.
  if (empty($plugin)) {
    $title = isset($display->display_title) ? $display->display_title : t('Invalid');
    // @TODO: Improved UX for the case where a plugin is missing.
    $build['#markup'] = t("Error: Display @display refers to a plugin named '@plugin', but that plugin is missing.", array('@display' => $display->id, '@plugin' => $display->display_plugin));
  }
  // Build the content of the edit page.
  else {
    $build['details'] = views_ui_get_display_tab_details($view, $display);
  }
  return $build;
}

/**
 * Helper function to get the body of the page query edit UI.
 *
 * @param $view
 * @param $display
 *
 * @return array
 *   A renderable page build array.
 */
function views_ui_get_display_tab_details($view, $display) {
  $type = $display->display_title;
  $build = array(
    '#type' => 'fieldset',
    '#title' => t('@display_type details', array('@display_type' => ucwords($type))),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE, // @TODO: Respect user preference for this attachment.
  );

  // The Delete and Duplicate buttons.
  $build['actions'] = array(
    '#prefix' => '<div class="actions">',
    '#suffix' => '</div>',
  );
  $plugin = views_fetch_plugin_data('display', $view->display[$display->id]->display_plugin);
  if (empty($plugin['no remove'])) {
    $build['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#limit_validation_errors' => array(),
      '#submit' => array('views_ui_edit_form_submit_delete_display'),
    );
  }
  $build['actions']['duplicate'] = array(
    '#type' => 'submit',
    '#value' => t('Duplicate'),
    '#limit_validation_errors' => array(),
    '#submit' => array('views_ui_edit_form_submit_duplicate_display'),
  );

  // The area above the three columns.
  $build['top']['view_name'] = array(
    '#theme' => 'views_ui_display_tab_setting',
    '#description' => t('Display name'),
    // @TODO: Are we letting them edit the display id, or some other property?
    '#link' => check_plain($display->id),
  );

  $build['columns'] = array();
  $build['columns']['#theme_wrappers'] = array('container');
  $build['columns']['#attributes'] = array('class' => array('clearfix', 'views-display-columns'));

  $build['columns']['first']['#theme_wrappers'] = array('container');
  $build['columns']['first']['#attributes'] = array('class' => array('views-display-column', 'first'));

  $build['columns']['second']['#theme_wrappers'] = array('container');
  $build['columns']['second']['#attributes'] = array('class' => array('views-display-column', 'second'));

  $build['columns']['second']['settings'] = array();
  $build['columns']['second']['header'] = array();
  $build['columns']['second']['footer'] = array();
  $build['columns']['second']['pager'] = array();

  $build['columns']['third']['#theme_wrappers'] = array('container');
  $build['columns']['third']['#attributes'] = array('class' => array('views-display-column', 'third'));

  // Each option (e.g. title, access, display as grid/table/list) fits into one
  // of several "buckets," or boxes (Format, Fields, Sort, and so on).
  $buckets = array();

  // Fetch options from the display plugin, with a list of buckets they go into.
  $options = array();
  $display->handler->options_summary($buckets, $options);

  // Place each option into its bucket.
  foreach ($options as $id => $option) {
    // Each option self-identifies as belonging in a particular bucket.
    $buckets[$option['category']]['build'][$id] = views_ui_edit_form_get_build_from_option($id, $option, $view, $display);
  }

  // Place each bucket into the proper column.
  foreach ($buckets as $id => $bucket) {
    // Let buckets identify themselves as belonging in a column.
    if (isset($bucket['column']) && isset($build['columns'][$bucket['column']])) {
      $column = $bucket['column'];
    }
    // If a bucket doesn't pick one of our predefined columns to belong to, put
    // it in the last one.
    else {
      $column = 'third';
    }
    if (isset($bucket['build']) && is_array($bucket['build'])) {
      $build['columns'][$column][$id] = $bucket['build'];
      $build['columns'][$column][$id]['#theme_wrappers'][] = 'views_ui_display_tab_bucket';
      $build['columns'][$column][$id]['#title'] = !empty($bucket['title']) ? $bucket['title'] : '';
    }
  }

  // Fetch the style plugin info so we know whether to list fields or not.
  $style_plugin = $display->handler->get_plugin();
  $no_fields = !($style_plugin && $style_plugin->uses_fields());

  $build['columns']['first']['fields'] = views_ui_edit_form_get_bucket('field', $view, $display, $no_fields);
  $build['columns']['second']['header'] = views_ui_edit_form_get_bucket('header', $view, $display);
  $build['columns']['second']['footer'] = views_ui_edit_form_get_bucket('footer', $view, $display);
  $build['columns']['second']['empty'] = views_ui_edit_form_get_bucket('empty', $view, $display);
  $build['columns']['third']['relationships'] = views_ui_edit_form_get_bucket('relationship', $view, $display);
  $build['columns']['third']['arguments'] = views_ui_edit_form_get_bucket('argument', $view, $display);
  $build['columns']['first']['filters'] = views_ui_edit_form_get_bucket('filter', $view, $display);
  $build['columns']['first']['sorts'] = views_ui_edit_form_get_bucket('sort', $view, $display);


  // Add a checkbox controlling whether or not this display auto-previews.
  // TODO: Wire it up to actual functionality.
  $build['auto_preview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto preview'),
  );

  return $build;
}

/**
 * Build a renderable array representing one option on the edit form.
 *
 * This function might be more logical as a method on an object, if a suitable
 * object emerges out of refactoring.
 */
function views_ui_edit_form_get_build_from_option($id, $option, $view, $display) {
  $option_build = array();
  $option_build['#theme'] = 'views_ui_display_tab_setting';

  $option_build['#description'] = $option['title'];

  $option_build['#link'] = $display->handler->option_link($option['value'], $id, '', empty($option['desc']) ? '' : $option['desc']);

  $option_build['#links'] = array();
  if (!empty($option['links']) && is_array($option['links'])) {
    foreach ($option['links'] as $link_id => $link_value) {
      $option_build['#optional_links'][] = $display->handler->option_link($link_value, $link_id, 'views-button-configure');
    }
  }

  if (!empty($display->handler->options['defaults'][$id])) {
    $display_id = 'default';
    $option_build['#defaulted'] = TRUE;
  }
  else {
    $display_id = $display->id;
    if (!$display->handler->is_default_display()) {
      if ($display->handler->defaultable_sections($id)) {
        $option_build['#overridden'] = TRUE;
      }
    }
  }
  $option_build['#attributes']['class'][] = drupal_clean_css_identifier($display_id . '-' . $id);
  if (!empty($view->changed_sections[$display_id . '-' . $id])) {
    $option_build['#changed'] = TRUE;
  }
  return $option_build;
}

function template_preprocess_views_ui_display_tab_setting(&$variables) {
  static $zebra = 0;
  $variables['zebra'] = ($zebra % 2 === 0 ? 'odd' : 'even');
  $zebra++;

  $variables['gear'] = implode($variables['settings_links']);
}

function template_preprocess_views_ui_display_tab_bucket(&$variables) {
  $element = $variables['element'];

  $variables['content'] = $element['#children'];
  $variables['title'] = $element['#title'];
  $variables['overridden'] = !empty($element['#overridden']);
  $variables['rearrange'] = !empty($element['#rearrange']) ? $element['#rearrange'] : '';
  $variables['add'] = !empty($element['#add']) ? $element['#add'] : '';
}

function template_preprocess_views_ui_display_tab_column(&$variables) {
  $element = $variables['element'];

  $variables['content'] = $element['#children'];
  $variables['column'] = $element['#column'];
}

/**
 * Export a view for cut & paste.
 */
function views_ui_export_page($form, &$form_state, $view) {
  $code = $view->export();
  $lines = substr_count($code, "\n");
  $form['code'] = array(
    '#type' => 'textarea',
    '#title' => $view->name,
    '#default_value' => $code,
    '#rows' => $lines,
  );
  return $form;
}

/**
 * Import a view from cut & paste
 */
function views_ui_import_page($form, &$form_state) {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('View name'),
    '#description' => t('Enter the name to use for this view if it is different from the source view. Leave blank to use the name of the view.'),
  );

  $form['view'] = array(
    '#type' => 'textarea',
    '#title' => t('Paste view code here'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array('views_ui_import_submit'),
    '#validate' => array('views_ui_import_validate'),
  );
  return $form;
}

/**
 * Validate handler to import a view
 */
function views_ui_import_validate($form, &$form_state) {
  $view = '';
  views_include('view');
  ob_start();
  eval($form_state['values']['view']);
  ob_end_clean();

  if (!is_object($view)) {
    return form_error($form['view'], t('Unable to interpret view code.'));
  }

  if (empty($view->api_version) || $view->api_version < 2) {
    // Check for some value that would only exist on a Views 1 view.
    if (isset($view->url) || isset($view->page) || isset($view->block)) {
      views_include('convert');
      $view = views1_import($view);
      drupal_set_message(t('You are importing a view created in Views version 1. You may need to adjust some parameters to work correctly in version 2.'), 'warning');
    }
    else {
      form_error($form['view'], t('That view is not compatible with this version of Views.'));
    }
  }
  elseif ($view->api_version > views_api_version()) {
    form_error($form['view'], t('That view is created for the version @import_version of views, but you only have @api_version', array(
      '@import_version' => $view->api_version,
      '@api_version' => views_api_version())));
  }

  // View name must be alphanumeric or underscores, no other punctuation.
  if (!empty($form_state['values']['name']) && preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
    form_error($form['name'], t('View name must be alphanumeric or underscores only.'));
  }

  if ($form_state['values']['name']) {
    $view->name = $form_state['values']['name'];
  }

  $test = views_get_view($view->name);
  if ($test && $test->type != t('Default')) {
    form_set_error('', t('A view by that name already exists; please choose a different name'));
  }

  $view->init_display();

  $broken = FALSE;
  // Make sure that all plugins and handlers needed by this view actually exist.
  foreach ($view->display as $id => $display) {
    if (empty($display->handler) || !empty($display->handler->broken)) {
      drupal_set_message(t('Display plugin @plugin is not available.', array('@plugin' => $display->display_plugin)), 'error');
      $broken = TRUE;
      continue;
    }

    $plugin = views_get_plugin('style', $display->handler->get_option('style_plugin'));
    if (!$plugin) {
      drupal_set_message(t('Style plugin @plugin is not available.', array('@plugin' => $display->handler->get_option('style_plugin'))), 'error');
      $broken = TRUE;
    }
    elseif ($plugin->uses_row_plugin()) {
      $plugin = views_get_plugin('row', $display->handler->get_option('row_plugin'));
      if (!$plugin) {
        drupal_set_message(t('Row plugin @plugin is not available.', array('@plugin' => $display->handler->get_option('row_plugin'))), 'error');
        $broken = TRUE;
      }
    }

    foreach (views_object_types() as $type => $info) {
      $handlers = $display->handler->get_handlers($type);
      if ($handlers) {
        foreach ($handlers as $id => $handler) {
          if ($handler->broken()) {
            drupal_set_message(t('@type handler @table.@field is not available.', array(
              '@type' => $info['stitle'],
              '@table' => $handler->table,
              '@field' => $handler->field,
            )), 'error');
            $broken = TRUE;
          }
        }
      }
    }
  }

  if ($broken) {
    form_set_error('', t('Unable to import view.'));
  }

  $form_state['view'] = &$view;
}

/**
 * Submit handler for view import
 */
function views_ui_import_submit($form, &$form_state) {
  // Store in cache and then go to edit.
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/structure/views/view/' . $form_state['view']->name . '/edit';
}

/**
 * The main edit view form, which is really just a save/cancel/delete button.
 */
function views_ui_edit_view_form($form, &$form_state, $view) {
  $form['buttons']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#validate' => array('views_ui_edit_view_form_validate'),
    '#submit' => array('views_ui_edit_view_form_submit'),
  );

  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('views_ui_edit_view_form_cancel'),
  );

  if (is_numeric($view->vid)) {
    $form['buttons']['delete'] = array(
      '#type' => 'submit',
      '#value' => $view->type == t('Overridden') ? t('Revert') : t('Delete'),
      '#submit' => array('views_ui_edit_view_form_delete'),
    );
  }

  $form_state['view'] = &$view;
  return $form;
}

/**
 * Validate that a view is complete and whole.
 */
function views_ui_edit_view_form_validate($form, &$form_state) {
  // Do not validate cancel or delete or revert.
  if (empty($form_state['clicked_button']['#value']) || $form_state['clicked_button']['#value'] != t('Save')) {
    return;
  }

  $errors = $form_state['view']->validate();
  if ($errors !== TRUE) {
    foreach ($errors as $error) {
      form_set_error('', $error);
    }
  }
}

/**
 * Submit handler for the edit view form.
 */
function views_ui_edit_view_form_submit($form, &$form_state) {
  // Go through and remove displayed scheduled for removal.
  foreach ($form_state['view']->display as $id => $display) {
    if (!empty($display->deleted)) {
      unset($form_state['view']->display[$id]);
    }
  }
  // Rename display ids if needed.
  foreach ($form_state['view']->display as $id => $display) {
    if (!empty($display->new_id)) {
      $form_state['view']->display[$id]->id = $display->new_id;
    }
  }

  $form_state['view']->save();
  drupal_set_message(t('The view %name has been saved.', array('%name' => $form_state['view']->name)));

  // Make sure menu items get rebuilt as neces
  menu_rebuild();

  // Clear the views cache.
  cache_clear_all('*', 'cache_views');

  // Clear the page cache.
  cache_clear_all();

  // Remove this view from cache so we can edit it properly.
  ctools_object_cache_clear('view', $form_state['view']->name);
}

/**
 * Submit handler for the edit view form.
 */
function views_ui_edit_view_form_cancel($form, &$form_state) {
  // Remove this view from cache so edits will be lost.
  ctools_object_cache_clear('view', $form_state['view']->name);
  if (empty($form['view']->vid)) {
    // I seem to have to drupal_goto here because I can't get fapi to
    // honor the redirect target. Not sure what I screwed up here.
    drupal_goto('admin/structure/views');
  }
}

function views_ui_edit_view_form_delete($form, &$form_state) {
  unset($_REQUEST['destination']);
  // Redirect to the delete confirm page
  $form_state['redirect'] = array('admin/structure/views/view/' . $form_state['view']->name . '/delete', array('query' => drupal_get_destination() + array('cancel' => 'admin/structure/views/view/' . $form_state['view']->name . '/edit')));
}

/**
 * Generate the summary output for a single display to render in a tab.
 */
function views_ui_display_tab($view, $display) {
  if (isset($display->handler)) {
    $plugin = $display->handler->definition;
  }
  if (empty($plugin)) {
    $title = isset($display->display_title) ? $display->display_title : t('Invalid');
    return array($title, t("Error: Display @display refers to a plugin named '@plugin', but that plugin doesn't exist!", array('@display' => $display->id, '@plugin' => $display->display_plugin)));

    // @todo We can do a better 'plugin does not exist' tab.
  }

  // The display should always be initialized prior to this call.
  if (empty($display->handler)) {
    return FALSE;
  }

  $body = theme('views_ui_edit_tab', array('view' => $view, 'display' => $display));
  return array($display->display_title, $body);
}

/**
 * Add information about a section to a display.
 */
function views_ui_edit_form_get_bucket($type, $view, $display, $no_fields = FALSE) {
  $build = array(
    '#theme_wrappers' => array('views_ui_display_tab_bucket'),
  );
  $types = views_object_types();

  $build['#overridden'] = FALSE;
  $build['#defaulted'] = FALSE;
  if (module_exists('advanced_help')) {
    $build['item_help_icon'] = array(
      '#theme' => 'advanced_help_topic',
      '#module' => 'views',
      '#topic' => $type,
    );
  }
  
  if ($no_fields) {
    $build['#title'] = $types[$type]['title'];
    $build['#rearrange'] = '';
    $build['#add'] = '';
    return $build;
  }

  $build['#rearrange'] = l('<span>' . t('Rearrange') . '</span>', "admin/structure/views/nojs/rearrange/$view->name/$display->id/$type", array('attributes' => array('class' => array('views-button-rearrange', 'views-ajax-link'), 'title' => t('Rearrange'), 'id' => 'views-rearrange-' . $type), 'html' => true));
  $build['#add'] = l('<span>' . t('Add') . '</span>', "admin/structure/views/nojs/add-item/$view->name/$display->id/$type", array('attributes' => array('class' => array('views-button-add', 'views-ajax-link'), 'title' => t('Add'), 'id' => 'views-add-' . $type), 'html' => true));

  if (!$display->handler->is_default_display()) {
    if (!$display->handler->is_defaulted($types[$type]['plural'])) {
      $build['#overridden'] = TRUE;
    }
    else {
      $build['#defaulted'] = TRUE;
    }
  }

  if ($display->display_plugin != 'default') {
    $build['#title'] = l($types[$type]['title'], "admin/structure/views/nojs/config-type/$view->name/$display->id/$type", array('attributes' => array('class' => array('views-ajax-link'), 'id' => 'views-title-' . $type)));
  }
  else {
    $build['#title'] = $types[$type]['title'];
  }

  static $relationships = NULL;
  if (!isset($relationships)) {
    // Get relationship labels
    $relationships = array();
    // @todo: get_handlers()
    foreach ($display->handler->get_handlers('relationship') as $id => $handler) {
      $relationships[$id] = $handler->label();
    }
  }

  foreach ($display->handler->get_option($types[$type]['plural']) as $id => $field) {
    // Build the option link for this handler ("Node: ID = article").
    $build[$id] = array();
    $build[$id]['#theme'] = 'views_ui_display_tab_setting';

    $handler = $display->handler->get_handler($type, $id);
    if (empty($handler)) {
      $build[$id]['#attributes']['class'][] = 'broken';
      $field_name = t('Broken/missing handler: @table > @field', array('@table' => $field['table'], '@field' => $field['field']));
      $build[$id]['#link'] = l($field_name, "admin/structure/views/nojs/config-item/$view->name/$display->id/$type/$id", array('attributes' => array('class' => array('views-ajax-link')), 'html' => TRUE));
      continue;
    }

    $field_name = $handler->ui_name(TRUE);
    if (!empty($field['relationship']) && !empty($relationships[$field['relationship']])) {
      $field_name = '(' . $relationships[$field['relationship']] . ') ' . $field_name;
    }

    $description = $handler->admin_summary();
    $link_text = $field_name . (empty($description) ? '' : " = $description");
    $build[$id]['#link'] = l($link_text, "admin/structure/views/nojs/config-item/$view->name/$display->id/$type/$id", array('attributes' => array('class' => array('views-ajax-link')), 'html' => TRUE));
    $build[$id]['#attributes']['class'][] = drupal_clean_css_identifier($display->id . '-' . $type . '-' . $id);
    if (!empty($view->changed_sections[$display->id . '-' . $type . '-' . $id])) {
      $build[$id]['#changed'] = TRUE;
    }

    if ($display->handler->use_group_by()) {
      $build[$id]['#links'][] = l('<span>' . t('Group settings') . '</span>', "admin/structure/views/nojs/config-item-group/$view->name/$display->id/$type/$id", array('attributes' => array('class' => 'views-button-configure views-ajax-link', 'title' => t('Group settings')), 'html' => true));
    }

    if ($handler->has_extra_options()) {
      $build[$id]['#links'][] = l('<span>' . t('Settings') . '</span>', "admin/structure/views/nojs/config-item-extra/$view->name/$display->id/$type/$id", array('attributes' => array('class' => array('views-button-configure', 'views-ajax-link'), 'title' => t('Settings')), 'html' => true));
    }

    if ($handler->needs_style_plugin()) {
      // Add a second option link for the style plugin.
      $style_plugin = views_fetch_plugin_data('style', $handler->options['style_plugin']);
      $style_title = empty($style_plugin['title']) ? t('Missing style plugin') : $style_plugin['title'];
      $pid = $id . '-style-plugin';

      $build[$pid] = array();
      $build[$pid]['#theme'] = 'views_ui_display_tab_setting';

      if (!empty($style_plugin['uses options'])) {
        $build[$pid]['#links'][] = l('<span>' . t('Change settings for this style') . '</span>', "admin/structure/views/nojs/config-style/$view->name/$display->id/$type/$id", array('attributes' => array('class' => array('views-button-configure', 'views-ajax-link'), 'title' => t('Settings')), 'html' => true));
      }

      $build[$pid]['#link'] = ' ' . t('&nbsp; Style: !style', array('!style' => l($style_title, "admin/structure/views/nojs/change-style/$view->name/$display->id/$type/$id", array('attributes' => array('class' => array('views-ajax-link'))))));
      $build[$pid]['#attributes']['class'][] = drupal_clean_css_identifier($display->id . '-' . $type . '-' . $pid);
      if (!empty($view->changed_sections[$display->id . '-' . $type . '-' . $pid])) {
        $build[$pid]['#changed'] = TRUE;
      }
    }
  }

  return $build;
}

/**
 * Regenerate the tabs for AJAX updates.
 */
function views_ui_regenerate_tabs(&$view, &$output, $display_id = NULL) {
  if (empty($display_id)) {
    $displays = array_keys($view->display);
  }
  elseif (!is_array($display_id)) {
    $displays = array($display_id);
    if ($display_id != 'default') {
      $displays[] = 'default';
    }
  }
  else {
    $displays = $display_id;
  }

  if (!$view->set_display('default')) {
    return;
  }

  foreach ($displays as $id) {
    list($title, $body) = views_ui_display_tab($view, $view->display[$id]);
    $output[] = ajax_command_html('#views-tab-' . $id, $body);
    $output[] = ajax_command_html('#views-tab-title-' . $id, check_plain($title));
  }
}

/**
 * Provide standard buttons for the forms to make it easy. Also provide
 * a hidden op operator because the forms plugin doesn't seem to properly
 * provide which button was clicked.
 */
function views_ui_standard_form_buttons(&$form, &$form_state, $form_id, $name = NULL, $third = NULL, $submit = NULL) {
  $form['buttons'] = array(
    '#prefix' => '<div class="clearfix"><div class="form-buttons">',
    '#suffix' => '</div></div>',
  );

  if (empty($name)) {
    $name = t('Update');
  }

  // remove default validate handler
  $form['#validate'] = array();

    // Add the override and update button
  if ($name == t('Update default display')) {
    $form['buttons']['override_update'] = array(
      '#type' => 'submit',
      '#value' => t('Update and override'),
      '#submit' => array(
        'views_ui_edit_display_form_override_update_section',
        'views_ui_standard_submit',
        'views_ui_edit_display_form_override_update',
      ),
    );
  }

  if (empty($form_state['ok_button'])) {
    // but be sure submit button validates!
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => $name,
      '#submit' => array('views_ui_standard_submit', $form_id . '_submit'),
    );
    // Take sure that the validation handler exists.
    if (function_exists($form_id . '_validate')) {
      $form['buttons']['submit']['#validate'][] = $form_id . '_validate';
    }

  }

  $cancel_submit = function_exists($form_id . '_cancel') ? $form_id . '_cancel' : 'views_ui_standard_cancel';
  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => empty($form_state['ok_button']) ? t('Cancel') : t('Ok'),
    '#submit' => array($cancel_submit),
    '#validate' => array(),
  );

  if ($third) {
    if (empty($submit)) {
      $submit = 'third';
    }
    $third_submit = function_exists($form_id . '_' . $submit) ? $form_id . '_' . $submit : 'views_ui_standard_cancel';

    $form['buttons'][$submit] = array(
      '#type' => 'submit',
      '#value' => $third,
      '#validate' => array(),
      '#submit' => array($third_submit),
    );
  }

  // Compatibility, to be removed later:
  // We used to set these items on the form, but now we want them on the $form_state:
  if (isset($form['#title'])) {
    $form_state['title'] = $form['#title'];
  }
  if (isset($form['#help_topic'])) {
    $form_state['help_topic'] = $form['#help_topic'];
  }
  if (isset($form['#help_module'])) {
    $form_state['help_module'] = $form['#help_module'];
  }
  if (isset($form['#url'])) {
    $form_state['url'] = $form['#url'];
  }
  if (isset($form['#js'])) {
    if (!empty($form_state['js settings']) && is_array($form_state['js settings'])) {
      $form_state['js settings'] = array_merge($form_state['js settings'], $form['#js']);
    }
    else {
      $form_state['js settings'] = $form['#js'];
    }
  }
  if (isset($form['#section'])) {
    $form_state['#section'] = $form['#section'];
  }
  // Finally, we never want these cached -- our object cache does that for us.
  $form['#no_cache'] = TRUE;

  // If this isn't an ajaxy form, then we want to set the title.
  if (!empty($form['#title'])) {
    drupal_set_title($form['#title']);
  }
  // @todo Find a way to load the views-admin.seven.css only when Seven is the
  // admin theme
  $form['#attached']['css'][] = array(
    drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.css'),
    drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/ie/views-admin.ie7.css', 
      array(
        'group' => CSS_DEFAULT,
        'browsers' => array(
          'IE' => 'lte IE 7',
          '!IE' => FALSE
        ),
        'preprocess' => FALSE,
      )
    ),
    drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.theme.css', 
      array(
        'group' => CSS_DEFAULT,
        'preprocess' => TRUE,
      )
    ),
    drupal_add_css(drupal_get_path('module', 'views_ui') . '/css/views-admin.seven.css', 
      array(
        'group' => CSS_THEME,
        'preprocess' => TRUE,
      )
    ),
  );
}

/**
 * Basic submit handler applicable to all 'standard' forms
 */
function views_ui_standard_submit($form, &$form_state) {
  if (!empty($form['#section'])) {
    $form_state['view']->changed_sections[$form['#section']] = TRUE;
  }
}

/**
 * Submit handler for cancel button
 */
function views_ui_standard_cancel($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/views/view/' . $form_state['view']->name . '/edit';
}

// --------------------------------------------------------------------------
// Various subforms for editing the pieces of a view.

function views_ui_ajax_forms($key = NULL) {
  $forms = array(
    'display' => array(
      'form_id' => 'views_ui_edit_display_form',
      'args' => array('section'),
    ),
    'remove-display' => array(
      'form_id' => 'views_ui_remove_display_form',
      'args' => array(),
    ),
    'config-type' => array(
      'form_id' => 'views_ui_config_type_form',
      'args' => array('type'),
    ),
    'rearrange' => array(
      'form_id' => 'views_ui_rearrange_form',
      'args' => array('type'),
    ),
    'add-item' => array(
      'form_id' => 'views_ui_add_item_form',
      'args' => array('type'),
    ),
    'config-item' => array(
      'form_id' => 'views_ui_config_item_form',
      'args' => array('type', 'id'),
    ),
    'config-item-extra' => array(
      'form_id' => 'views_ui_config_item_extra_form',
      'args' => array('type', 'id'),
    ),
    'config-item-group' => array(
      'form_id' => 'views_ui_config_item_group_form',
      'args' => array('type', 'id'),
    ),
    'change-style' => array(
      'form_id' => 'views_ui_change_style_form',
      'args' => array('type', 'id'),
    ),
    'config-style' => array(
      'form_id' => 'views_ui_config_style_form',
      'args' => array('type', 'id'),
    ),
  );

  if ($key) {
    return !empty($forms[$key]) ? $forms[$key] : NULL;
  }

  return $forms;
}

/**
 * Build a form identifier that we can use to see if one form
 * is the same as another. Since the arguments differ slightly
 * we do a lot of spiffy concenation here.
 */
function views_ui_build_identifier($key, $view, $display_id, $args) {
  $form = views_ui_ajax_forms($key);
  $identifier = implode('-', array($key, $view->name, $display_id));

  foreach ($form['args'] as $id) {
    $arg = (!empty($args)) ? array_shift($args) : NULL;
    $identifier .= '-' . $arg;
  }
  return $identifier;
}

/**
 * Build up a $form_state object suitable for use with drupal_build_form
 * based on known information about a form.
 */
function views_ui_build_form_state($js, $key, &$view, $display_id, $args) {
  $form = views_ui_ajax_forms($key);
  // Build up form state
  $form_state = array(
    'form_key' => $key,
    'form_id' => $form['form_id'],
    'view' => &$view,
    'ajax' => $js,
    'display_id' => $display_id,
    'no_redirect' => TRUE,
  );

  foreach ($form['args'] as $id) {
    $form_state[$id] = (!empty($args)) ? array_shift($args) : NULL;
  }

  return $form_state;
}

/**
 * Create the URL for one of our standard AJAX forms based upon known
 * information about the form.
 */
function views_ui_build_form_url($form_state) {
  $form = views_ui_ajax_forms($form_state['form_key']);
  $ajax = empty($form_state['ajax']) ? 'nojs' : 'ajax';
  $name = $form_state['view']->name;
  $url = "admin/structure/views/$ajax/$form_state[form_key]/$name/$form_state[display_id]";
  foreach ($form['args'] as $arg) {
    $url .= '/' . $form_state[$arg];
  }
  return $url;
}

/**
 * Add another form to the stack; clicking 'update' will go to this form
 * rather than closing the ajax pad.
 */
function views_ui_add_form_to_stack($key, &$view, $display_id, $args, $top = FALSE) {
  if (empty($view->stack)) {
    $view->stack = array();
  }

  $stack = array(views_ui_build_identifier($key, $view, $display_id, $args), $key, &$view, $display_id, $args);
  if ($top) {
    array_unshift($view->stack, $stack);
  }
  else {
    $view->stack[] = $stack;
  }
}

/**
 * Generic entry point to handle forms.
 *
 * We do this for consistency and to make it easy to chain forms
 * together. This only works for forms that use both $view
 * and $display_id, so we have a couple of ajax forms that we don't
 * use with this system.
 */
function views_ui_ajax_form($js, $key, $view, $display_id) {
  $form = views_ui_ajax_forms($key);
  if (empty($form)) {
    return drupal_not_found();
  }

  views_include('ajax');
  $args = func_get_args();
  // Remove the known args
  array_splice($args, 0, 4);

  $form_state = views_ui_build_form_state($js, $key, $view, $display_id, $args);
  // check to see if this is the top form of the stack. If it is, pop
  // it off; if it isn't, the user clicked somewhere else and the stack is
  // now irrelevant.
  if (!empty($view->stack)) {
    $identifier = views_ui_build_identifier($key, $view, $display_id, $args);
    $top = array_shift($view->stack);
    if (array_shift($top) != $identifier) {
      $view->stack = array();
    }
  }

  $output = views_ajax_form_wrapper($form_state['form_id'], $form_state);
  if ($form_state['submitted'] && empty($form_state['rerender'])) {
    // Sometimes we need to re-generate the form for multi-step type operations.
    $object = NULL;
    if (!empty($view->stack)) {
      $stack = $view->stack;
      $top = array_shift($stack);
      $top[0] = $js;
      $form_state = call_user_func_array('views_ui_build_form_state', $top);
      $form_state['input'] = array();
      $form_state['url'] = url(views_ui_build_form_url($form_state));
      if (!$js) {
        return drupal_goto(views_ui_build_form_url($form_state));
      }
      $output = views_ajax_form_wrapper($form_state['form_id'], $form_state);
    }
    elseif (!$js) {
      // if nothing on the stack, non-js forms just go back to the main view editor.
      return drupal_goto("admin/structure/views/view/$view->name/edit");
    }
    else {
      $output = array();
      $output[] = views_ajax_command_enable_buttons();
      $output[] = views_ajax_command_dismiss_form();
      $output[] = views_ajax_command_trigger_preview();
    }
    views_ui_regenerate_tabs($view, $output);
  }
  elseif ($js) {
    $output[] = views_ajax_command_disable_buttons();
  }

  return $js ? array('#type' => 'ajax', '#commands' => $output) : $output;
}

/**
 * Submit handler to add a restore a removed display to a view.
 */
function views_ui_remove_display_form_restore($form, &$form_state) {
  // Create the new display
  $id = $form_state['display_id'];
  $form_state['view']->display[$id]->deleted = FALSE;

  // Store in cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Page callback to display analysis information on a view.
 */
function views_ui_analyze_view($js, $view) {
  views_include('ajax');
  $form_state = array(
    'view' => &$view,
    'ajax' => $js,
  );

  $output = views_ajax_form_wrapper('views_ui_analyze_view_form', $form_state);

  if ($js) {
    // If we don't have an output object, it was submitted. Set up the submission.
    if ($form_state['submitted'] && empty($form_state['rerender'])) {
      $output = array();
      $output[] = views_ajax_command_enable_buttons();
      $output[] = views_ajax_command_dismiss_form();
      views_ui_regenerate_tabs($view, $output);
    }
    $commands = $output;
    return $js ? array('#type' => 'ajax', '#commands' => $commands) : $commands;
  }
  return $output;
}

/**
 * This form doesn't particularly do much; it's really just providing a link
 * but a button seems like it would be nicer here.
 *
 * It has no submit or anything, as we will never actually submit this form
 * where the form is placed.
 */
function views_ui_analyze_view_button($form, &$form_state, $view) {
  $form['#action'] = url("admin/structure/views/nojs/analyze/$view->name");
  $form['#attributes'] = array('class' => array('views-ajax-form'));
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Analyze'),
    '#ajax' => array(
      'path' => "admin/structure/views/ajax/analyze/$view->name",
    ),
  );

  return $form;
}

/**
 * Form constructor callback to display analysis information on a view
 */
function views_ui_analyze_view_form($form, &$form_state) {
  $view = &$form_state['view'];

  $form['#title'] = t('View analysis');
  $form['#section'] = 'analyze';

  views_include('analyze');
  $messages = views_analyze_view($view);

  $form['analysis'] = array(
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
    '#markup' => views_analyze_format_result($view, $messages),
  );

  // Inform the standard button function that we want an OK button.
  $form_state['ok_button'] = TRUE;
  views_ui_standard_form_buttons($form, $form_state, 'views_ui_analyze_view_form');
  return $form;
}

/**
 * Submit handler for views_ui_analyze_view_form
 */
function views_ui_analyze_view_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/views/view/' . $form_state['view']->name . '/edit';
}

/**
 * Page callback to display analysis information on a view.
 */
function views_ui_reorder_view($js, $view) {
  views_include('ajax');
  $form_state = array(
    'view' => &$view,
    'ajax' => $js,
  );

  $output = views_ajax_form_wrapper('views_ui_reorder_displays_form', $form_state);

  if ($js) {
    if ($form_state['submitted'] && empty($form_state['rerender'])) {
      // I don't want preprocess to modify the views -> no references
      $vars = array('view' => $view);
      template_preprocess_views_ui_edit_view($vars);
      $output = array();
      $output[] = ajax_command_replace('.views-tabset', $vars['tabs']);
      // Not the right place to have html i know !
      $output[] = ajax_command_replace('.views-quick-links', '<div class="views-quick-links">'. $vars['quick_links'] .'</div>');
      $output[] = views_ajax_command_enable_buttons();
      $output[] = views_ajax_command_dismiss_form();
      // Doesn't work yet, maybe we should reload the page dunno
      views_ui_regenerate_tabs($view, $output);
    }
    $commands = $output;
    return $js ? array('#type' => 'ajax', '#commands' => $commands) : $commands;
  }
  return $output;
}

/**
 * Form constructor callback to reorder displays on a view
 */
function views_ui_reorder_displays_form($form, &$form_state) {
  $view = &$form_state['view'];

  $form['view'] = array('#type' => 'value', '#value' => $view);

  $form['#tree'] = TRUE;

  $last_display = end($view->display);

  foreach ($view->display as $display) {
    $form[$display->id] = array(
      'title'  => array('#markup' => $display->display_title),
      'weight' => array(
        '#type' => 'weight',
        '#value' => $display->position,
        '#delta' => $last_display->position,
        '#title' => t('Weight for @display', array('@display' => $display->display_title)),
        '#title_display' => 'invisible',
      ),
      '#tree' => TRUE,
      '#display' => $display,
      'removed' => array(
        '#type' => 'checkbox',
        '#id' => 'display-removed-' . $display->id,
        '#attributes' => array('class' => array('views-remove-checkbox')),
        '#default_value' => isset($display->deleted),
      ),
    );

    if (isset($display->deleted) && $display->deleted) {
      $form[$display->id]['deleted'] = array('#type' => 'value', '#value' => TRUE);
    }
    if ($display->id === 'default') {
      unset($form[$display->id]['weight']);
      unset($form[$display->id]['removed']);
    }

  }

  $form['#title'] = t('Displays Reorder');
  $form['#section'] = 'reorder';

  // Add javascript settings that will be added via $.extend for tabledragging
  $form['#js']['tableDrag']['reorder-displays']['weight'][0] = array(
    'target' => 'weight',
    'source' => NULL,
    'relationship' => 'sibling',
    'action' => 'order',
    'hidden' => TRUE,
    'limit' => 0,
  );

  $form['#action'] = url('admin/structure/views/nojs/reorder-displays/'. $view->name);

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_reorder_displays_form');

  return $form;
}

/**
 * Display position sorting function
 */
function _views_position_sort($display1, $display2) {
  if ($display1->position != $display2->position) {
    return $display1->position < $display2->position ? -1 : 1;
  }

  return 0;
}

/**
 * Submit handler for rearranging display form
 */
function views_ui_reorder_displays_form_submit($form, &$form_state) {
  foreach($form_state['input'] as $display => $info) {
    // add each value that is a field with a weight to our list, but only if
    // it has had its 'removed' checkbox checked.
    if (is_array($info) && isset($info['weight']) && empty($info['removed'])) {
      $order[$display] = $info['weight'];
    }
  }

  // Sort the order array
  asort($order);

  // Fixing up positions
  $position = 2;

  foreach(array_keys($order) as $display) {
    $order[$display] = $position++;
  }

  // Setting up position and removing deleted displays
  $displays = $form_state['view']->display;
  foreach($displays as $display_id => $display) {
    // Don't touch the default !!!
    if ($display_id === 'default') {
      continue;
    }
    if (isset($order[$display_id])) {
      $form_state['view']->display[$display_id]->position = $order[$display_id];
    }
    else {
      $form_state['view']->display[$display_id]->deleted = TRUE;
    }
  }

  // Sorting back the display array as the position is not enough
  uasort($form_state['view']->display, '_views_position_sort');

  // Store in cache
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = array('admin/structure/views/view/' . $form_state['view']->name . '/edit', array('fragment' => 'views-tab-default'));
}

/**
 * Turn the reorder form into a proper table
 */
function theme_views_ui_reorder_displays_form($vars) {
  $form = $vars['form'];
  $rows = array();
  foreach (element_children($form) as $key) {
    if (isset($form[$key]['#display'])) {
      $display = &$form[$key];

      $row = array();
      $row[] = drupal_render($display['title']);
      $form[$key]['weight']['#attributes']['class'] = array('weight');
      $row[] = drupal_render($form[$key]['weight']);
      if (isset($display['removed'])) {
        $row[] = drupal_render($form[$key]['removed']) . 
          l('<span>' . t('Remove') . '</span>', 
            'javascript:void()', 
            array(
              'attributes' => array(
                'id' => 'display-remove-link-' . $key, 
                'class' => array('views-button-remove display-remove-link'),
                'alt' => t('Remove this display'), 
                'title' => t('Remove this display')), 
              'html' => TRUE));
      }
      else {
        $row[] = '';
      }
      $class = array();
      $styles = array();
      if (isset($form[$key]['weight']['#type'])) {
        $class[] = 'draggable';
      }
      if (isset($form[$key]['deleted']['#value']) && $form[$key]['deleted']['#value']) {
        $styles[] = 'display: none;';
      }
      $rows[] = array('data' => $row, 'class' => $class, 'id' => 'display-row-' . $key, 'style' => $styles);
    }
  }

  $header = array(t('Display'), t('Weight'), t('Remove'));
  $output = '';
  drupal_add_tabledrag('reorder-displays', 'order', 'sibling', 'weight');

  $output = drupal_render($form['override']);
  $output .= theme('table',
    array('header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'reorder-displays'),
  ));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * This form doesn't particularly do much; it's really just providing a link
 * but a button seems like it would be nicer here.
 *
 * It has no submit or anything, as we will never actually submit this form
 * where the form is placed.
 */
function views_ui_reorder_displays_button($form, &$form_state, $view) {
  $form['#action'] = url("admin/structure/views/nojs/reorder-displays/$view->name");
  $form['#attributes'] = array('class' => array('views-ajax-form'));
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Reorder'),
    '#ajax' => array(
      'path' => "admin/structure/views/ajax/reorder-displays/$view->name",
    ),
  );

  return $form;
}

/**
 * Page callback to edit details of a view.
 */
function views_ui_edit_details($js, $view) {
  views_include('ajax');
  $form_state = array(
    'view' => &$view,
    'ajax' => $js,
  );

  $output = views_ajax_form_wrapper('views_ui_edit_details_form', $form_state);

  if ($js) {
    // If we don't have an output object, it was submitted. Set up the submission.
    if (empty($output)) {
      return views_ui_regenerate_tabs($view);
    }
    return $js ? array('#type' => 'ajax', '#commands' => $output) : $output;

  }
  return $output;
}

/**
 * Form constructor callback to edit details of a view
 */
function views_ui_edit_details_form($form, &$form_state) {
  $view = &$form_state['view'];

  $form['#title'] = t('View details');
  $form['#section'] = 'details';

  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('View description'),
    '#description' => t('This description will appear on the Views administrative UI to tell you what the view is about.'),
    '#default_value' => $view->description,
  );

  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('View tag'),
    '#description' => t('Enter an optional tag for this view; it is used only to help sort views on the administrative page.'),
    '#default_value' => $view->tag,
    '#autocomplete_path' => 'admin/views/ajax/autocomplete/tag',
  );

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_edit_details_form');
  return $form;
}

/**
 * Submit handler for views_ui_edit_details_form
 */
function views_ui_edit_details_form_submit($form, &$form_state) {
  $form_state['view']->description = $form_state['values']['description'];
  $form_state['view']->tag = $form_state['values']['tag'];
  views_ui_cache_set($form_state['view']);
  $form_state['redirect'] = 'admin/structure/views/view/' . $form_state['view']->name . '/edit';
}

/**
 * Form constructor callback to edit display of a view
 */
function views_ui_edit_display_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $section = $form_state['section'];

  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];

  // Get form from the handler.
  $display->handler->options_form($form, $form_state);
  $name = NULL;
  if (isset($form_state['update_name'])) {
    $name = $form_state['update_name'];
  }

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_edit_display_form', $name);
  return $form;
}

/**
 * Validate handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_validate($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_validate($form, $form_state);
}

/**
 * Submit handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_submit($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_submit($form, $form_state);

  views_ui_cache_set($form_state['view']);
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_override($form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_override($form, $form_state);

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}
/**
 * Override handler and submit views_ui_edit_display_form
 */
function views_ui_edit_display_form_override_update(&$form, &$form_state) {
  $display = &$form_state['view']->display[$form_state['display_id']];
  $display->handler->options_override($form, $form_state);
  $display->handler->options_submit($form, $form_state);
  views_ui_cache_set($form_state['view']);
}

/**
 * Override handler and submit views_ui_edit_display_form
 */
function views_ui_edit_display_form_override_update_section(&$form, &$form_state) {
  // Update the #section so it knows what to mark changed.
  $form['#section'] = str_replace('default-', $form_state['display_id'] . '-', $form['#section']);
}

/**
 * Form to config items in the views UI.
 */
function views_ui_config_type_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];

  $types = views_object_types();
  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];
  $form['#title'] = check_plain($display->display_title) . ': ';
  $form['#title'] .= t('Configure @type', array('@type' => $types[$type]['ltitle']));
  $form['#section'] = $display_id . 'config-item';

  if ($display->handler->defaultable_sections($types[$type]['plural'])) {
    $form_state['section'] = $types[$type]['plural'];
    $display->handler->add_override_button($form, $form_state, $form_state['section']);
  }

  if (!empty($types[$type]['options']) && function_exists($types[$type]['options'])) {
    $options = $type . '_options';
    $form[$options] = array('#tree' => TRUE);
    $types[$type]['options']($form, $form_state);
  }

  $name = NULL;
  if (isset($form_state['update_name'])) {
    $name = $form_state['update_name'];
  }

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_type_form', $name);
  return $form;
}

/**
 * Submit handler for type configuration form
 */
function views_ui_config_type_form_submit($form, &$form_state) {
  $types = views_object_types();
  $display = &$form_state['view']->display[$form_state['display_id']];

  // Store in cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Configure settings particular to filters.
 */
function views_ui_config_filters_form(&$form, &$form_state) {

}

/**
 * Form to rearrange items in the views UI.
 */
function views_ui_rearrange_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];

  $types = views_object_types();
  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];
  $form['#title'] = check_plain($display->display_title) . ': ';
  $form['#title'] .= t('Rearrange @type', array('@type' => $types[$type]['ltitle']));
  $form['#section'] = $display_id . 'rearrange-item';

  if ($display->handler->defaultable_sections($types[$type]['plural'])) {
    $form_state['section'] = $types[$type]['plural'];
    $display->handler->add_override_button($form, $form_state, $form_state['section']);
  }

  $count = 0;

  // Get relationship labels
  $relationships = array();
  foreach ($display->handler->get_handlers('relationship') as $id => $handler) {
    $relationships[$id] = $handler->label();
  }

  foreach ($display->handler->get_option($types[$type]['plural']) as $id => $field) {
    $form[$id] = array('#tree' => TRUE);
    $form[$id]['weight'] = array(
      '#type' => 'weight',
      '#delta' => 200,
      '#default_value' => ++$count,
    );
    $handler = $display->handler->get_handler($type, $id);
    if ($handler) {
      $name = $handler->ui_name() . ' ' . $handler->admin_summary();
      if (!empty($field['relationship']) && !empty($relationships[$field['relationship']])) {
        $name = '(' . $relationships[$field['relationship']] . ') ' . $name;
      }

      $form[$id]['name'] = array(
        '#markup' => $name,
      );
    }
    else {
      $form[$id]['name'] = array('#value' => t('Broken field @id', array('@id' => $id)));
    }
    $form[$id]['removed'] = array(
      '#type' => 'checkbox',
      '#id' => 'views-removed-' . $id,
      '#attributes' => array('class' => array('views-remove-checkbox')),
      '#default_value' => 0,
    );
  }

  // Add javascript settings that will be added via $.extend for tabledragging
  $form['#js']['tableDrag']['arrange']['weight'][0] = array(
    'target' => 'weight',
    'source' => NULL,
    'relationship' => 'sibling',
    'action' => 'order',
    'hidden' => TRUE,
    'limit' => 0,
  );

  $name = NULL;
  if (isset($form_state['update_name'])) {
    $name = $form_state['update_name'];
  }

  views_ui_standard_form_buttons($form, $form_state, 'views_ui_rearrange_form');
  return $form;
}

/**
 * Turn the rearrange form into a proper table
 */
function theme_views_ui_rearrange_form($variables) {
  $form = $variables['form'];

  $rows = array();
  foreach (element_children($form) as $id) {
    if (isset($form[$id]['name'])) {
      $row = array();
      $row[] = drupal_render($form[$id]['name']);
      $form[$id]['weight']['#attributes']['class'] = array('weight');
      $row[] = drupal_render($form[$id]['weight']);
      $row[] = drupal_render($form[$id]['removed']) . l('<span>' . t('Remove') . '</span>', 'javascript:void()', array('attributes' => array('id' => 'views-remove-link-' . $id, 'class' => array('views-button-remove', 'views-remove-link'), 'alt' => t('Remove this item'), 'title' => t('Remove this item')), 'html' => true));
      $rows[] = array('data' => $row, 'class' => array('draggable'), 'id' => 'views-row-' . $id);
    }
  }
  if (empty($rows)) {
    $rows[] = array(array('data' => t('No fields available.'), 'colspan' => '2'));
  }

  $header = array('', t('Weight'), t('Remove'));
  $output = drupal_render($form['override']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'arrange')));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag('arrange', 'order', 'sibling', 'weight');

  return $output;
}

/**
 * Submit handler for rearranging form
 */
function views_ui_rearrange_form_submit($form, &$form_state) {
  $types = views_object_types();
  $display = &$form_state['view']->display[$form_state['display_id']];

  $old_fields = $display->handler->get_option($types[$form_state['type']]['plural']);
  $new_fields = $order = array();

  // Make an array with the weights
  foreach ($form_state['values'] as $field => $info) {
    // add each value that is a field with a weight to our list, but only if
    // it has had its 'removed' checkbox checked.
    if (is_array($info) && isset($info['weight']) && empty($info['removed'])) {
      $order[$field] = $info['weight'];
    }
  }

  // Sort the array
  asort($order);

  // Create a new list of fields in the new order.
  foreach (array_keys($order) as $field) {
    $new_fields[$field] = $old_fields[$field];
  }
  $display->handler->set_option($types[$form_state['type']]['plural'], $new_fields);

  // Store in cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to add_item items in the views UI.
 */
function views_ui_add_item_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];

  ctools_add_js('dependent');

  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $display = &$view->display[$display_id];

  $types = views_object_types();

  if (!empty($types[$type]['type'])) {
    $type = $types[$type]['type'];
  }

  $form['#title'] = check_plain($display->display_title) . ': ';
  $form['#title'] .= t('Add @type', array('@type' => $types[$type]['ltitle']));
  $form['#section'] = $display_id . 'add-item';

  // Figure out all the base tables allowed based upon what the relationships provide.
  $base_tables = $view->get_base_tables();
  $options = views_fetch_fields(array_keys($base_tables), $type, $display->handler->use_group_by());

  if (!empty($options)) {
    $groups = array('all' => t('- All -'));
    $form['group'] = array(
      '#type' => 'select',
      '#title' => t('Groups'),
      '#options' => array(),
      '#attributes' => array('class' => array('ctools-master-dependent')),
    );

    $form['name'] = array(
      '#prefix' => '<div class="views-radio-box form-checkboxes">',
      '#suffix' => '</div>',
      '#tree' => TRUE,
      '#default_value' => 'all',
    );

    // Group options first to simplify the DOM objects that Views
    // dependent JS will act upon.
    $grouped_options = array();
    foreach ($options as $key => $option) {
      $group = preg_replace('/[^a-z0-9]/', '-', strtolower($option['group']));
      $groups[$group] = $option['group'];
      $grouped_options[$group][$key] = $option;
    }

    foreach ($grouped_options as $group => $group_options) {
      $form['name'][$group . '_start']['#markup'] = '<div class="ctools-dependent-all ctools-dependent-' . $group . '">';
      foreach ($group_options as $key => $option) {
        $form['name'][$key] = array(
          '#type' => 'checkbox',
          '#title' => t('!group: !field', array('!group' => $option['group'], '!field' => $option['title'])),
          '#description' => $option['help'],
          '#return_value' => $key,
        );
      }
      $form['name'][$group . '_end']['#markup'] = '</div>';
    }

    $form['group']['#options'] = $groups;
  }
  else {
    $form['markup'] = array(
      '#value' => '<div class="form-item">' . t('There are no @types available to add.', array('@types' =>  $types[$type]['ltitle'])) . '</div>',
    );
  }
  ctools_include('dependent');
  views_ui_standard_form_buttons($form, $form_state, 'views_ui_add_item_form', t('Add'));

  return $form;
}

/**
 * Submit handler for adding new item(s) to a view.
 */
function views_ui_add_item_form_submit($form, &$form_state) {
  $type = $form_state['type'];
  $types = views_object_types();

  if (!empty($form_state['values']['name']) && is_array($form_state['values']['name'])) {
    // Loop through each of the items that were checked and add them to the view.
    foreach (array_keys(array_filter($form_state['values']['name'])) as $field) {
      list($table, $field) = explode('.', $field, 2);
      $id = $form_state['view']->add_item($form_state['display_id'], $type, $table, $field);

      // check to see if we have group by settings
      if ($form_state['view']->display_handler->use_group_by()) {
        views_ui_add_form_to_stack('config-item-group', $form_state['view'], $form_state['display_id'], array($type, $id));
      }

      // check to see if this type has settings, if so add the settings form first
      $handler = views_get_handler($table, $field, $type);
      if ($handler && $handler->has_extra_options()) {
        views_ui_add_form_to_stack('config-item-extra', $form_state['view'], $form_state['display_id'], array($type, $id));
      }
      // Then add the form to the stack
      views_ui_add_form_to_stack('config-item', $form_state['view'], $form_state['display_id'], array($type, $id));
    }
  }

  // Store in cache
  views_ui_cache_set($form_state['view']);
}


/**
 * Form to config_item items in the views UI.
 */
function views_ui_config_item_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = $view->display_handler->get_handler($type, $id);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
    }
    else {
      $types = views_object_types();

      if ($view->display_handler->defaultable_sections($types[$type]['plural'])) {
        $form_state['section'] = $types[$type]['plural'];
        $view->display_handler->add_override_button($form['options'], $form_state, $form_state['section']);
      }

      // A whole bunch of code to figure out what relationships are valid for
      // this item.
      $relationships = $view->display_handler->get_option('relationships');
      $relationship_options = array();

      foreach ($relationships as $relationship) {
        // relationships can't link back to self. But also, due to ordering,
        // relationships can only link to prior relationships.
        if ($type == 'relationship' && $id == $relationship['id']) {
          break;
        }
        $relationship_handler = views_get_handler($relationship['table'], $relationship['field'], 'relationship');
        // ignore invalid/broken relationships.
        if (empty($relationship_handler)) {
          continue;
        }

        // If this relationship is valid for this type, add it to the list.
        $data = views_fetch_data($relationship['table']);
        $base = $data[$relationship['field']]['relationship']['base'];
        $base_fields = views_fetch_fields($base, $form_state['type'], $view->display_handler->use_group_by());
        if (isset($base_fields[$item['table'] . '.' . $item['field']])) {
          $relationship_handler->init($view, $relationship);
          $relationship_options[$relationship['id']] = $relationship_handler->label();
        }
      }

      if (!empty($relationship_options)) {
        // Make sure the existing relationship is even valid. If not, force
        // it to none.
        $base_fields = views_fetch_fields($view->base_table, $form_state['type'], $view->display_handler->use_group_by());
        if (isset($base_fields[$item['table'] . '.' . $item['field']])) {
          $relationship_options = array_merge(array('none' => t('Do not use a relationship')), $relationship_options);
        }
        $rel = empty($item['relationship']) ? 'none' : $item['relationship'];
        if (empty($relationship_options[$rel])) {
          // Pick the first relationship.
          $rel = key($relationship_options);
          // We want this relationship option to get saved even if the user
          // skips submitting the form.
          $view->set_item_option($display_id, $type, $id, 'relationship', $rel);
          $temp_view = $view->clone_view();
          views_ui_cache_set($temp_view);
        }

        $form['options']['relationship'] = array(
          '#type' => 'select',
          '#title' => t('Relationship'),
          '#options' => $relationship_options,
          '#default_value' => $rel,
        );
      }
      else {
        $form['options']['relationship'] = array(
          '#type' => 'value',
          '#value' => 'none',
        );
      }

      $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
      $form['#title'] .= t('Configure @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

      $form['#section'] = $display_id . '-' . $type . '-' . $id;

      // Get form from the handler.
      $handler->options_form($form['options'], $form_state);
      $form_state['handler'] = &$handler;
    }

    $name = NULL;
    if (isset($form_state['update_name'])) {
      $name = $form_state['update_name'];
    }

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_item_form', $name, t('Remove'), 'remove');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_form_validate($form, &$form_state) {
  $form_state['handler']->options_validate($form['options'], $form_state);
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['options'], $form_state);
  $item = $form_state['handler']->options;
  $types = views_object_types();

  $type = $form_state['type'];
  if (!empty($types[$type]['type'])) {
    $type = $types[$type]['type'];
  }

  // Create a new handler and unpack the options from the form onto it. We
  // can use that for storage.
  $handler = views_get_handler($item['table'], $item['field'], $type);


  // Add the incoming options to existing options because items using
  // the extra form may not have everything in the form here.
  $options = $form_state['values']['options'] + $form_state['handler']->options;

  // This unpacks only options that are in the definition, ensuring random
  // extra stuff on the form is not sent through.
  $handler->unpack_options($handler->options, $options, NULL, FALSE);

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $handler->options);
  if ($form_state['handler'] && $form_state['handler']->needs_style_plugin()) {
    views_ui_add_form_to_stack('change-style', $form_state['view'], $form_state['display_id'], array($form_state['type'], $form_state['id']), TRUE);
  }

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to config_item items in the views UI.
 */
function views_ui_config_item_group_form($type, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_render(t('Invalid display id @display', array('@display' => $display_id)));
  }

  $view->init_query();

  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = $view->display_handler->get_handler($type, $id);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
    }
    else {
      $handler->init($view, $item);
      $types = views_object_types();

      $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
      $form['#title'] .= t('Configure group settings for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

      $form['#section'] = $display_id . '-' . $type . '-' . $id;

      $info = $view->query->get_aggregation_info();
      foreach ($info as $id => $aggregate) {
        $group_types[$id] = $aggregate['title'];
      }

      $form['group_type'] = array(
        '#type' => 'select',
        '#title' => t('Group type'),
        '#default_value' => $handler->options['group_type'],
        '#description' => t('Grouping is enabled for this display. You must select what function to use on this field.'),
        '#options' => $group_types,
      );
      $form_state['handler'] = &$handler;
    }

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_item_group_form');
  }
  return $form;
}

/**
 * Submit handler for configing group settings on a view.
 */
function views_ui_config_item_group_form_submit($form, &$form_state) {
  $item = $form_state['handler']->options;

  $item['group_type'] = $form_state['values']['group_type'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Submit handler for removing an item from a view
 */
function views_ui_config_item_form_remove($form, &$form_state) {
  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], NULL);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_config_item_form_expose($form, &$form_state) {
  $item = &$form_state['handler']->options;
  // flip
  $item['exposed'] = empty($item['exposed']);

  // If necessary, set new defaults:
  if ($item['exposed']) {
    $form_state['handler']->expose_options();
  }

  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
  $form_state['force_expose_options'] = TRUE;
}

/**
 * Form to config_item items in the views UI.
 */
function views_ui_config_item_extra_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
      break;
    }
    else {
      $handler->init($view, $item);
      $types = views_object_types();

      $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
      $form['#title'] .= t('Configure extra settings for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

      $form['#section'] = $display_id . '-' . $type . '-' . $id;

      // Get form from the handler.
      $handler->extra_options_form($form['options'], $form_state);
      $form_state['handler'] = &$handler;
    }

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_item_extra_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_extra_form_validate($form, &$form_state) {
  $form_state['handler']->extra_options_validate($form['options'], $form_state);
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_item_extra_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->extra_options_submit($form['options'], $form_state);
  $item = $form_state['handler']->options;

  // Store the data we're given.
  foreach ($form_state['values']['options'] as $key => $value) {
    $item[$key] = $value;
  }

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to change_style items in the views UI.
 */
function views_ui_change_style_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();
    $form['#title'] = t('Change summary style for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

    $form['#section'] = $display_id . '-' . $type . '-' . $id . '-style-plugin';

    $form['style_plugin'] =  array(
      '#type' => 'radios',
      '#options' => views_fetch_plugin_names('style', 'summary'),
      '#default_value' => $item['style_plugin'],
    );

    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_change_style_form');
  }
  return $form;
}

function views_ui_change_style_form_validate($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_validate($form['options'], $form_state);

  $plugin = views_get_plugin('style', $form_state['values']['style_plugin']);
  if (!$plugin) {
    form_error($form['style_plugin'], t('Internal error: broken plugin.'));
  }
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_change_style_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['options'], $form_state);
  $item = $form_state['handler']->options;

  $plugin = views_get_plugin('style', $form_state['values']['style_plugin']);
  if (!$plugin) {
    drupal_set_message(t('Internal error: broken plugin.'), 'error');
    return;
  }

  $plugin->init($form_state['view'], $form_state['view']->display[$form_state['display_id']]);

  // If changing style plugin, reset options to defaults.
  if (empty($item['style_plugin']) || $item['style_plugin'] != $form_state['values']['style_plugin']) {
    $item['style_options'] = $plugin->options;
  }

  // Store the data we're given.
  $item['style_plugin'] = $form_state['values']['style_plugin'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  if (!empty($plugin->definition['uses options'])) {
    views_ui_add_form_to_stack('config-style', $form_state['view'], $form_state['display_id'], array($form_state['type'], $form_state['id']), TRUE);
  }

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Form to config_style items in the views UI.
 */
function views_ui_config_style_form($form, &$form_state) {
  $view = &$form_state['view'];
  $display_id = $form_state['display_id'];
  $type = $form_state['type'];
  $id = $form_state['id'];

  $form = array('options' => array('#tree' => TRUE));
  if (!$view->set_display($display_id)) {
    views_ajax_error(t('Invalid display id @display', array('@display' => $display_id)));
  }
  $item = $view->get_item($display_id, $type, $id);

  if ($item) {
    $handler = views_get_handler($item['table'], $item['field'], $type);
    if (empty($handler)) {
      $form['markup'] = array('#value' => t("Error: handler for @table > @field doesn't exist!", array('@table' => $item['table'], '@field' => $item['field'])));
      break;
    }
    $handler->init($view, $item);
    $types = views_object_types();

    $form['#title'] = check_plain($view->display[$display_id]->display_title) . ': ';
    $form['#title'] .= t('Configure summary style for @type %item', array('@type' => $types[$type]['lstitle'], '%item' => $handler->ui_name()));

    $form['#section'] = $display_id . '-' . $type . '-style-options';

    $plugin = views_get_plugin('style', $handler->options['style_plugin']);
    if ($plugin) {
      $form['style_options'] = array(
        '#tree' => TRUE,
      );
      $plugin->init($view, $view->display[$display_id], $handler->options['style_options']);

      $plugin->options_form($form['style_options'], $form_state);
    }

    $form_state['handler'] = &$handler;

    views_ui_standard_form_buttons($form, $form_state, 'views_ui_config_style_form');
  }
  return $form;
}

/**
 * Submit handler for configing new item(s) to a view.
 */
function views_ui_config_style_form_submit($form, &$form_state) {
  // Run it through the handler's submit function.
  $form_state['handler']->options_submit($form['style_options'], $form_state);
  $item = $form_state['handler']->options;

  // Store the data we're given.
  $item['style_options'] = $form_state['values']['style_options'];

  // Store the item back on the view
  $form_state['view']->set_item($form_state['display_id'], $form_state['type'], $form_state['id'], $item);

  // Write to cache
  views_ui_cache_set($form_state['view']);
}

/**
 * Get a list of roles in the system.
 */
function views_ui_get_roles() {
  static $roles = NULL;
  if (!isset($roles)) {
    $roles = array();
    $result = db_query("SELECT r.rid, r.name FROM {role} r ORDER BY r.name");
    foreach ($result as $obj) {
      $roles[$obj->rid] = $obj->name;
    }
  }

  return $roles;
}

/**
 * Page callback for the Views enable page.
 */
function views_ui_enable_page($view) {
  $views_status = variable_get('views_defaults', array());
  $views_status[$view->name] = FALSE; // FALSE is enabled
  variable_set('views_defaults', $views_status);
  views_invalidate_cache();
  menu_rebuild();
  drupal_goto('admin/structure/views');
}

/**
 * Page callback for the Views enable page
 */
function views_ui_disable_page($view) {
  $views_status = variable_get('views_defaults', array());
  $views_status[$view->name] = TRUE; // True is disabled
  variable_set('views_defaults', $views_status);
  views_invalidate_cache();
  menu_rebuild();
  drupal_goto('admin/structure/views');
}

/**
 * Page callback for the tools - other page
 */
function views_ui_admin_tools() {
  $form['clear_cache'] = array(
    '#type' => 'submit',
    '#value' => t("Clear Views' cache"),
    '#submit' => array('views_ui_tools_clear_cache'),
  );

  $form['views_sql_signature'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add Views signature to all SQL queries'),
    '#description' => t("All Views-generated queries will include a special 'VIEWS' = 'VIEWS' string in the WHERE clause. This makes identifying Views queries in database server logs simpler, but should only be used when troubleshooting."),
    '#default_value' => variable_get('views_sql_signature', FALSE),
  );

  $form['views_skip_cache'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable views data caching'),
    '#description' => t("Views caches data about tables, modules and views available, to increase performance. By checking this box, Views will skip this cache and always rebuild this data when needed. This can have a serious performance impact on your site."),
    '#default_value' => variable_get('views_skip_cache', FALSE),
  );

  $form['views_hide_help_message'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ignore missing advanced help module'),
    '#description' => t("Views uses the advanced help module to provide help text; if this module is not present Views will complain, unless this setting is checked."),
    '#default_value' => variable_get('views_hide_help_message', FALSE),
  );

  $form['views_ui_query_on_top'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show query above live preview'),
    '#description' => t("The live preview feature will show you the output of the view you're creating, as well as the view. Check here to show the query and other information above the view; leave this unchecked to show that information below the view."),
    '#default_value' => variable_get('views_ui_query_on_top', FALSE),
  );

  $form['views_ui_disable_live_preview'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable automatic live preview'),
    '#description' => t("Don't automatically update the preview. This can speed up the editing of views a bit.'"),
    '#default_value' => variable_get('views_ui_disable_live_preview', 0),
  );

  $form['views_show_additional_queries'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show other queries run during render during live preview'),
    '#description' => t("Drupal has the potential to run many queries while a view is being rendered. Checking this box will display every query run during view render as part of the live preview."),
    '#default_value' => variable_get('views_show_additional_queries', FALSE),
  );

  $form['views_no_hover_links'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show hover links over views'),
    '#description' => t("To make it easier to administrate your views, Views provides 'hover' links to take you to the edit and export screen of a view whenever the view is used. This can be distracting on some themes, though; if it is problematic, you can turn it off here."),
    '#default_value' => variable_get('views_no_hover_links', FALSE),
  );

  $form['views_devel_output'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable views performance statistics via the Devel module'),
    '#description' => t("Check this to enable some Views query and performance statistics <em>if Devel is installed</em>."),
    '#default_value' => variable_get('views_devel_output', FALSE),
  );

  $form['views_no_javascript'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable javascript with Views'),
    '#description' => t("If you are having problems with the javascript, you can disable it here; the Views UI should degrade and still be usable without javascript, it just not as good."),
    '#default_value' => variable_get('views_no_javascript', FALSE),
  );

  $regions = system_region_list(variable_get('theme_default', 'garland'));
  $regions['watchdog'] = t('Watchdog');

  $form['views_devel_region'] = array(
    '#type' => 'select',
    '#title' => t('Page region to output performance statistics'),
    '#default_value' => variable_get('views_devel_region', 'footer'),
    '#options' => $regions,
  );

  $form['views_exposed_filter_any_label'] = array(
    '#type' => 'select',
    '#title' => t('Label for "Any" value on optional single-select exposed filters'),
    '#options' => array('old_any' => '<Any>', 'new_any' => t('- Any -')),
    '#default_value' => variable_get('views_exposed_filter_any_label', 'old_any'),
  );

  return system_settings_form($form);
}

/**
 * Submit hook to clear the views cache.
 */
function views_ui_tools_clear_cache() {
  views_invalidate_cache();
  drupal_set_message(t('The cache has been cleared.'));
}

/**
 * Submit hook to clear Drupal's theme registry (thereby triggering
 * a templates rescan).
 */
function views_ui_config_item_form_rescan($form, &$form_state) {
  drupal_theme_rebuild();

  // The 'Theme: Information' page is about to be shown again. That page
  // analyzes the output of theme_get_registry(). However, this latter
  // function uses an internal cache (which was initialized before we
  // called drupal_theme_rebuild()) so it won't reflect the
  // current state of our theme registry. The only way to clear that cache
  // is to re-initialize the theme system:
  unset($GLOBALS['theme']);
  drupal_theme_initialize();

  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}

/**
 * Override handler for views_ui_edit_display_form
 */
function views_ui_edit_display_form_change_theme($form, &$form_state) {
  // This is just a temporary variable.
  $form_state['view']->theme = $form_state['values']['theme'];

  views_ui_cache_set($form_state['view']);
  $form_state['rerender'] = TRUE;
  $form_state['rebuild'] = TRUE;
}

/**
 * Page callback for views tag autocomplete
 */
function views_ui_autocomplete_tag($string = '') {
  $matches = array();
  // get matches from default views:
  views_include('view');
  $views = views_discover_default_views();
  foreach ($views as $view) {
    if (!empty($view->tag) && strpos($view->tag, $string) === 0) {
      $matches[$view->tag] = $view->tag;
    }
  }

  $count = 10 - count($matches);
  if ($string) {
    $result = db_query_range("SELECT DISTINCT tag FROM {views_view} WHERE LOWER(tag) LIKE LOWER(:tag)",
      0, $count,
      array(':tag' => $string . '%'));
    foreach ($result as $view) {
      $matches[$view->tag] = check_plain($view->tag);
    }
  }

  drupal_json_output($matches);
}

// ------------------------------------------------------------------
// Get information from the Views data

function _views_weight_sort($a, $b) {
  if ($a['weight'] != $b['weight']) {
    return $a['weight'] < $b['weight'] ? -1 : 1;
  }
  if ($a['title'] != $b['title']) {
    return $a['title'] < $b['title'] ? -1 : 1;
  }

  return 0;
}

/**
 * Fetch a list of all base tables available
 *
 * @return
 *   A keyed array of in the form of 'base_table' => 'Description'.
 */
function views_fetch_base_tables() {
  static $base_tables = array();
  if (empty($base_tables)) {
    $weights = array();
    $tables = array();
    $data = views_fetch_data();
    foreach ($data as $table => $info) {
      if (!empty($info['table']['base'])) {
        $tables[$table] = array(
          'title' => $info['table']['base']['title'],
          'description' => $info['table']['base']['help'],
          'weight' => !empty($info['table']['base']['weight']) ? $info['table']['base']['weight'] : 0,
        );
      }
    }
    uasort($tables, '_views_weight_sort');
    $base_tables = $tables;
  }

  return $base_tables;
}

function _views_sort_types($a, $b) {
  if ($a['group'] != $b['group']) {
    return $a['group'] < $b['group'] ? -1 : 1;
  }

  if ($a['title'] != $b['title']) {
    return $a['title'] < $b['title'] ? -1 : 1;
  }

  return 0;
}

/**
 * Fetch a list of all fields available for a given base type.
 *
 * @return
 *   A keyed array of in the form of 'base_table' => 'Description'.
 */
function views_fetch_fields($base, $type, $grouping = FALSE) {
  static $fields = array();
  if (empty($fields)) {
    $data = views_fetch_data();
    $start = microtime(TRUE);
    // This constructs this ginormous multi dimensional array to
    // collect the important data about fields. In the end,
    // the structure looks a bit like this (using nid as an example)
    // $strings['nid']['filter']['title'] = 'string'.
    //
    // This is constructed this way because the above referenced strings
    // can appear in different places in the actual data structure so that
    // the data doesn't have to be repeated a lot. This essentially lets
    // each field have a cheap kind of inheritance.

    foreach ($data as $table => $table_data) {
      $bases = array();
      $strings = array();
      $skip_bases = array();
      foreach ($table_data as $field => $info) {
        // Collect table data from this table
        if ($field == 'table') {
          // calculate what tables this table can join to.
          if (!empty($info['join'])) {
            $bases = array_keys($info['join']);
          }
          // And it obviously joins to itself.
          $bases[] = $table;
          continue;
        }
        foreach (array('field', 'sort', 'filter', 'argument', 'relationship', 'area') as $key) {
          if (!empty($info[$key])) {
            if ($grouping && !empty($info[$key]['no group by'])) {
              continue;
            }
            if (!empty($info[$key]['skip base'])) {
              foreach ((array) $info[$key]['skip base'] as $base_name) {
                $skip_bases[$field][$key][$base_name] = TRUE;
              }
            }
            elseif (!empty($info['skip base'])) {
              foreach ((array) $info['skip base'] as $base_name) {
                $skip_bases[$field][$key][$base_name] = TRUE;
              }
            }
            foreach (array('title', 'group', 'help', 'base') as $string) {
              // First, try the lowest possible level
              if (!empty($info[$key][$string])) {
                $strings[$field][$key][$string] = $info[$key][$string];
              }
              // Then try the field level
              elseif (!empty($info[$string])) {
                $strings[$field][$key][$string] = $info[$string];
              }
              // Finally, try the table level
              elseif (!empty($table_data['table'][$string])) {
                $strings[$field][$key][$string] = $table_data['table'][$string];
              }
              else {
                if ($string != 'base') {
                  $strings[$field][$key][$string] = t("Error: missing @component", array('@component' => $string));
                }
              }
            }
          }
        }
      }
      foreach ($bases as $base_name) {
        foreach ($strings as $field => $field_strings) {
          foreach ($field_strings as $type_name => $type_strings) {
            if (empty($skip_bases[$field][$type_name][$base_name])) {
              $fields[$base_name][$type_name]["$table.$field"] = $type_strings;
            }
          }
        }
      }
    }
//    vsm('Views UI data build time: ' . (views_microtime() - $start) * 1000 . ' ms');
  }

  // If we have an array of base tables available, go through them
  // all and add them together. Duplicate keys will be lost and that's
  // Just Fine.
  if (is_array($base)) {
    $strings = array();
    foreach ($base as $base_table) {
      if (isset($fields[$base_table][$type])) {
        $strings += $fields[$base_table][$type];
      }
    }
    uasort($strings, '_views_sort_types');
    return $strings;
  }

  if (isset($fields[$base][$type])) {
    uasort($fields[$base][$type], '_views_sort_types');
    return $fields[$base][$type];
  }
  return array();
}

/**
 * Fetch a list of all base tables available
 *
 * @param $type
 *   Either 'display', 'style' or 'row'
 * @param $key
 *   For style plugins, this is an optional type to restrict to. May be 'normal',
 *   'summary', 'feed' or others based on the neds of the display.
 * @param $base
 *   An array of possible base tables.
 *
 * @return
 *   A keyed array of in the form of 'base_table' => 'Description'.
 */
function views_fetch_plugin_names($type, $key = NULL, $base = array()) {
  $data = views_fetch_plugin_data();

  $plugins[$type] = array();

  foreach ($data[$type] as $id => $plugin) {
    // Skip plugins that don't conform to our key.
    if ($key && (empty($plugin['type']) || $plugin['type'] != $key)) {
      continue;
    }
    if (empty($plugin['no ui']) && (empty($base) || empty($plugin['base']) || array_intersect($base, $plugin['base']))) {
      $plugins[$type][$id] = $plugin['title'];
    }
  }

  if (!empty($plugins[$type])) {
    asort($plugins[$type]);
    return $plugins[$type];
  }
  // fall-through
  return array();
}


/**
 * Theme the form for the table style plugin
 */
function theme_views_ui_style_plugin_table($variables) {
  $form = $variables['form'];

  $output = drupal_render($form['description_markup']);

  $header = array(
    t('Field'),
    t('Column'),
    t('Align'),
    t('Separator'),
    array(
      'data' => t('Sortable'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default order'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default sort'),
      'align' => 'center',
    ),
  );
  $rows = array();
  foreach (element_children($form['columns']) as $id) {
    $row = array();
    $row[] = drupal_render($form['info'][$id]['name']);
    $row[] = drupal_render($form['columns'][$id]);
    $row[] = drupal_render($form['info'][$id]['align']);
    $row[] = drupal_render($form['info'][$id]['separator']);
    if (!empty($form['info'][$id]['sortable'])) {
      $row[] = array(
        'data' => drupal_render($form['info'][$id]['sortable']),
        'align' => 'center',
      );
      $row[] = array(
        'data' => drupal_render($form['info'][$id]['default_sort_order']),
        'align' => 'center',
      );
      $row[] = array(
        'data' => drupal_render($form['default'][$id]),
        'align' => 'center',
      );
    }
    else {
      $row[] = '';
      $row[] = '';
      $row[] = '';
    }
    $rows[] = $row;
  }

  // Add the special 'None' row.
  $rows[] = array(t('None'), '', '', '', '', array('align' => 'center', 'data' => drupal_render($form['default'][-1])));

  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Placeholder function for overriding $display->display_title.
 *
 * @todo Remove this function once editing the display title is possible.
 */
function views_ui_get_display_label($view, $display_id) {
  return $display_id == 'default' ? t('Master') : ucwords($display_id);
}
