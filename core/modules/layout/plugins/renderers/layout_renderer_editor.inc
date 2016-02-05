<?php

/**
 * @file
 * Class file to control the main Layout editor.
 */

class LayoutRendererEditor extends LayoutRendererStandard {
  /**
   * {@inheritdoc}
   */
  function __construct(Layout $layout, array $renderer_plugin) {
    parent::__construct($layout, $renderer_plugin);

    // Set the page title region if not already.
    if (!isset($this->layout_info['regions']['title'])) {
      $this->layout_info['regions']['title'] = t('Page title');
    }
    $this->admin = TRUE;

  }

  /**
   * {@inheritdoc}
   */
  function addMeta() {
    parent::addMeta();
    backdrop_add_library('system', 'ui.sortable');
    backdrop_add_js(backdrop_get_path('module', 'layout') . '/js/layout.admin.js');
    backdrop_add_css(backdrop_get_path('module', 'layout') . '/css/layout.admin.css');
  }

  /**
   * {@inheritdoc}
   */
  function render() {
    $this->addMeta();

    $output = '<div id="layout-edit-main">';
    $output .= $this->renderLayout();
    $output .= '</div>';

    return $output;
  }

  /**
   * {@inheritdoc}
   */
  function renderRegion($region_id, $blocks) {
    $output = '';
    if ($region_id == 'title') {
      return $this->renderTitleRegion($region_id);
    }
    else {
      $content = implode('', $blocks);
      $region_buttons = $this->getRegionLinks($region_id);
      
      $output = '<div class="layout-editor-region" id="layout-editor-region-' . $region_id . '" data-region-name="' . $region_id . '">';
      $output .= '<div class="layout-editor-region-title clearfix">';
      $output .= '<h2 class="label">' . check_plain($this->layout_info['regions'][$region_id]) . '</h2>';
      $output .= $region_buttons;
      $output .= '</div>';
      $output .= '<div class="layout-editor-region-content">' . $content . '</div>';
      $output .= '</div>';
    }

    return $output;
  }

  /**
   * Render the Page Title region in the Layout Editor.
   */
  function renderTitleRegion($region_id) {
    $options = array(
      LAYOUT_TITLE_DEFAULT => t('Default title'),
      LAYOUT_TITLE_CUSTOM => t('Custom title'),
      LAYOUT_TITLE_NONE => t('No title'),
    );
    $title_setting = $this->layout->settings['title_display'];
    $description = t('Displaying: ') . $options[$title_setting];
    $region_buttons = $this->getRegionLinks($region_id);
    
    $output = '<div class="layout-editor-region layout-editor-region-page-title" id="layout-editor-region-' . $region_id . '" data-region-name="' . $region_id . '">';
    $output .= '<div class="layout-editor-region-title clearfix">';
    $output .= '<h2 class="label">' . check_plain($this->layout_info['regions'][$region_id]) . '</h2>';
    $output .= $region_buttons;
    $output .= '<div class="description">' . $description . '</div>';
    $output .= '</div>';
    $output .= '</div>';

    return $output;
  }

  /**
   * {@inheritdoc}
   */
  function renderBlock($block) {
    $buttons = $this->getBlockLinks($block);
    $content = $block->getAdminPreview();

    $class = 'layout-editor-block';
    if (isset($this->layout->title_block) && $this->layout->title_block == $block->uuid) {
      $class .= ' layout-editor-block-is-title';
    }
    $class .= ' clearfix';

    $output = '<div class="' . $class . '" id="layout-editor-block-' . $block->uuid . '" data-block-id="' . $block->uuid . '">';
    $output .= '<div class="layout-editor-block-title clearfix">';
    $output .= '<span class="text">' . $block->getAdminTitle() . '</span>';
    if ($buttons) {
      $output .= '<span class="buttons">' . $buttons . '</span>';
    }
    $output .= '</div>'; // layout-block-title
    $output .= '<div class="layout-editor-block-content">' . render($content) . '</div>';
    $output .= '</div>'; // layout-block

    return $output;
  }


  /**
   * {@inheritdoc}
   */
  function renderBlocks() {
    $this->rendered['blocks'] = array();
    foreach ($this->prepared['blocks'] as $uuid => $block) {
      $this->rendered['blocks'][$uuid] = $this->renderBlock($block);
    }
    return $this->rendered['blocks'];
  }

  /**
   * Short-cut to generate a URL path to a particular action.
   *
   * @param string $command
   *   The command to which the URL should point, usually "configure-block",
   *   "remove-block", or "add-block".
   * @param ...
   *   Any additional parameters to be tacked on the end of the URL.
   */
  function getUrl($command) {
    $args = func_get_args();
    $command = array_shift($args);
    $url = 'admin/structure/layouts/manage/' . $this->layout->name . '/' . $command . '/' . $this->plugin['name'];
    if ($args) {
      $url .= '/' . implode('/', $args);
    }
    return $url;
  }

  /**
   * Render the links to display when editing a region.
   */
  protected function getRegionLinks($region_id) {
    if ($region_id == 'title') {
      $title =  t('Configure');
      $link = 'edit-title';
    }
    else {
      $title = t('Add block');
      $link = 'add-block';
    }
    $links = array();
    $links['add'] = array(
      'title' => $title,
      'href' => $this->getUrl($link, $region_id),
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }

  /**
   * Render the links to display when editing a block.
   */
  protected function getBlockLinks($block) {
    $links = array();
    $links['configure'] = array(
      'title' => t('Configure'),
      'href' => $this->getUrl('configure-block', $block->uuid),
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );

    $links['remove'] = array(
      'title' => t('Remove'),
      'href' => $this->getUrl('remove-block', $block->uuid),
      'query' => array('token' => backdrop_get_token('layout-' . $this->layout->name)),
      'attributes' => array(
        'class' => array('remove-block', 'use-ajax'),
      ),
    );

    $dropbutton = array(
      '#type' => 'dropbutton',
      '#links' => $links,
    );

    return backdrop_render($dropbutton);
  }
}
