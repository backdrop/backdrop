<?php

/**
 * @file
 * Definition of views_handler_area_title.
 */

/**
 * Views area handler to override the view's title.
 *
 * @ingroup views_area_handlers
 */
class views_handler_area_title extends views_handler_area {

  function option_definition() {
    $options = parent::option_definition();

    $options['content'] = array(
      'default' => 'Welcome to @site_name',
      'translatable' => TRUE,
    );

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $variables = array(
      'items' => array(
        '@site_name -- The name of the site.',
        '@site_slogan -- The slogan of the site.',
        '@site_mail -- The administrative email address for the site.',
      ),
    );
    $list = theme('item_list', $variables);
    $form['content'] = array(
      '#title' => t('Display'),
      '#type' => 'textfield',
      '#default_value' => $this->options['content'],
      '#description' => t('The following global tokens are supported:') . $list,
    );
  }

  /**
   * Find out the information to render.
   */
  function render($empty = FALSE) {
    // Must have options and does not work on summaries.
    if (!isset($this->options['content']) || $this->view->plugin_name == 'default_summary') {
      return;
    }

    $title = $this->options['content'];

    // Get the search information.
    $items = array('site_name', 'site_slogan', 'site_mail');
	$site_name = config_get('system.core', 'site_name');
	$site_slogan = config_get('system.core', 'site_slogan');
	$site_mail = config_get('system.core', 'site_mail');

    $replacements = array();
    foreach ($items as $item) {
      $replacements["@$item"] = ${$item};
    }

	$this->view->build_info['title'] = filter_xss_admin(str_replace(array_keys($replacements), array_values($replacements), $title));
    return;
  }
}
