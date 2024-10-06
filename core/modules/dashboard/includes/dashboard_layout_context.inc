<?php
/**
 * @file
 * Layout context handler for entities.
 */
class DashboardLayoutContext extends LayoutContext {
  var $usageType = LayoutContext::USAGE_TYPE_SYSTEM;

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
}
