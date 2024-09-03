<?php
/**
 * @file
 * Definition of views_handler_field_file_icon.
 */

/**
 * @ingroup views_field_handlers
 */
class views_handler_field_file_icon extends views_handler_field {

  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['icon_size'] = array('default' => 32);
    $options['display_type'] = array('default' => 'svg');
    $options['add_alt_text'] = array('default' => TRUE);
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['icon_size'] = array(
      '#type' => 'number',
      '#title' => t('Icon size'),
      '#min' => 8,
      '#max' => 300,
      '#default_value' => $this->options['icon_size'],
      '#field_suffix' => 'px',
    );
    $form['display_type'] = array(
      '#type' => 'radios',
      '#title' => t('Display type'),
      '#options' => array(
        'svg' => t('Inline SVG markup'),
        'img' => t('Regular images'),
      ),
      '#default_value' => $this->options['display_type'],
    );
    $form['display_type']['svg']['#description'] = t('Inline SVG images are usually more efficient and easier to style.');
    $form['display_type']['img']['#description'] = t('Regular image tags can be more efficient when showing the same icon many times.');
    $form['add_alt_text'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add alternative text'),
      '#default_value' => $this->options['add_alt_text'],
      '#description' => t('If alternative text is omitted, this icon will be marked as decorative only.'),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $uri = $this->get_value($values);

    // If there is an extension.
    if ($extension = pathinfo($uri, PATHINFO_EXTENSION)) {
      $icon_name = $this->getIconName($uri);
      if ($this->options['display_type'] == 'img') {
        $icon = $this->renderImg($icon_name, $extension);
      }
      else {
        $icon = $this->renderSvg($icon_name, $extension);
      }
      return $icon;
    }
  }

  /**
   * Render the icon as svg tag.
   */
  protected function renderSvg($icon_name, $extension) {
    $options = array(
      'attributes' => array(
        'width' => $this->options['icon_size'],
        'height' => $this->options['icon_size'],
      ),
    );
    if ($this->options['add_alt_text']) {
      $options['alt'] = t('File icon for @extension extension', array(
        '@extension' => $extension,
      ));
    }
    return icon($icon_name, $options);
  }

  /**
   * Render the icon as img tag.
   */
  protected function renderImg($icon_name, $extension) {
    $attributes = array(
      'width' => $this->options['icon_size'],
      'height' => $this->options['icon_size'],
      'alt' => '',
      'src' => base_path() . icon_get_path($icon_name),
    );
    if ($this->options['add_alt_text']) {
      $attributes['alt'] = t('File icon for @extension extension', array(
        '@extension' => $extension,
      ));
    }
    else {
      // Add aria-hidden if this is a purely decorative situation.
      // See https://developer.mozilla.org/en-US/docs/Web/Accessibility/ARIA/Attributes/aria-hidden
      $attributes['aria-hidden'] = 'true';
    }
    return '<img ' . backdrop_attributes($attributes) . '>';
  }

  /**
   * Map file extensions to Phosphor icons.
   *
   * @param string $uri
   *   The original file path.
   *
   * @return string
   *   The name of an icon file, as expected by icon().
   */
  protected function getIconName($uri) {
    // If there's no 1:1 icon name match for the extension, map to one. It's
    // possible that modules or themes provide additional file icons, so this
    // only acts as a fallback for some common types.
    $mapping = array(
      'docx' => 'doc',
      'gz' => 'archive',
      'jpeg' => 'jpg',
      'odt' => 'text',
      'pptx' => 'ppt',
      'tar' => 'archive',
      'xlsx' => 'xls',
    );
    $extension = pathinfo($uri, PATHINFO_EXTENSION);
    if (icon_get_path('file-' . $extension)) {
      $icon_name = 'file-' . $extension;
    }
    elseif (array_key_exists($extension, $mapping)) {
      $icon_name = 'file-' . $mapping[$extension];
    }
    else {
      // If the extension has a more specific mimetype, use that type prefix.
      $mime_type = file_get_mimetype($uri);
      $mime_parts = explode('/', $mime_type);
      if (count($mime_parts) && in_array($mime_parts[0], array(
        'image',
        'video',
        'audio',
        'text',
      ))) {
        $icon_name = 'file-' . $mime_parts[0];
      };
    }

    // Fallback to the generic icon if there are no other matches.
    if (!isset($icon_name)) {
      $icon_name = 'file';
    }

    return $icon_name;
  }

}
