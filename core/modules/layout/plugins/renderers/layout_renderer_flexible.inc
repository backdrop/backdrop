<?php

/**
 * @file
 * Class file to control the main Layout editor.
 */

class LayoutRendererFlexible extends LayoutRendererEditor {

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
    $this->plugin = $default_template_info['plugin'];
    $this->admin = TRUE;
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
  function renderRegion($region_id, $blocks) {
    $output = '';
    if ($region_id == 'title') {
      return '';
    }
    else {
      $content = implode('', $blocks);
      $this->region_buttons[$region_id] = $this->getRegionLinks($region_id);

      $output = '<div class="layout-editor-region" id="layout-editor-region-' . $region_id . '" data-region-name="' . $region_id . '">';
      $output .= '<div class="layout-editor-region-title clearfix">';
      $output .= '<h2 class="label">' . check_plain($this->layout_info['regions'][$region_id]) . '</h2>';
      $output .= '</div>';
      //$output .= '<div class="layout-editor-region-content">' . $content . '</div>';
      $output .= '</div>';
    }

    return $output;
  }

  /**
   * {@inheritdoc}
   */
  function renderBlock($block) {}

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
    $links['configure'] = array(
      'title' => t('Configure row'),
      'href' => 'admin/structure/layouts/settings/flexible-template/' . $this->layout_info['name'] . '/row/' . $region_id . '/configure',
      'attributes' => array(
        'class' => array('use-ajax'),
        'data-dialog' => TRUE,
        'data-dialog-options' => json_encode(array('dialogClass' => 'layout-dialog')),
      ),
    );
    $links['delete'] = array(
      'title' => t('Delete row'),
      'href' => 'admin/structure/layouts/settings/flexible-template/' . $this->layout_info['name'] . '/row/' . $region_id . '/delete',
      'attributes' => array(
        'class' => array('use-ajax'),
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

}
