<?php

/**
 *  BlockCopy extends Block
 *
 *  This class allows us to create blocks which are copies of other blocks.
 */
class BlockCopy extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

  }

  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    $parent_block = $this->getParentBlock();
    $parent_block_info = $this->getParentBlockInfo($parent_block);
    return t('Copy of %block', array('%block' => $parent_block_info['info']));
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    $parent_block = $this->getParentBlock();
    return isset($parent_block->settings['title']) ? check_plain($this->parent_block['title']) : '';
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   */
  function getContent() {
    if (!empty($this->settings['source_block'])) {
      $parent_block = $this->getParentBlock();
      return $parent_block->getContent();
    }
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    // Hide the title display.
    $form['title_display']['#access'] = FALSE;

    $all_block_info = layout_get_block_info();
    if (!isset($this->settings['source_block'])) {
      $blocks = array();
      $usage = layout_get_block_usage();
      foreach ($usage as $module => $block) {
        foreach ($block as $block_delta => $layout_blocks) {
          foreach ($layout_blocks as $layout_name => $block_positions) {
            foreach ($block_positions as $position => $block_instances) {
              foreach ($block_instances as $key => $instance) {
                // todo use custom title if set.
                $key_string = $key > 0 ? ': ' . t('instance !count', array('!count' => $instance + 1)) : ''; 
                $instance_info = $layout_name . t(' layout in ') . $position . t(' region') . $key_string;
                $blocks[$module . ':' . $block_delta . ':' . $layout_name . ':' . $instance->uuid] = $all_block_info[$module][$block_delta]['info'] . ': ' . $instance_info;
              }
            }
          }
        }
      }

      $form['source_block'] = array(
        '#type' => 'select',
        '#title' => t('Select block'),
        '#default_value' => $this->settings['source_block'],
        '#options' => $blocks,
      );
    }
    else {
      list($module, $delta, $layout, $uuid) = explode(':', $this->settings['source_block']);
      $parent_layout = layout_load($layout);
      $block = $parent_layout->content[$uuid];
      $block_name = $all_block_info[$module][$delta]['info'];
      $edit_link = l(t('editing the parent block'), 'admin/structure/layouts/manage/' . $layout . '/configure-redirect/' . $uuid, array('fragment' => 'configure-block:' . $uuid));


      $form['copy_message'] = array(
        '#markup' => t('This is a copy of !block_name block. Settings may be modified by !edit_link. This will load the parent block layout and will open the parent block\'s configuration form', array('!block_name' => $block_name, '!edit_link' => $edit_link)),
      );
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    if (!isset($this->settings['source_block'])) {
      $this->settings['source_block'] = $form_state['values']['source_block'];
    }
  }

  /**
   * Get the block which is being copied.
   */
  function getParentBlock() {
    list($module, $delta, $layout, $uuid) = explode(':', $this->settings['source_block']);
    $parent_layout = layout_load($layout);
    return $parent_layout->content[$uuid];
  }

  /**
   * Get block info for the block which is being copied.
   */
  function getParentBlockInfo($block) {
    $all_block_info = layout_get_block_info();
    return $all_block_info[$block->module][$block->delta];
  }
}
