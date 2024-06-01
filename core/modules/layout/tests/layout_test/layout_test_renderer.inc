<?php

/**
 * A test renderer.
 */
class LayoutTestRenderer extends LayoutRendererStandard {
  /**
   * Simply changes the page title.
   */
  function renderRegion($region_id, $blocks) {
    $content = parent::renderRegion($region_id, $blocks);
    if ($region_id == 'header') {
      $content .= 'This layout uses the Test renderer';
    }
    return $content;
  }
}
