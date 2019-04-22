<?php
/**
 * @file
 * Administrative functions for Layout module.
 *
 * This provides the UI to list, create, edit and delete layouts.
 */


/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_add_flexible_template() {
  // $layout = layout_load('default');
  $config = array(
    'is_new' => TRUE,
  );
  $layout = new Layout($config);
  $flexible_template_defaults = layout_flexible_template_defaults();

  dpm($layout);
  // Don't use layout_create_renderer() because we need to pass in template
  // info.
  $renderer = new LayoutRendererFlexible($layout, $flexible_template_defaults);
  $form['#attached'] = array(
    'library' => array(
      array('system', 'backdrop.ajax'),
    ),
  );

  if (!isset($form_state['renderer'])) {
    $form_state['renderer'] = $renderer;
  }
  $form['content']['display'] = array(
    '#markup' => $renderer->render(),
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save layout'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#validate' => array(),
  );

  return $form;
}

/**
 * Render the settings form for layout.
 *
 * @ingroup forms
 */
function layout_flexible_template_defaults() {
  $defaults = array(
    'type' => 'layout',
    'name' => 'flex_default',
    'version' => '1.13.x-dev',
    'backdrop' => '1.x',
    'regions' => array(
      'header' => 'Header',
      'top' => 'Top',
      'content' => 'Content',
      'bottom' => 'Bottom',
      'footer' => 'Footer',
    ),
    'default region' => 'content',
    'preview' => 'flex_default.png',
    'libraries' => array(
      'bootstrap4-gs',
    ),
    'path' => 'core/layouts/flex_default',
    'title' => 'Flex default',
    'hidden' => FALSE,
    'stylesheets' => array(
      'all' => array(
        'flex_default.css',
      ),
    ),
    'template' => 'layout-flexible',
  );
  return $defaults;
}
  


/**
 * Provides the default template for new layouts.
 */
function layout_flexible_template_template() {
  // set up a default
  $settings = array(
    'regions' => array(
      'header' => array(
        'name' => t('Header'),
        'contains' => 'two_column',
      ),
      'top' => array(
        'name' => t('Header'),
        'contains' => 'three_column',
      ),
      'content' => array(
        'name' => t('Header'),
        'contains' => 'column',
      ),
      'bottom' => array(
        'name' => t('Header'),
        'contains' => 'column',
      ),
      'footer' => array(
        'name' => t('Header'),
        'contains' => 'column',
      ),
    ),
  );

  return $settings;
}

/**
 * Provides the default template for new layouts.
 */
function theme_layout_flexible($variables) {
  $content = $variables['content'];
  $data = layout_flexible_template_template();
  $output = '<div class="layout--boxtonian">';
  foreach ($data['regions'] as $key => $region) {
    $output .= '<div class="l-header-inner container container-fluid">' . $content[$key] . '</div>';
  }
  $output .= '</div>';
  return $output;
}

