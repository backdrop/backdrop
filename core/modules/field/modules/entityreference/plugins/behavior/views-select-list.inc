<?php

if (module_exists('views')) {
  $plugin = array(
    'title' => t('Render Views filters as select list'),
    'description' => t('Provides a select list for Views filters on this field. This should not be used when there are over 100 entities, as it might cause an out of memory error.'),
    'class' => 'EntityReferenceBehavior_ViewsFilterSelect',
    'behavior type' => 'field',
  );
}
