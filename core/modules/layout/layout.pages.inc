<?php
/**
 * @file
 * Page callbacks for non-administrative pages of Layout module.
 */

/**
 * Menu callback; Generic page callback for all Layout-provided pages.
 */
function layout_page_callback() {
  $args = func_get_args();
  $menu_item_name = array_shift($args);
  $menu_item = layout_menu_item_load($menu_item_name);

  // Determine the correct layout to use at this path.
  $router_item_path = $menu_item->path;
  if (strpos($router_item_path, '%') !== FALSE) {
    $router_item_path = current_path();
  }
  $router_item = menu_get_item($router_item_path);
  $layout = layout_get_layout_by_path(NULL, $router_item, TRUE);

  // No layouts match based on conditions.
  if (!$layout) {
    return MENU_NOT_FOUND;
  }

  // Use the selected layout and render it.
  $renderer = layout_create_renderer($layout->renderer_name, $layout);
  return $renderer->render();
}

/**
 * Access callback; Check access for Layout-provided pages.
 *
 * This private function is called from layout_page_access() in layout.module.
 */
function _layout_page_access() {
  $args = func_get_args();
  $menu_item_name = array_shift($args);
  $menu_item = layout_menu_item_load($menu_item_name);

  if ($menu_item) {
    // Populate the contexts from the menu system.
    $contexts = $menu_item->getContexts();
    foreach ($contexts as $context) {
      if (isset($context->position) && isset($args[$context->position])) {
        $context->setData($args[$context->position]);
      }
    }
    return $menu_item->checkAccess();
  }

  return FALSE;
}
