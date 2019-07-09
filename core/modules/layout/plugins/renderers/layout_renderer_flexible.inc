<?php

/**
 * @file
 * Class file to control the flexible layout editor.
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
    $this->admin = TRUE;

  }

  /**
   * {@inheritdoc}
   */
  function renderRegion($region_id, $blocks) {
    $this->region_buttons[$region_id] = $this->getRegionLinks($region_id);
  }

  /**
   * {@inheritdoc}
   */
  function renderBlock($block) {}

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
