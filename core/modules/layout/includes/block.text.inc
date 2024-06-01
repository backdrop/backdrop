<?php

/**
 *  BlockText extends Block
 *
 *  This class allows us to create text blocks.
 */
class BlockText extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    $this->settings += array(
      'content' => '',
      'format' => NULL,
    );
  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Custom block');
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return isset($this->settings['title']) ? check_plain($this->settings['title']) : '';
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   */
  function getContent() {
    if (!empty($this->settings['content'])) {
      return check_markup(token_replace($this->settings['content']), $this->settings['format'], '', TRUE);
    }
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    // Hide the title display, as custom text blocks don't have "default" title
    // from which they would inherit, so it's either a custom title or no title.
    $form['title_display']['title_display']['#access'] = FALSE;

    $form['#attached']['js'][] = backdrop_get_path('module', 'layout') . '/js/layout.admin.js';

    $form['title_display']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Display title'),
      '#default_value' => $this->settings['title'],
      '#maxlength' => 255,
    );
    $form['content'] = array(
      '#type' => 'text_format',
      '#title' => t('Block content'),
      '#default_value' => $this->settings['content'],
      '#format' => $this->settings['format'],
      '#editor_uploads' => TRUE,
      '#rows' => 5,
    );

    if (module_exists('block')) {
      $form['convert'] = array(
        '#weight' => 2,
        '#parents' => array(),
      );
      $form['convert']['reusable'] = array(
        '#type' => 'checkbox',
        '#title' => t('Make this block reusable'),
        '#description' => t('If enabled, this block will be made reusable across different layouts and be listed in on the !block_link page.', array('!block_link' => l(t('Custom blocks'), 'admin/structure/block'))),
        '#weight' => 1,
      );
      // Add a notice for translation.
      if (module_exists('locale')) {
        $form['convert']['reusable']['#description'] .= ' ' . t('Reusable blocks may also be translated.');
      }
    }
    $form['admin_label'] = array(
      '#type' => 'fieldset',
      '#title' => t('Admin label'),
      '#tree' => FALSE,
      '#collapsed' => empty($this->settings['admin_label']),
      '#states' => array(
        'collapsed' => array(
          ':input[name="reusable"]' => array('checked' => FALSE),
        ),
      ),
      '#collapsible' => TRUE,
      '#weight' => 4,
    );
    $form['admin_label']['admin_label'] = array(
      '#type' => 'textfield',
      '#default_value' => isset($this->settings['admin_label']) ? $this->settings['admin_label'] : '',
      '#title' => t('Admin label'),
      '#description' => t('Used to identify the block on layout pages. Required for reusable blocks.'),
      '#maxlength' => 255,
    );
    $form['admin_label']['delta'] = array(
      '#type' => 'machine_name',
      '#title' => t('Internal name'),
      '#maxlength' => 64,
      '#machine_name' => array(
        'source' => array('admin_label', 'admin_label'),
        'exists' => 'block_custom_block_load',
      ),
      '#description' => t('A unique machine-readable name containing letters, numbers, and underscores.'),
      '#weight' => 3,
      '#required' => FALSE,
    );
    $form['admin_label']['admin_description'] = array(
      '#type' => 'textfield',
      '#title' => t('Admin description'),
      '#maxlength' => 128,
      '#default_value' => isset($this->settings['admin_description']) ? $this->settings['admin_description'] : '',
      '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.<br />Allowed HTML tags: @tags', array('@tags' => _filter_xss_display_allowed_tags())),
      '#weight' => 4,
    );
  }

  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    if (module_exists('block')) {
      $admin_label = trim($form_state['values']['admin_label']);
      $delta = trim($form_state['values']['delta']);
      if ($form_state['values']['reusable']) {
        if (empty($admin_label)) {
          form_error($form['admin_label']['admin_label'], t('Admin label is required when making a block reusable.'));
        }
        elseif (empty($delta)) {
          form_error($form['admin_label']['delta'], t('An internal name is required when making a block reusable.'));
        }
      }
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    if (module_exists('block') && $form_state['values']['reusable']) {
      $delta = $form_state['values']['delta'];
      $this->settings = array_diff_key($this->settings, array('title' => '', 'content' => '', 'format' => ''));
      $this->plugin = 'block:' . $delta;

      $edit = array(
        'info' => $form_state['values']['admin_label'],
        'title' => $form_state['values']['title'],
        'description' => $form_state['values']['admin_description'],
        'body' => $form_state['values']['content'],
      );
      block_custom_block_save($edit, $delta);
    }
    else {
      $this->settings['title'] = $form_state['values']['title'];
      $this->settings['content'] = $form_state['values']['content']['value'];
      $this->settings['format'] = $form_state['values']['content']['format'];
    }
  }
}
