<?php
/**
 * @file
 * Admin page callbacks for the CKEditor 5 module.
 */

/**
 * Editor settings callback; Provide options for CKEditor 5 module.
 */
function ckeditor5_settings_form(&$form, $form_state, $format) {
  $plugins = ckeditor5_plugins();
  $toolbar_element = theme('ckeditor5_settings_toolbar', array(
    'format' => $format,
    'plugins' => $plugins,
  ));
  $elements = array(
    '#attached' => array(
      'library' => array(
        array('ckeditor5', 'backdrop.ckeditor5.admin'),
      ),
      'js' => array(
        array(
          'data' => array(
            'ckeditor5' => array(
              'toolbarAdmin' => $toolbar_element,
            ),
          ),
          'type' => 'setting',
        ),
      ),
    ),
    '#attributes' => array('class' => array('ckeditor5-toolbar-configuration')),
  );
  $elements['toolbar'] = array(
    '#type' => 'textarea',
    '#title' => t('Toolbar configuration'),
    '#default_value' => json_encode($format->editor_settings['toolbar']),
    '#attributes' => array('class' => array('ckeditor5-toolbar-textarea')),
  );

  $elements['plugins'] = array(
    '#type' => 'vertical_tabs',
  );

  // @todo: Have this settings form be provided via a plugin hook.
  $elements['plugins']['image'] = filter_editor_image_upload_settings_form($format);
  $elements['plugins']['image'] += array(
    '#type' => 'fieldset',
    '#title' => t('Image uploading'),
    '#attributes' => array(
      'class' => array('ckeditor5-plugin-backdropimage'),
      'data-ckeditor5-feature-dependency' => 'backdropImage',
    ),
    '#parents' => array('editor_settings', 'image_upload'),
  );

  $elements['plugins']['file'] = filter_editor_file_upload_settings_form($format);
  $elements['plugins']['file'] += array(
    '#type' => 'fieldset',
    '#title' => t('File uploading'),
    '#attributes' => array(
      'class' => array('ckeditor5-plugin-link'),
      'data-ckeditor5-feature-dependency' => 'backdropLink',
    ),
    '#parents' => array('editor_settings', 'file_upload'),
  );

  $elements['plugins']['style'] = array(
    '#type' => 'fieldset',
    '#title' => t('Style list'),
    '#attributes' => array(
      'class' => array('ckeditor5-plugin-style'),
      'data-ckeditor5-feature-dependency' => 'style',
    ),
  );
  $style_list = '';
  if (isset($format->editor_settings['style_list'])) {
    $style_list = _ckeditor5_settings_stringify_style_list($format->editor_settings['style_list']);
  }
  $elements['plugins']['style']['style_list'] = array(
    '#type' => 'textarea',
    '#rows' => 4,
    '#default_value' => $style_list,
    '#description' => t('A list of classes that will be provided in the "Styles" dropdown. Enter one class on each line in the format: element.class|Label. Example: h1.title|Title.') . '<br />' . t('Each style should be in your theme\'s main CSS as well as in your theme\'s ckeditor5-styles.css file.'),
    '#parents' => array('editor_settings', 'style_list'),
  );

  $elements['plugins']['heading'] = array(
    '#type' => 'fieldset',
    '#title' => t('Headings'),
    '#attributes' => array(
      'class' => array('ckeditor5-plugin-heading'),
      'data-ckeditor5-feature-dependency' => 'heading',
    ),
  );
  $elements['plugins']['heading']['heading_list'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled headings'),
    '#default_value' => $format->editor_settings['heading_list'],
    '#options' => array(
      'h1' => t('Heading 1'),
      'h2' => t('Heading 2'),
      'h3' => t('Heading 3'),
      'h4' => t('Heading 4'),
      'h5' => t('Heading 5'),
      'h6' => t('Heading 6'),
    ),
    '#description' => t('Select heading tags that are shown in the Headings toolbar button. Usually Heading 1 is reserved for the page title and is not shown in the editor.'),
    '#attributes' => array('class' => array('ckeditor5-heading-list')),
    '#parents' => array('editor_settings', 'heading_list'),
  );

  array_unshift($form['#validate'], 'ckeditor5_settings_form_validate');
  array_unshift($form['#submit'], 'ckeditor5_settings_form_submit');
  return $elements;
}

/**
 * Additional validate handler for filter_admin_format_form().
 */
function ckeditor5_settings_form_validate($form, &$form_state) {
  // Get rid of submission values caused by vertical tabs.
  unset($form_state['values']['editor_settings']['plugins']);
  $settings = $form_state['values']['editor_settings'];
  $toolbar_string = $settings['toolbar'];
  $result = json_decode($toolbar_string, TRUE);
  if (empty($result)) {
    form_error($form['editor_settings']['toolbar'], t('The CKEditor toolbar configuration could not be saved.'));
  }

  $styles = _ckeditor5_settings_parse_style_list($settings['style_list']);
  foreach ($styles as $style) {
    if (empty($style['name']) || empty($style['element']) || empty($style['classes'])) {
      form_error($form['editor_settings']['plugins']['style']['style_list'], t('The CKEditor list of styles is not formatted properly. Be sure to include one style per line, with the format "element.class|Label".'));
    }
  }
}

/**
 * Additional submit handler for filter_admin_format_form().
 */
function ckeditor5_settings_form_submit($form, &$form_state) {
  $settings = $form_state['values']['editor_settings'];

  // Convert JSON toolbar settings to an array.
  form_set_value($form['editor_settings']['toolbar'], json_decode($settings['toolbar'], TRUE), $form_state);

  // Convert string style list into an array.
  $styles = _ckeditor5_settings_parse_style_list($settings['style_list']);
  form_set_value($form['editor_settings']['plugins']['style']['style_list'], $styles, $form_state);

  // Convert heading list to simple unindexed array.
  $heading_list = array_filter($settings['heading_list']);
  form_set_value($form['editor_settings']['plugins']['heading']['heading_list'], $heading_list, $form_state);
}

/**
 * Parse a string of styles in the format of element.class|Label into an array.
 *
 * @param string $style_list_string
 *   A list of styles separated by new line characters.
 *
 * @return array
 *   An unindexed array of styles with the following keys:
 *   - name: The label of the style.
 *   - element: The type of element this still will use.
 *   - classes: An array of classes to be applied to the element.
 */
function _ckeditor5_settings_parse_style_list($style_list_string) {
  $styles = array();
  foreach (explode("\n", $style_list_string) as $style) {
    $style = trim($style);
    if ($style) {
      @list($element, $label) = explode('|', $style, 2);
      @list($element, $classes) = explode('.', $element, 2);
      $styles[] = array(
        'name' => $label,
        'element' => $element,
        // Note: CKEditor 4 used 'attributes' => array('class' => $class).
        // CKEditor 5 simplified this down to just a list of classes.
        // We are casting $classes to a string in order to avoid deprecation
        // warnings for the second parameter of explode() in PHP >= 8.1.
        'classes' => explode(' ', (string) $classes),
      );
    }
  }
  return $styles;
}

/**
 * Convert the stored array of styles into a string for editing.
 *
 * @param array $style_list
 *   The style list as returned by _ckeditor5_settings_parse_style_list().
 *
 * @return string
 *   The string representation of the style list.
 */
function _ckeditor5_settings_stringify_style_list(array $style_list = NULL) {
  $string = '';
  if ($style_list) {
    foreach ($style_list as $style) {
      $string .= $style['element'] . '.' . implode(' ', $style['classes']) . '|' . $style['name'] . "\n";
    }
  }
  return $string;
}
