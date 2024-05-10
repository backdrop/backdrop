<?php
/**
 * @file
 * CKEditor 4 to CKEditor 5 upgrade code.
 */

/**
 * Form callback for upgrading a text format from CKEditor 4 to CKEditor 5.
 */
function ckeditor5_upgrade_form($form, &$form_state, $format) {
  backdrop_set_title(t('Upgrade %format to CKEditor 5', array('%format' => $format->name)), PASS_THROUGH);
  form_load_include($form_state, 'inc', 'ckeditor5', 'ckeditor5.upgrade');
  $form_state['format'] = $format;

  $config_name = 'filter.format.' . $format->format;
  $form['help'] = array(
    '#type' => 'help',
    '#markup' => t('Upgrading from CKEditor 4 to CKEditor 5 is a one-way process. The text format configuration can be restored using a <a href="!full_export">full site configuration export</a>, or by restoring the individual text format configuration file (!example). Back up the configuration now before continuing.', array(
      '!full_export' => url('admin/config/development/configuration/full/export'),
      '!example' => '<code>' . l($config_name . '.json', 'admin/config/development/configuration/single/export', array(
        'query' => array('group' => 'Text formats', 'name' => $config_name))
      ) . '</code>')
    ),
  );

  // Check for upgrade issues before actually running the upgrade.
  $warnings = ckeditor5_upgrade_warnings($format);
  if ($warnings['error']) {
    backdrop_set_message($warnings['error'], 'error');
  }
  elseif (!empty($warnings['removed_buttons'])) {
    $warning_message = '<p>' . t('The following toolbar buttons will removed in the upgrade:') . '</p>';
    $warning_message .= theme('item_list', array(
      'items' => $warnings['removed_buttons'],
      'attributes' => array(
        'class' => array('ckeditor5-removed-buttons-list'))
      )
    );
    $warning_message .= '<p>' . t('Some buttons are no longer needed in CKEditor 5, while others might not be supported. See the <a href="!url">CKEditor 5 change record</a> for more information.', array('!url' => 'https://docs.backdropcms.org/change-records/ckeditor5-upgrade')) . '</p>';
    $form['warnings'] = array(
      '#type' => 'markup',
      '#markup' => $warning_message,
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['upgrade'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm upgrade'),
    '#attributes' => array('class' => array('button-danger')),
    '#access' => empty($warnings['error']),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#href' => 'admin/config/content/formats/' . $format->format,
    '#title' => t('Cancel'),
  );
  return $form;
}

/**
 * Submit handler for ckeditor5_upgrade_form().
 */
function ckeditor5_upgrade_form_submit($form, &$form_state) {
  $format = $form_state['format'];
  $updated_format = ckeditor5_upgrade_format($format);
  filter_format_save($updated_format);
  backdrop_set_message(t('The %label text format has been upgraded from CKEditor 4 to CKEditor 5. Verify the CKEditor 5 toolbar and settings.', array('%label' => $format->name)));
  $form_state['redirect'] = 'admin/config/content/formats/' . $format->format;
}

/**
 * Upgrade a text format configuration from CKEditor 4 to CKEditor 5.
 *
 * @param stdClass $format
 *   The text format to be upgraded.
 * @return stdClass
 *   The modified text format, ready to be saved with filter_format_save().
 *
 * @see filter_format_save()
 */
function ckeditor5_upgrade_format($format) {
  $warnings = array();
  return _ckeditor5_upgrade_format($format, $warnings);
}

/**
 * Return any warnings that may occur during a CKEditor 4 to 5 upgrade.
 *
 * @param stdClass $format
 *   The text format to be upgraded.
 *
 * @return array
 *   Any warnings that will occur during the upgrade.
 */
function ckeditor5_upgrade_warnings($format) {
  $warnings = array();
  _ckeditor5_upgrade_format($format, $warnings);
  return $warnings;
}

/**
 * Internal use only, performs the actual upgrade.
 *
 * @param stdClass $original_format
 *   The text format to be upgraded.
 * @param array $warnings
 *   Any warnings that occur during the upgrade. Modified by reference.
 *
 * @return stdClass
 *   The modified text format, ready to be saved with filter_format_save().
 *
 * @see ckeditor5_upgrade_format()
 * @see ckeditor5_upgrade_warnings()
 */
function _ckeditor5_upgrade_format($original_format, &$warnings) {
  $format = clone($original_format);
  foreach ($original_format->filters as $filter_name => $filter) {
    $format->filters[$filter_name] = clone($filter);
  }

  $warnings = array(
    'error' => NULL,
    'removed_buttons' => array(),
  );

  if ($format->editor === 'ckeditor5') {
    $warnings['error'] = t('The text format %format is already upgraded to CKEditor 5.', array('%format' => $format->name));
    return FALSE;
  }
  elseif ($format->editor !== 'ckeditor') {
    $warnings['error'] = t('The text format %format is not using the CKEditor 4 text editor.', array('%format' => $format->name));
    return FALSE;
  }
  $format->editor = 'ckeditor5';

  // Get a list of button mappings between CKEditor 4 and 5 modules.
  $button_mapping = module_invoke_all('ckeditor5_upgrade_button_mapping');
  backdrop_alter('ckeditor5_upgrade_button_mapping', $button_mapping);

  // Convert the toolbar.
  $new_toolbar = array();
  $toolbar = $format->editor_settings['toolbar'];
  $row_count = count($toolbar);
  foreach ($toolbar as $row_index => $toolbar_row) {
    $group_count = count($toolbar_row);
    foreach ($toolbar_row as $group_index => $toolbar_group) {
      $new_group = array();
      foreach ($toolbar_group['items'] as $button_name) {
        if (isset($button_mapping[$button_name])) {
          $new_group[] = $button_mapping[$button_name];
        }
        else {
          $warnings['removed_buttons'][] = $button_name;
        }
      }
      // Add a separator between groups (but not at the end of the row).
      if ($group_index < $group_count - 1) {
        $new_group[] = '|';
      }
      $new_toolbar = array_merge($new_toolbar, $new_group);
    }
    // Add a line break between rows (except on the last row).
    if ($row_index < $row_count - 1) {
      $new_toolbar[] = '-';
    }
  }

  // Clean up any empty groups. This can happen when buttons have no equivalent
  // in CKEditor 5, resulting in empty groups and rows.
  $prev_button = '';
  foreach ($new_toolbar as $index => $button) {
    // Remove empty groups.
    if ($button === '|' && ($prev_button === '|' || $prev_button === '-')) {
      unset($new_toolbar[$index]);
    }
    // Remove empty rows.
    elseif ($button === '-' && $prev_button === '-') {
      unset($new_toolbar[$index]);
    }
    else {
      $prev_button = $button;
    }
  }
  $new_toolbar = array_values($new_toolbar);

  // Remove any trailing group separator.
  if ($new_toolbar[count($new_toolbar) - 1] === '|') {
    array_pop($new_toolbar);
  }

  $format->editor_settings['toolbar'] = $new_toolbar;

  // Remove the unused format_list configuration.
  if (isset($format->editor_settings['format_list'])) {
    unset($format->editor_settings['format_list']);
  }

  // Convert the styles list.
  if (!empty($format->editor_settings['plugins']['style']['style_list'])) {
    $style_list = $format->editor_settings['plugins']['style']['style_list'];
    foreach ($style_list as $style_item) {
      $format->editor_settings['style_list'][] = array(
        'name' => $style_item['name'],
        'element' => $style_item['element'],
        'classes' => explode('.', $style_item['attributes']['class']),
      );
    }
    unset($format->editor_settings['plugins']['style']);
  }

  // Adjust the settings based on allowed tags if filtered_html is enabled.
  if (!empty($format->filters['filter_html']->status)) {
    // Create the heading list based on allowed HTML tags.
    $tag_list = $format->filters['filter_html']->settings['allowed_html'];
    $tags = explode(' ', $tag_list);
    $new_tags = array();
    foreach ($tags as $tag) {
      $tag = str_replace(array('<', '>'), '', $tag);
      if (substr($tag, 0, 1) === 'h' && strlen($tag) === 2) {
        $new_tags[] = $tag;
      }
    }
    sort($new_tags);
    $format->editor_settings['heading_list'] = backdrop_map_assoc($new_tags);

    // Add <u> to the list of allowed tags if the underline button is present.
    if (in_array('underline', $new_toolbar) && !in_array('<u>', $new_tags)) {
      $format->filters['filter_html']->settings['allowed_html'] = $tag_list . ' <u>';
    }
  }
  else {
    $format->editor_settings['heading_list'] = array(
      'h1' => 'h1',
      'h2' => 'h2',
      'h3' => 'h3',
      'h4' => 'h4',
      'h5' => 'h5',
      'h6' => 'h6',
    );
  }

  // Allow other modules to further modify the upgraded text format.
  backdrop_alter('ckeditor5_upgrade_format', $format, $original_format);

  // Remove the now un-used "plugins" parent array.
  unset($format->editor_settings['plugins']);

  return $format;
}

/**
 * Implements hook_ckeditor5_upgrade_button_mapping().
 */
function ckeditor5_ckeditor5_upgrade_button_mapping() {
  return array(
    'Bold' => 'bold',
    'Italic' => 'italic',
    'Underline' => 'underline',
    'Strike' => 'strikethrough',
    'JustifyLeft' => 'alignment:left',
    'JustifyCenter' => 'alignment:center',
    'JustifyRight' => 'alignment:right',
    'JustifyBlock' => 'alignment:justify',
    'BulletedList' => 'bulletedList',
    'NumberedList' => 'numberedList',
    'Undo' => 'undo',
    'Redo' => 'redo',
    'Superscript' => 'superscript',
    'Subscript' => 'subscript',
    'Blockquote' => 'blockQuote',
    'Source' => 'sourceEditing',
    'HorizontalRule' => 'horizontalLine',
    'Cut' => NULL,
    'Copy' => NULL,
    'Paste' => NULL,
    'PasteText' => NULL,
    'PasteFromWord' => NULL,
    'ShowBlocks' => 'showBlocks',
    'RemoveFormat' => 'removeFormat',
    'SpecialChar' => 'specialCharacters',
    'Format' => 'heading',
    'Styles' => 'style',
    'Table' => 'insertTable',
    'Maximize' => 'maximize',
    'BackdropImage' => 'backdropImage',
    'BackdropLink' => 'backdropLink',
    'BackdropUnlink' => NULL,
    '-' => '|',
  );
}
