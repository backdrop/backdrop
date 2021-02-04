<?php
/**
 * @file
 * Theme functions for the Locale module.
 */

/**
 * Returns HTML for the language negotiation configuration form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_language_negotiation_configure_form($variables) {
  $form = $variables['form'];
  $output = '';

  foreach ($form['#language_types'] as $type) {
    $rows = array();
    $info = $form['#language_types_info'][$type];
    $title = '<label>' . $form[$type]['#title'] . '</label>';
    $description = '<div class="description">' . $form[$type]['#description'] . '</div>';

    foreach (element_children($form[$type]) as $id) {
      if (array_key_exists('title', $form[$type][$id])) {
        $row = array(
          'data' => array(
            '<strong>' . backdrop_render($form[$type][$id]['title']) . '</strong>',
            backdrop_render($form[$type][$id]['description']),
            backdrop_render($form[$type][$id]['enabled']),
            backdrop_render($form[$type][$id]['weight']),
          ),
          'class' => array('draggable'),
        );
        if ($form[$type]['#show_operations']) {
          $row['data'][] = backdrop_render($form[$type][$id]['operation']);
        }
        $rows[] = $row;
      }
    }

    $header = array(
      array('data' => t('Detection method')),
      array('data' => t('Description')),
      array('data' => t('Enabled')),
      array('data' => t('Weight')),
    );

    // If there is at least one operation enabled show the operation column.
    if ($form[$type]['#show_operations']) {
      $header[] = array('data' => t('Operations'));
    }

    $variables = array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => "language-negotiation-providers-$type"),
    );
    $table  = theme('table', $variables);
    $table .= backdrop_render_children($form[$type]);

    backdrop_add_tabledrag("language-negotiation-providers-$type", 'order', 'sibling', "language-provider-weight-$type");

    $output .= '<div class="form-item">' . $title . $description . $table . '</div>';
  }

  $output .= backdrop_render_children($form);
  return $output;
}

/**
 * Returns themed HTML for the locale_date_format_form() form.
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @ingroup themeable
 */
function theme_locale_date_format_form($variables) {
  $form = $variables['form'];
  $header = array(
    'machine_name' => t('Language'),
    'pattern' => t('Format'),
  );

  foreach (element_children($form['locales']) as $key) {
    $row = array();
    $row[] = $form['locales'][$key]['#title'];
    unset($form['locales'][$key]['#title']);
    $row[] = array('data' => backdrop_render($form['locales'][$key]));
    $rows[] = $row;
  }

  $output = backdrop_render($form['help']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= backdrop_render_children($form);

  return $output;
}

/**
 * Returns themed HTML for the language switcher block.
 *
 * @param $variables
 *   An associative array containing:
 *   - links: (object) The output of language_negotiation_get_switch_links():
 *     - links: An associativearray of language links to be rendered.
 *     - provider: ???
 *   - settings: An associative array with the display settings for the language
 *     switcher block:
 *     - language_list_display: either "vertical" or "inline".
 *     - language_link_label_display: either "name" or "code".
 *     - custom_delimiter: either completely omitted (falls back to "|"), or a
 *     single character, to be used as a delimiter when the language links are
 *     to be rendered inline. This is added to the list items as a
 *     data-list-delimiter attribute, and can be used in CSS via a 'content'
 *     property. For example:
 *     @code
 *     .language-switcher.links.inline li:after {
 *       content: attr(data-list-delimiter);
 *       padding: 0 0.6em;
 *       color: gray;
 *     }
 *     @endcode
 *
 * @ingroup themeable
 */
function theme_locale_block($variables) {
  backdrop_add_css(backdrop_get_path('module', 'locale') . '/css/locale.css');
  $settings = $variables['settings'];
  $links = $variables['links'];
  $links_variables = array();

  if ($settings['language_link_label_display'] == 'code') {
    foreach ($links->links as $code => $link) {
      $links->links[$code]['title'] = $code;
    }
  }
  $links_variables['links'] = $links->links;

  $classes = array('language-switcher', "language-switcher-{$links->provider}");
  if ($settings['language_list_display'] == 'inline') {
    $classes[] = 'links';
    $classes[] = 'inline';

    $links_variables['li_attributes']['data-list-delimiter'] = isset($settings['custom_delimiter']) ? $settings['custom_delimiter'] : '|';
  }
  $links_variables['attributes']['class'] = $classes;

  return theme('links', $links_variables);
}
