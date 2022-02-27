<?php

/**
 * @file
 * Holds the Entity module's theme functions.
 */

/**
 * Process variables for entity.tpl.php.
 */
function template_preprocess_entity(&$variables) {
  $variables['view_mode'] = $variables['elements']['#view_mode'];
  $variables['entity_type'] = $variables['elements']['#entity_type'];
  $entity_type = $variables['entity_type'];
  $variables['entity'] = $variables['elements']['#entity'];
  $entity = $variables['entity'];
  $variables['entity_id'] = $entity->id();
  $info = entity_get_info($entity_type);

  $variables['title'] = check_plain(entity_label($entity_type, $entity));

  $uri = entity_uri($entity_type, $entity);
  $uri += array('options' => array());
  $variables['url'] = $uri && !empty($uri['path']) ? url($uri['path'], $uri['options']) : FALSE;

  if (isset($variables['elements']['#page'])) {
    // If set by the caller, respect the page property.
    $variables['page'] = $variables['elements']['#page'];
  }
  else {
    // Else, try to automatically detect it.
    $variables['page'] = $uri && !empty($uri['path']) && $uri['path'] == $_GET['q'];
  }

  // Helpful $content variable for templates.
  $variables['content'] = array();
  foreach (element_children($variables['elements']) as $key) {
    $variables['content'][$key] = $variables['elements'][$key];
  }

  if (!empty($info['fieldable'])) {
    // Make the field variables available with the appropriate language.
    field_attach_preprocess($entity_type, $entity, $variables['content'], $variables);
  }
  list(, , $bundle) = entity_extract_ids($entity_type, $entity);

  // Gather css classes.
  $variables['classes'][] = backdrop_html_class('entity-' . $entity_type);
  $variables['classes'][] = backdrop_html_class('entity-' . $entity_type . '-' . $bundle);
  if ($variables['view_mode']) {
    $variables['classes'][] = backdrop_html_class('view-mode-' . $variables['view_mode']);
  }

  // Add suggestions.
  $variables['theme_hook_suggestions'][] = 'entity__' . $entity_type;
  $variables['theme_hook_suggestions'][] = 'entity__' . $entity_type . '__' . $bundle;
  $variables['theme_hook_suggestions'][] = 'entity__' . $entity_type . '__' . $bundle . '__' . $variables['view_mode'];
  if ($id = $entity->id()) {
    $variables['theme_hook_suggestions'][] = 'entity__' . $entity_type . '__' . $id;
  }
}
