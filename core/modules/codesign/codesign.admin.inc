<?php

/**
 * Returns the form for the code_sign administration page.
 */
function code_sign_admin_form($form, &$form_state) {
  $form['dummy'] = array(
    '#type' => 'markup',
    '#markup' => t('A dummy admin page so contrib modules can attach their own as sub-menu items.  This will eventually display some site-wide settings or an overview of signing modules.'),
  );

  return $form;
}
