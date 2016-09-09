<?php

/**
 *  BlockHero extends BlockText
 *
 *  This class allows us to create hero blocks with image backgrounds.
 */
class BlockHero extends BlockText {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    $this->settings += array(
      'image' => '',
      'image_path' => '',
    );
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Hero block');
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $upload_validators = array(
      'file_validate_extensions' => array('jpg jpeg png gif'),
      'file_validate_image_resolution' => array('3200x1600', '1200x600'),
    );
    $upload_description = theme('file_upload_help', array(
      'description' => t('The uploaded image will be displayed as a background.'),
      'upload_validators' => $upload_validators,
    ));

    // Add an image field at the top of the settings form.
    $form['image'] = array(
      '#type' => 'managed_file',
      '#title' => t('Background image'),
      '#description' => $upload_description,
      '#default_value' => $this->settings['image'],
      '#upload_location' => 'public://hero/',
      '#upload_validators' => $upload_validators,
      '#weight' => -10,
    );
    // Store the image path separately.
    $form['image_path'] = array(
      '#type' => 'value',
      '#value' => $this->settings['image_path'],
    );
  }

  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    // Check that one of title, text, or image, is provided.
    if ($form_state['values']['image'] == 0 && empty($form_state['values']['title']) && empty($form_state['values']['content']['value'])) {
      form_error($form['image'], t('At least one of Background image, a Display title, or Block content must be provided.'));
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {

    // Save the things not handled by BlockText.
    if ($form_state['values']['image']) {
      // Save the file as permanent.
      $file = file_load($form_state['values']['image']);
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      // Record that the layout module is using this file.
      file_usage_add($file, 'layout', 'block', $file->fid);

      // Store the filepath separately for use as inline background image.
      $fileurl = file_create_url($file->uri);
      $filepath = str_replace($GLOBALS['base_url'], '', $fileurl);
      $form_state['values']['image_path'] = $filepath;

      $this->settings['image'] = $form_state['values']['image'];
      $this->settings['image_path'] = $form_state['values']['image_path'];
    }

    parent::formSubmit($form, $form_state);
  }
}
