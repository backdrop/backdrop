<?php
/**
 * @file
 * User page callbacks for the Filter module.
 */

/**
 * Page callback: Displays a page with long filter tips.
 *
 * @return string
 *   An HTML-formatted string.
 *
 * @see filter_menu()
 * @see theme_filter_tips()
 */
function filter_tips_long() {
  $format_id = arg(2);
  if ($format_id) {
    $output = theme('filter_tips', array('tips' => _filter_tips($format_id, TRUE), 'long' => TRUE));
  }
  else {
    $output = theme('filter_tips', array('tips' => _filter_tips(-1, TRUE), 'long' => TRUE));
  }
  return $output;
}

/**
 * Form callback: Display a form for inserting/editing a link.
 */
function filter_format_editor_link_form($form, &$form_state, $format) {
  $form_state['format'] = $format;

  // Pull in any default values set by the editor.
  $values = array();
  if (isset($form_state['input']['editor_object'])) {
    $values = $form_state['input']['editor_object'];
  }

  // Set the dialog title.
  if (!empty($values['href'])) {
    backdrop_set_title(t('Edit link'));
  }
  else {
    backdrop_set_title(t('Insert link'));
  }

  // Use a "textfield" rather than "url" to allow relative paths.
  $form['href'] = array(
    '#title' => t('URL'),
    '#type' => 'textfield',
    '#element_validate' => array('_filter_format_editor_link_url_validate'),
    '#default_value' => isset($values['href']) ? $values['href'] : FALSE,
    '#parents' => array('attributes', 'href'),
  );
  $form['target'] = array(
    '#title' => t('Open in new window'),
    '#type' => 'checkbox',
    '#return_value' => '_blank',
    '#default_value' => isset($values['target']) ? $values['target'] : FALSE,
    '#parents' => array('attributes', 'target'),
  );
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#ajax' => array(
      'callback' => 'filter_format_editor_dialog_save',
      'event' => 'click',
    ),
  );

  return $form;
}

/**
 * Form callback: Display a form for inserting/editing a link.
 */
function filter_format_editor_image_form($form, &$form_state, $format) {
  $form_state['format'] = $format;

  // Pull in any default values set by the editor.
  $values = array();
  if (isset($form_state['input']['editor_object'])) {
    $values = $form_state['input']['editor_object'];
  }

  // Set the dialog title.
  if (!empty($values['src'])) {
    backdrop_set_title(t('Edit image'));
  }
  else {
    backdrop_set_title(t('Insert image'));
  }

  $form['src'] = array(
    '#title' => t('URL'),
    '#type' => 'url',
    '#default_value' => isset($values['src']) ? $values['src'] : NULL,
    '#parents' => array('attributes', 'src'),
  );
  $form['alt'] = array(
    '#title' => t('Alternative text'),
    '#type' => 'textfield',
    '#default_value' => isset($values['alt']) ? $values['alt'] : NULL,
    '#parents' => array('attributes', 'alt'),
  );
  $form['size'] = array(
    '#title' => t('Image size'),
    '#wrapper_attributes' => array('class' => array('editor-image-size')),
    '#theme_wrappers' => array('form_element'),
  );
  $form['size']['width'] = array(
    '#title' => t('Width'),
    '#title_display' => 'attribute',
    '#type' => 'number',
    '#default_value' => isset($values['width']) ? $values['width'] : NULL,
    '#min' => 1,
    '#max' => 99999,
    '#attributes' => array('placeholder' => t('width')),
    '#parents' => array('attributes', 'width'),
    '#field_suffix' => ' &times; ',
  );
  $form['size']['height'] = array(
    '#title' => t('Height'),
    '#title_display' => 'attribute',
    '#type' => 'number',
    '#default_value' => isset($values['height']) ? $values['height'] : NULL,
    '#min' => 1,
    '#max' => 99999,
    '#attributes' => array('placeholder' => t('height')),
    '#parents' => array('attributes', 'height'),
    '#field_suffix' => ' ' . t('pixels')
  );
  $form['align'] = array(
    '#title' => t('Align'),
    '#type' => 'radios',
    '#default_value' => isset($values['data-align']) ? $values['data-align'] : 'none',
    '#options' => array(
      'none' => t('None'),
      'left' => t('Left'),
      'center' => t('Center'),
      'right' => t('Right'),
    ),
    '#wrapper_attributes' => array('class' => array('editor-image-align')),
    '#parents' => array('attributes', 'data-align'),
    '#access' => !empty($format->filters['filter_image_align']->status),
  );
  $form['caption'] = array(
    '#title' => t('Add a caption'),
    '#type' => 'checkbox',
    '#default_value' => (isset($values['data-has-caption']) && strcmp($values['data-has-caption'], 'false') !== 0) ? (bool) $values['data-has-caption'] : FALSE,
    '#parents' => array('attributes', 'data-has-caption'),
    '#access' => !empty($format->filters['filter_image_caption']->status),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#ajax' => array(
      'callback' => 'filter_format_editor_dialog_save',
      'event' => 'click',
    ),
  );

  return $form;
}

/**
 * Form AJAX callback. Sends the save editor AJAX command and closes the dialog.
 *
 * @see filter_format_editor_link_form()
 * @see filter_format_editor_image_form()
 */
function filter_format_editor_dialog_save($form, &$form_state) {
  $commands = array();
  $errors = form_get_errors();
  if (!empty($errors)) {
    $error_messages = theme('status_messages');
    $rendered_form = backdrop_render($form);
    $commands[] = ajax_command_remove('.editor-dialog .messages');
    $commands[] = ajax_command_replace('.editor-dialog form', $rendered_form);
    $commands[] = ajax_command_prepend('.editor-dialog .ui-dialog-content', $error_messages);
  }
  else {
    $commands[] = array(
      'command' => 'editorDialogSave',
      'values' => $form_state['values'],
    );
    $commands[] = ajax_command_close_modal_dialog();
  }
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Element validation function.
 */
function _filter_format_editor_link_url_validate(&$element, &$form_state) {
  $value = trim($element['#value']);
  form_set_value($element, $value, $form_state);

  // Unlike
  if ($value !== '' && !valid_url($value, TRUE) && !valid_url($value, FALSE)) {
    form_error($element, t('The URL %url is not valid.', array('%url' => $value)));
  }
}