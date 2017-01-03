<?php

/**
 * The settings form for Devel module.
 */
function devel_admin_settings() {
  $config = config('devel.settings');
  $form['queries'] = array(
    '#type' => 'fieldset',
    '#title' => t('Query log'),
  );

  $description = t('Display a log of the database queries needed to generate the current page, and the execution time for each. Also, queries which are repeated during a single page view are summed in the # column, and printed in red since they are candidates for caching.');
  if (!devel_is_compatible_optimizer()) {
    $description = t('You must disable or upgrade the php Zend Optimizer extension in order to enable this feature. The minimum required version is 3.2.8. Earlier versions of Zend Optimizer are <a href="!url">horribly buggy and segfault your Apache</a> ... ', array('!url' => url('http://drupal.org/node/126098'))) . $description;
  }
  $form['queries']['query_display'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display query log'),
    '#default_value' => $config->get('query_display', 0),
    '#description' => $description,
    '#disabled' => !devel_is_compatible_optimizer(),
  );
  $form['queries']['settings'] = array(
    '#type' => 'container',
      '#states' => array(
        // Hide the query log settings when not displaying query log.
        'invisible' => array(
          'input[name="query_display"]' => array('checked' => FALSE),
        ),
      ),
  );
  $form['queries']['settings']['query_sort'] = array(
    '#type' => 'radios',
    '#title' => t('Sort query log'),
    '#default_value' =>   $config->get('query_sort', DEVEL_QUERY_SORT_BY_SOURCE),
    '#options' => array(t('by source'), t('by duration')),
    '#description' => t('The query table can be sorted in the order that the queries were executed or by descending duration.'),
  );
  $form['queries']['settings']['query_slow_limit'] = array('#type' => 'textfield',
    '#title' => t('Slow query highlighting'),
    '#default_value' => $config->get('query_slow_limit'),
    '#size' => 4,
    '#maxlength' => 4,
    '#description' => t('Enter an integer in milliseconds. Any query which takes longer than this many milliseconds will be highlighted in the query log. This indicates a possibly inefficient query, or a candidate for caching.'),
  );

  $form['xhprof'] = array(
    '#type' => 'fieldset',
    '#title' => 'XHProf',
    '#description' => t('XHProf is a php extension which is essential for profiling your site. It pinpoints slow functions, and also memory hogging functions.'),
  );
  $description = extension_loaded('xhprof') ? t('Profile requests with the xhprof php extension.') : '<span class="warning">' . t('You must enable the <a href="!url">xhprof php extension</a> to use this feature.', array('!url' => url('http://techportal.ibuildings.com/2009/12/01/profiling-with-xhprof/'))) . '</span>';
  $form['xhprof']['xhprof_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable profiling of all page views and <a href="!drush">drush</a> requests.', array('!drush' => url('http://drush.ws'))),
    '#default_value' => $config->get('xhprof_enabled'),
    '#description' => $description,
    '#disabled' => !extension_loaded('xhprof'),
  );
  $form['xhprof']['settings'] = array(
    '#type' => 'container',
    '#states' => array(
      'invisible' => array(
        'input[name="xhprof_enabled"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['xhprof']['settings']['xhprof_directory'] = array(
    '#type' => 'textfield',
    '#title' => 'xhprof directory',
    '#description' => t('Location of the xhprof source code on your system, usually somewhere in /usr/local/share or /usr/share, include the leading forward slash.'),
    '#default_value' => $config->get('xhprof_directory'),
    '#states' => array(
      'invisible' => array(
        'input[name="xhprof_enabled"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['xhprof']['settings']['xhprof_url'] = array(
    '#type' => 'textfield',
    '#title' => 'XHProf URL',
    '#description' => t('Path to the publically accessible xhprof_html - required to display profiler reports. You will need to set this up outside your site directory, for example at http://xhprof.localhost/xhprof_html'),
    '#default_value' => $config->get('xhprof_url'),
    '#states' => array(
      'invisible' => array(
        'input[name="xhprof_enabled"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('API Site'),
    '#default_value' => $config->get('api_url'),
    '#description' => t('The base URL for your developer documentation links. You might change this if you run <a href="!url">api.module</a> locally.', array('!url' => url('http://drupal.org/project/api'))));
  $form['timer_display'] = array('#type' => 'checkbox',
    '#title' => t('Display page timer'),
    '#default_value' => $config->get('timer_display'),
    '#description' => t('Display page execution time in the query log box.'),
  );

  $form['memory_display'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display memory usage'),
    '#default_value' => $config->get('memory_display'),
    '#description' => t('Display how much memory is used to generate the current page. This will show memory usage when devel_init() is called and when devel_exit() is called.'),
  );
  $form['redirect_page'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display redirection page'),
    '#default_value' => $config->get('redirect_page'),
    '#description' => t('When a module executes backdrop_goto(), the query log and other developer information is lost. Enabling this setting presents an intermediate page to developers so that the log can be examined before continuing to the destination page.'),
  );
  $form['permission_raw_names'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display machine names of permissions and modules'),
    '#default_value' => $config->get('permission_raw_names'),
    '#description' => t('Display the language-independent machine names of the permissions in mouse-over hints on the !Permissions page and the module base file names on the @Permissions and !Modules pages.', array('!Permissions' => l(t('Permissions'), 'admin/people/permissions'), '@Permissions' => t('Permissions'), '!Modules' => l(t('Modules'), 'admin/modules'))),
  );

  $error_handlers = devel_get_handlers();
  $form['devel_error_handlers'] = array(
    '#type' => 'select',
    '#title' => t('Error handlers'),
    '#options' => array(
      DEVEL_ERROR_HANDLER_NONE => t('None'),
      DEVEL_ERROR_HANDLER_STANDARD => t('Standard Backdrop'),
      DEVEL_ERROR_HANDLER_BACKTRACE_DPM => t('Krumo backtrace in the message area'),
      DEVEL_ERROR_HANDLER_BACKTRACE_KRUMO => t('Krumo backtrace above the rendered page'),
    ),
    '#multiple' => TRUE,
    '#default_value' => empty($error_handlers) ? DEVEL_ERROR_HANDLER_STANDARD : $error_handlers,
    '#description' => t('Select the error handler(s) to use, in case you <a href="@choose">choose to show errors on screen</a>.', array('@choose' => url('admin/config/development/logging'))) . '<ul>' .
                      '<li>' . t('<em>None</em> is a good option when stepping through the site in your debugger.') . '</li>' .
                      '<li>' . t('<em>Standard Backdrop</em> does not display all the information that is often needed to resolve an issue.') . '</li>' .
                      '<li>' . t('<em>Krumo backtrace</em> displays nice debug information when any type of error is noticed, but only to users with the %perm permission.', array('%perm' => t('Access developer information'))) . '</li></ul>' .
                      t('Depending on the situation, the theme, the size of the call stack and the arguments, etc., some handlers may not display their messages, or display them on the subsequent page. Select <em>Standard Backdrop</em> <strong>and</strong> <em>Krumo backtrace</em> above the rendered page to maximize your chances of not missing any messages.') . '<br />' .
                      t('Demonstrate the current error handler(s):') . ' ' . l('notice+warning', $_GET['q'], array('query' => array('demo' => 'warning'))) . ', ' . l('notice+warning+error', $_GET['q'], array('query' => array('demo' => 'error'))) . ' ' . t('(The presentation of the @error is determined by PHP.)', array('@error' => 'error')),
  );
  $form['devel_error_handlers']['#size'] = count($form['devel_error_handlers']['#options']);
  if (empty($_POST) && !empty($_GET['demo'])) {
    devel_demonstrate_error_handlers($_GET['demo']);
  }

  $form['rebuild_theme_registry'] = array(
   '#type' => 'checkbox',
   '#title' => t('Rebuild the theme registry on every page load'),
   '#description' => t('While creating new templates and theme_ overrides the theme registry needs to be rebuilt.'),
   '#default_value' => $config->get('rebuild_theme_registry'),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  return $form;
}

/**
 * Submit handler for devel_admin_settings().
 */
function devel_admin_settings_submit($form, &$form_state) {
  $config = config('devel.settings');
  $values = $form_state['values'];

  $config->set('query_display', (bool) $values['query_display']);
  $config->set('query_sort', $values['query_sort']);
  $config->set('query_slow_limit', (int) $values['query_slow_limit']);

  $config->set('xhprof_enabled', (bool) $values['xhprof_enabled']);
  $config->set('xhprof_directory', $values['xhprof_directory']);
  $config->set('xhprof_url', $values['xhprof_url']);

  $config->set('api_url', $values['api_url']);

  $config->set('timer_display', (bool) $values['timer_display']);
  $config->set('memory_display', (bool) $values['memory_display']);
  $config->set('redirect_page', (bool) $values['redirect_page']);
  $config->set('permission_raw_names', (bool) $values['permission_raw_names']);
  $config->set('devel_error_handlers', array_values(array_filter($values['devel_error_handlers'])));
  $config->set('rebuild_theme_registry', (bool) $values['rebuild_theme_registry']);

  $config->save();
}

function devel_demonstrate_error_handlers($severity) {
  switch ($severity) {
    case 'warning':
      $undefined = $undefined;
      1/0;
      break;
    case 'error':
      $undefined = $undefined;
      1/0;
      devel_undefined_function();
      break;
  }
}
