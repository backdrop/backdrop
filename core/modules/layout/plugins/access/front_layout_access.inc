<?php
/**
 * @file
 * Class for loading, modifying, and saving a layout access rule (condition).
 */
class FrontLayoutAccess extends LayoutAccessNegatable {
  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    return backdrop_is_front_page();
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    return $this->settings['negate'] ? t('Is not the front page') : t('Is the front page');
  }
}
