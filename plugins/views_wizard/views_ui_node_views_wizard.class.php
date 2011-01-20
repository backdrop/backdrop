<?php

class ViewsUiNodeViewsWizard extends ViewsUiBaseViewsWizard {

  /**
   * @override
   */
  protected function default_display_options($form, $form_state) {
    $display_options = parent::default_display_options($form, $form_state);

    // Add permission-based access control.
    $display_options['access']['type'] = 'perm';

    // Display full nodes by default, rather than fields.
    $display_options['row_plugin'] = 'node';
    $display_options['row_options']['links'] = 1;

    return $display_options;
  }
}
