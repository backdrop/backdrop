<?php
/**
 * @file
 * Builds placeholder replacement tokens for menu-related data.
 */

/**
 * Implements hook_token_info().
 */
function menu_token_info() {
  // Menu tokens.
  $info['types']['menu'] = array(
    'name' => t('Menus'),
    'description' => t('Tokens related to menus.'),
    'needs-data' => 'menu',
  );
  $info['tokens']['menu']['name'] = array(
    'name' => t('Name'),
    'description' => t("The name of the menu."),
  );
  $info['tokens']['menu']['machine-name'] = array(
    'name' => t('Machine-readable name'),
    'description' => t("The unique machine-readable name of the menu."),
  );
  $info['tokens']['menu']['description'] = array(
    'name' => t('Description'),
    'description' => t('The optional description of the menu.'),
  );
  $info['tokens']['menu']['menu-link-count'] = array(
    'name' => t('Menu link count'),
    'description' => t('The number of menu links belonging to the menu.'),
  );
  $info['tokens']['menu']['edit-url'] = array(
    'name' => t('Edit URL'),
    'description' => t("The URL of the menu's edit page."),
  );

  $info['tokens']['menu-link']['menu'] = array(
    'name' => t('Menu'),
    'description' => t('The menu of the menu link.'),
    'type' => 'menu',
  );
  $info['tokens']['menu-link']['edit-url'] = array(
    'name' => t('Edit URL'),
    'description' => t("The URL of the menu link's edit page."),
  );
  $info['tokens']['node']['menu-link'] = array(
    'name' => t('Menu link'),
    'description' => t("The menu link for this node."),
    'type' => 'menu-link',
  );

  return $info;
}

/**
 * Implements hook_tokens() on behalf of menu.module.
 */
function menu_tokens($type, $tokens, array $data = array(), array $options = array()) {
  $replacements = array();

  $url_options = array('absolute' => TRUE);
  $sanitize = !empty($options['sanitize']);

  // Node tokens.
  if ($type == 'node' && !empty($data['node'])) {
    $node = $data['node'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'menu-link':
          if ($link = token_node_menu_link_load($node)) {
            $replacements[$original] = $sanitize ? check_plain($link['title']) : $link['title'];
          }
          break;
      }

      // Chained token relationships.
      if ($menu_tokens = token_find_with_prefix($tokens, 'menu-link')) {
        if ($link = token_node_menu_link_load($node)) {
          $replacements += token_generate('menu-link', $menu_tokens, array('menu-link' => $link), $options);
        }
      }
    }
  }

  // Menu link tokens.
  if ($type == 'menu-link' && !empty($data['menu-link'])) {
    $link = (array) $data['menu-link'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'menu':
          if ($menu = menu_load($link['menu_name'])) {
            $replacements[$original] = $sanitize ? check_plain($menu['title']) : $menu['title'];
          }
          break;
        case 'edit-url':
          $replacements[$original] = url("admin/structure/menu/item/{$link['mlid']}/edit", $url_options);
          break;
      }
    }

    // Chained token relationships.
    if (($menu_tokens = token_find_with_prefix($tokens, 'menu')) && $menu = menu_load($link['menu_name'])) {
      $replacements += token_generate('menu', $menu_tokens, array('menu' => $menu), $options);
    }
  }

  // Menu tokens.
  if ($type == 'menu' && !empty($data['menu'])) {
    $menu = (array) $data['menu'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'name':
          $replacements[$original] = $sanitize ? check_plain($menu['title']) : $menu['title'];
          break;
        case 'machine-name':
          // This is a machine name so does not ever need to be sanitized.
          $replacements[$original] = $menu['menu_name'];
          break;
        case 'description':
          $replacements[$original] = $sanitize ? filter_xss($menu['description']) : $menu['description'];
          break;
        case 'menu-link-count':
          $query = db_select('menu_links');
          $query->condition('menu_name', $menu['menu_name']);
          $query->addTag('menu_menu_link_count');
          $count = $query->countQuery()->execute()->fetchField();
          $replacements[$original] = (int) $count;
          break;
        case 'edit-url':
          $replacements[$original] = url("admin/structure/menu/manage/" . $menu['menu_name'], $url_options);
          break;
      }
    }
  }

  return $replacements;
}
