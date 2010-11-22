<?php
// $Id$

if (module_exists('taxonomy')) {
  $plugin = array(
    'name' => 'taxonomy_term',
    'base_table' => 'taxonomy_term_data',
    'form_wizard_class' => array(
      'file' => 'views_ui_taxonomy_term_views_wizard.class.php',
      'class' => 'ViewsUiTaxonomyTermViewsWizard',
    ),
    'title' => t('Taxonomy terms'),
    'filters' => array(
    ),
  );
}
