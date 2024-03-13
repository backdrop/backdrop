<?php
/**
 * @file
 * Layout context handler for entities.
 */
class DashboardLayoutContext extends LayoutContext {
  var $usageType = LayoutContext::USAGE_TYPE_CUSTOM;

  /**
   * Return the indicator for this context.
   */
  function type() {
    return 'dashboard';
  }

  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    return t('Dashboard');
  }

  /**
   * Submit handler for the settings form for this context item.
   */
  function formSubmit($form, &$form_state) {
    $this->name = 'dashboard';
    $this->settings['id'] = 'dashboard';
  }

  /**
   * Return a summary for this context.
   */
  function getAdminSummary($path = NULL) {
    return t('Dashboard');
  }
}
