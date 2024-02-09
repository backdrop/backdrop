<?php
/**
 * @file
 * Theme functions for CKEditor module.
 */

/**
 * Preprocess variables for theme_ckeditor5_settings_toolbar().
 */
function template_preprocess_ckeditor5_settings_toolbar(&$variables) {
  // Simplify the language direction information for toolbar buttons.
  global $language;
  $variables['language_direction'] = isset($language->direction) && $language->direction === LANGUAGE_RTL ? 'rtl' : 'ltr';

  // Create lists of active and disabled buttons.
  $format = $variables['format'];
  $plugins = $variables['plugins'];
  $buttons = array();
  $multiple_buttons = array();
  foreach ($plugins as $plugin_info) {
    if (isset($plugin_info['buttons'])) {
      foreach ($plugin_info['buttons'] as $button_name => $button) {
        $button['name'] = $button_name;
        if (!empty($button['multiple'])) {
          $multiple_buttons[$button_name] = $button;
        }
        $buttons[$button_name] = $button;
      }
    }
  }

  $active_buttons = array();
  $settings = $format->editor_settings;
  foreach ($settings['toolbar'] as $button_name) {
    if (isset($buttons[$button_name])) {
      $active_buttons[] = $buttons[$button_name];
      if (empty($buttons[$button_name]['multiple'])) {
        unset($buttons[$button_name]);
      }
    }
  }
  $disabled_buttons = array_diff_key($buttons, $multiple_buttons);

  $rtl = $variables['language_direction'] === 'rtl' ? '_rtl' : '';

  $build_button_item = function($button, $rtl) {
    // Value of the button item.
    if (isset($button['image_alternative' . $rtl])) {
      $value = filter_xss_admin($button['image_alternative' . $rtl]);
    }
    elseif (isset($button['image_alternative'])) {
      $value = filter_xss_admin($button['image_alternative']);
    }
    elseif (isset($button['image']) || isset($button['image' . $rtl])) {
      if (isset($button['image' . $rtl])) {
        $src = file_create_url($button['image' . $rtl]);
      }
      else {
        $src = file_create_url($button['image']);
      }
      $value = '<img src="'. $src . '" title="' . check_plain($button['label']) . '" />';
      $value = '<span title="' . $button['label'] . '" aria-label="' . $button['label'] . '">' . $value . '</span>';
    }
    else {
      $value = '?';
    }

    // Build the button attributes.
    $attributes = array(
      'data-button-name' => $button['name'],
      'class' => array('ckeditor5-button'),
      // Setting a tabindex makes the item receive keyboard focus.
      'tabindex' => '0',
    );
    if (!empty($button['multiple'])) {
      $attributes['class'][] = 'ckeditor5-multiple-button';
    }
    if (!empty($button['attributes'])) {
      $attributes = backdrop_array_merge_deep($attributes, $button['attributes']);
    }
    if (!empty($button['required_html'])) {
      $attributes['data-required-html'] = backdrop_json_encode($button['required_html']);
    }
    if (!empty($button['optional_html'])) {
      $attributes['data-optional-html'] = backdrop_json_encode($button['optional_html']);
    }

    // Build the button item.
    $button_item = array(
      'contents' => $value,
      'attributes' => $attributes,
    );

    // Set additional flag on the button if it can occur multiple times.
    if (!empty($button['multiple'])) {
      $button_item['multiple'] = TRUE;
    }

    return $button_item;
  };

  // Assemble list of active buttons.
  $variables['active_buttons'] = array();
  foreach ($active_buttons as $button) {
    $variables['active_buttons'][] = $build_button_item($button, $rtl);
  }

  // Assemble list of disabled buttons.
  $variables['disabled_buttons'] = array();
  foreach ($disabled_buttons as $button) {
    $variables['disabled_buttons'][] = $build_button_item($button, $rtl);
  }
  // Assemble list of multiple buttons that may be added multiple times.
  $variables['multiple_buttons'] = array();
  foreach ($multiple_buttons as $button_name => $button) {
    if ($button_name != '-') {
      $variables['multiple_buttons'][] = $build_button_item($button, $rtl);
    }
  }
}

/**
 * Displays the toolbar configuration for CKEditor.
 */
function theme_ckeditor5_settings_toolbar($variables) {
  // Controls for adding or removing a row.
  $row_controls = '<span class="ckeditor5-row-controls">';
  $row_controls .= '<a href="#" class="ckeditor5-row-remove" title="' . t('Remove row') . '">-</a>';
  $row_controls .= '<a href="#" class="ckeditor5-row-add" title="' . t('Add row') . '">+</a>';
  $row_controls .= '</span>';

  // theme_item_list() is avoided here because it cannot output an empty <ul>
  // tag, and the wrapping <div class="item-list"> can cause styling issues.
  $button_row = array();
  $button_rows = array();

  // Add a line break at the end of the list.
  $variables['active_buttons'][] = array(
    'attributes' => array('data-button-name' => '-'),
  );
  foreach ($variables['active_buttons'] as $button) {
    // CKEditor 5 uses a dash to indicate a line break in the toolbar.
    if ($button['attributes']['data-button-name'] === '-') {
      $rendered_row = '<ul class="ckeditor5-buttons">';
      $rendered_row .= implode('', $button_row);
      $rendered_row .= '</ul>';
      $rendered_row .= $row_controls;

      // Start a new row and go to the next item.
      $button_rows[] = $rendered_row;
      $button_row = array();
    }
    else {
      $button_row[] = '<li' . backdrop_attributes($button['attributes']) . '>' . $button['contents'] . '</li>';
    }
  }

  $output = '';

  // Active toolbar buttons.
  $output .= '<label id="ckeditor5-toolbar-active-label">' . t('Active toolbar') . '</label>';
  $output .= '<div class="ckeditor5-toolbar-active clearfix" aria-labelledby="ckeditor5-toolbar-active-label">';
  $output .= '<ul class="ckeditor5-active-toolbar-configuration">';
  foreach ($button_rows as $button_row) {
    $output .= '<li class="ckeditor5-row">' . $button_row . '</li>';
  }
  $output .= '</ul>';
  $output .= '</div>';

  // Disabled/available buttons.
  $output .= '<label id="ckeditor5-toolbar-disabled-label">' . t('Available buttons') . '</label>';
  $output .= '<div class="ckeditor5-toolbar-disabled clearfix" aria-labelledby="ckeditor5-toolbar-disabled-label">';
  $output .= '<ul class="ckeditor5-buttons">';
  foreach ($variables['disabled_buttons'] as $button) {
    $output .= '<li' . backdrop_attributes($button['attributes']) . '>' . $button['contents'] . '</li>';
  }
  $output .= '</ul>';

  // Buttons that may be placed multiple times.
  $output .= '<ul class="ckeditor5-multiple-buttons">';
  foreach ($variables['multiple_buttons'] as $button) {
    $output .= '<li' . backdrop_attributes($button['attributes']) . '>' . $button['contents'] . '</li>';
  }
  $output .= '</ul>';
  $output .= '</div>';

  // Wrap the whole thing in a fieldset.
  $fieldset = array(
    '#type' => 'fieldset',
    '#children' => $output,
    '#title' => t('CKEditor Toolbar'),
  );

  return backdrop_render($fieldset);
}
