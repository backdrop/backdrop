<?php

/**
 * @file
 *
 * A dashboard block.
 */


class DashboardBlock extends Block {

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    $title = parent::getTitle();

    // Use the default title if expected.
    if (empty($title) && $this->settings['title_display'] === LAYOUT_TITLE_DEFAULT) {
      $info = $this->getBlockInfo();
      return $info['info'];
    }

    return $title;
  }
}
