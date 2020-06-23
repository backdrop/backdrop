<?php
/**
 * @file
 * Layout context handler for entities.
 */
class DashboardLayoutContext extends LayoutContext {
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
