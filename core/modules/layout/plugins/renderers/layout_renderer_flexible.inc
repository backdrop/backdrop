<?php

/**
 * @file
 * Class file to control the main Layout editor.
 */

class LayoutRendererFlexible extends LayoutRendererStandard {

  /**
   * Include rendered HTML prior to the layout.
   *
   * @var array
   */
  var $region_buttons = array();

  /**
   * {@inheritdoc}
   */
  function __construct(Layout $layout, array $default_template_info) {
    $this->layout = &$layout;
    $this->layout_info = $default_template_info;

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
      $this->region_buttons[$region_id] = $this->getRegionLinks($region_id);

      $output = '<div class="layout-editor-region" id="layout-editor-region-' . $region_id . '" data-region-name="' . $region_id . '">';
      $output .= '<div class="layout-editor-region-title clearfix">';
      $output .= '<h2 class="label">' . check_plain($this->layout_info['regions'][$region_id]) . '</h2>';
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
    $description = layout_get_title_description($this->layout);

    $output  = '<div class="layout-editor-block" id="layout-editor-title" data-region-name="' . $region_id . '">';
    $output .= '  <div class="layout-editor-block-title clearfix">';
    $output .= '    <span class="text">' . check_plain($this->layout_info['regions'][$region_id]) . '</span>';
    $output .= '    <span class="description">' . $description . '</span>';
    $output .= '  </div>';
    $output .= '</div>';

    return $output;
  }

  /**
   * {@inheritdoc}
   */
  function renderBlock($block) {}


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
   *
   * @return string
   *   A Backdrop menu path.
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
    $links = array();
    $links['add-above'] = array(
      'title' => t('Add row above'),
      // 'href' => $this->getUrl('add-block', $region_id, $this->layout_info['name']),
      'href' => 'admin/structure/layouts/flexible-layout/' . $this->layout_info['name'] . '/' . $region_id . '/before/add-row',
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );
    $links['add'] = array(
      'title' => t('Add row below'),
      // 'href' => $this->getUrl('add-row', $region_id),
      'href' => 'admin/structure/layouts/settings/' . $this->layout_info['name'] . '/' . $region_id . '/after/add-row',
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );
    $links['configure'] = array(
      'title' => t('Configure row'),
      'href' => $this->getUrl('configure-region', $region_id),
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
  protected function getBlockLinks($block) {}

  /**
   * Perform display/layout-level render operations.
   *
   * This method triggers all the inner block/region rendering processes, passes
   * that to the layout plugin's theme callback, and returns the rendered HTML.
   *
   * If display-level caching is enabled and that cache is warm, this method
   * will not be called.
   *
   * @return string
   *   The HTML string representing the entire rendered, themed layout.
   */
  function renderLayout() {
    if (empty($this->prep_run)) {
      $this->prepare();
    }

    $this->renderBlocks();

    $this->renderRegions();

    if ($this->page_title) {
      backdrop_set_title($this->page_title);
    }

    module_load_include('inc', 'layout', 'layout.theme');
    if (empty($this->layout_info['template'])) {
      $theme = 'layout__' . $this->layout->layout_template;
    }
    else {
      $theme = str_replace('-', '_', $this->layout_info['template']);
    }
    $this->rendered['layout'] = theme($theme, array('content' => $this->rendered['regions'], 'region_buttons' => $this->region_buttons, 'layout_info' => $this->layout_info));
    return $this->prefix . $this->rendered['layout'] . $this->suffix;
  }
}
