<?php

/**
 *  BlockRegionCopy extends Block
 *
 *  This class allows us to display the blocks from a chosen region.
 */
class BlockRegionCopy extends Block {
  /**
   *  Sets title text on draggable block panel in Layout builder.
   */
  function getAdminTitle() {
    // Get the source layout and region.
    list($layout_name, $region) = explode(':', $this->settings['source_region']);
    $layout = layout_load($layout_name);
    $layout_info = layout_get_layout_template_info($layout->layout_template);

    return t('Copy of %region region from %layout', array('%region' => $layout_info['regions'][$region], '%layout' => $layout->title));
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    // region blocks shouldn't have titles.
    return '';
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   */
  function getContent() {
    // Get the source layout and region.
    list($layout_name, $region) = explode(':', $this->settings['source_region']);
    $layout = layout_load($layout_name);

    if (isset($layout->positions[$region])) {
      $blocks = array();
      foreach ($layout->positions[$region] as $uuid) {
        if (isset($layout->content[$uuid])) {
          $block = $layout->content[$uuid];
          $blocks[] = $block->delta != 'main' ? $this->renderBlock($layout, $block) : '';
        }
      }
      $rendered_region = $this->renderRegion($layout, $region_id, $blocks);
      return $rendered_region;
    }
    return '';
  }

  /**
   * Render a block using its designated style.
   *
   * @param Layout $layout
   *   The layout object containing the blocks to be copied.
   * @param Block $block
   *   The block to be rendered.
   *
   * @return string
   *   The rendered block as HTML.
   */
  function renderBlock($layout, $block) {
    $output = '';
    if ($content = $block->getContent()) {
      $style = layout_get_style_info($block->style->plugin);

      // Ensure block content is a renderable array for contextual links.
      if (is_string($content)) {
        $content = array(
          '#markup' => $content,
        );
      }
      // Add the contextual links.
      if (!($block->module === 'system' && $block->delta === 'main')) {
        $content['#contextual_links']['layout'] = array('admin/structure/layouts/manage/' . $layout->name . '/modal', array($block->uuid));
      }

      $title = '';
      if ($block->settings['title_display'] !== LAYOUT_TITLE_NONE) {
        $title = $block->getTitle();
      }

      // Assemble a structured content array.
      $content = (object) array(
        'title' => $title,
        'content' => $content,
      );

      if (isset($style) && isset($style['block theme'])) {
        $output = theme($style['block theme'] . '__' . $block->module . '__' . strtr($block->delta, '-', '_'), array('content' => $content, 'layout' => $layout, 'block' => $block, 'style' => $style, 'settings' => $block->style->settings));
      }
      else {
        // Fallback.
        $output = theme('block', array('content' => $content, 'layout' => $layout, 'block' => $block));
      }
    }
    return $output;
  }

  /**
   * Render the layout region to be copied.
   *
   * @param Layout $layout
   *   The layout object containing the blocks to be copied.
   * @param $region_id
   *   The ID of the layout region being rendered.
   * @param $blocks
   *   An array of blocks that are assigned to the region that's being rendered.
   *
   * @return string
   *   The rendered, HTML string output of the passed-in region.
   */
  function renderRegion($layout, $region_id, $blocks) {
    $settings = $layout->settings;
    if (empty($settings)) {
      $style_settings = isset($settings['style_settings']['default']) ? $settings['style_settings']['default'] : array();
    }
    else {
      $style_settings = isset($settings[$region_id]) ? $settings[$region_id] : array();
    }
    $tag = isset($style_settings['element']) ? $style_settings['element'] : '';
    $classes = array();
    if (isset($style_settings['classes'])) {
      $classes =  explode(' ', $style_settings['classes']);
    }

    return theme('layout_region_inner', array('blocks' => $blocks, 'tag' => $tag, 'classes' => $classes));
  }

  /**
   *  Builds the block's configuration form.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $regions = array();
    $all_layouts = layout_load_all();
    foreach ($all_layouts as $layout_name => $layout) {
      $layout_info = layout_get_layout_template_info($layout->layout_template);
      foreach ($layout->positions as $region => $position) {
        if ($region != 'title') {
          $region_name = $layout_info['regions'][$region];
          // Remove the the word "layout" from the end of layout names.
          $layout_title = str_ireplace(' layout', '', $layout->title);
          $regions[$layout_name . ':' . $region] = t('!region_name region from !layout layout', array('!region_name' => $region_name, '!layout' => $layout_title));
        }
      }
    }

    $form['title_display']['#access'] = FALSE;

    $form['source_region'] = array(
      '#type' => 'select',
      '#title' => t('Select region'),
      '#default_value' => $this->settings['source_region'],
      '#options' => $regions,
    );

  }

  /**
   * Submit handler to save the form settings.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['source_region'] = $form_state['values']['source_region'];
  }
}
