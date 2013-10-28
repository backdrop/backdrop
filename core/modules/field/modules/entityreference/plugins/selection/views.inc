<?php

if (module_exists('views')) {
  $plugin = array(
    'title' => t('Views: Filter by an entity reference view'),
    'class' => 'EntityReference_SelectionHandler_Views',
    'weight' => 0,
  );
}
