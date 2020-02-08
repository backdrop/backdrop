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
        '#description' => t('Once a block is made reusable, all copies of it can configured from a single place, in the !link listing. <strong>A reusable block can never be converted back into a stand-alone block</strong>.', array('!link' => l(t('Custom blocks'), 'admin/structure/block'))),
        '#weight' => 1,
      );
      // Add a notice for translation.
      if (module_exists('locale')) {
        $form['convert']['reusable']['#description'] .= ' ' . t('Reusable blocks may also be translated.');
      }
      $form['convert']['label'] = array(
        '#type' => 'textfield',
        '#title' => t('Admin label'),
        '#maxlength' => 64,
        '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.'),
        '#attributes' => array('class' => array('reusable-block-admin-label')),
        '#weight' => 2,
        '#states' => array(
          'visible' => array(
            ':input[name="reusable"]' => array('checked' => TRUE),
          ),
          // Workaround in order to make the field render as required, with a
          // red asterisk. Actual form validation happens in formValidate()
          // below.
          // See https://github.com/backdrop/backdrop-issues/issues/2732
          'required' => array(
            'input[name="reusable"]' => array('checked' => TRUE),
          ),
        ),
      );
      $form['convert']['delta'] = array(
        '#type' => 'machine_name',
        '#title' => t('Machine name'),
        '#maxlength' => 64,
        '#machine_name' => array(
          'source' => array('convert', 'label'),
          'exists' => 'block_custom_block_load',
        ),
        '#description' => t('A unique machine-readable name, containing letters, numbers, and underscores.'),
        '#attributes' => array('class' => array('reusable-block-machine-name')),
        '#weight' => 3,
        '#states' => array(
          // Workaround in order to make the field render as required, with a
          // red asterisk (when one clicks the "edit" link). Actual form
          // validation happens in formValidate() below.
          // See https://github.com/backdrop/backdrop-issues/issues/2732
          'required' => array(
            'input[name="reusable"]' => array('checked' => TRUE),
          ),
        ),
        // This line is still required in order to not throw form validation
        // errors when the 'Make this block reusable' checkbox is not checked.
        // Also see https://github.com/backdrop/backdrop-issues/issues/2732
        '#required' => FALSE,
      );
      $form['convert']['description'] = array(
       '#type' => 'textfield',
       '#title' => t('Admin description'),
       '#maxlength' => 128,
       '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.<br />Allowed HTML tags: @tags', array('@tags' => _filter_xss_display_allowed_tags())),
       '#attributes' => array('class' => array('reusable-block-admin-description')),
       '#weight' => 4,
        '#states' => array(
          'visible' => array(
            ':input[name="reusable"]' => array('checked' => TRUE),
          ),
        ),
      );
    }
  }

  /**
   * Validates the form settings.
   */
  function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    if (module_exists('block')) {
      $reusable_admin_label = trim($form_state['values']['label']);
      $delta = trim($form_state['values']['delta']);
      if ($form_state['values']['reusable']) {
        if (empty($reusable_admin_label)) {
          form_error($form['convert']['label'], t('Admin label is required when making a block reusable.'));
        }
        elseif (empty($delta)) {
          form_error($form['convert']['delta'], t('A machine name is required when making a block reusable.'));
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
        'info' => $form_state['values']['label'],
        'title' => $form_state['values']['title'],
        'description' => $form_state['values']['description'],
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
