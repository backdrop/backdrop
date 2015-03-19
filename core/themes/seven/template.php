<?php
/**
 * @file
 * Preprocess functions and theme function overrides for the Seven theme.
 */

/**
 * Prepares variables for layout templates.
 */
function seven_preprocess_layout(&$variables) {
  // Don't modify layouts that are being edited.
  if (!$variables['admin']) {
    // Move the page title and tabs into the "header" area, to fit with Seven's
    // markup requirements.
    if (isset($variables['content']['header'])) {
      if ($variables['title']) {
        $title = '<h1 class="title" id="page-title">' . $variables['title'] . '</h1>';
        $variables['content']['header'] .= $title;
        $variables['title'] = NULL;
      }
      if ($variables['tabs']) {
        $tabs = '<div class="tabs">' . $variables['tabs'] . '</div>';
        $variables['content']['header'] .= $tabs;
        $variables['tabs'] = NULL;
      }
    }
  }
}

/**
 * Overrides theme_node_add_list().
 *
 * Display the list of available node types for node creation.
 */
function seven_node_add_list($variables) {
  $content = $variables['content'];
  $output = '';
  if ($content) {
    $output = '<ul class="admin-list">';
    foreach ($content as $item) {
      $output .= '<li class="clearfix">';
      $output .= '<span class="label">' . l($item['title'], $item['href'], $item['localized_options']) . '</span>';
      $output .= '<div class="description">' . filter_xss_admin($item['description']) . '</div>';
      $output .= '</li>';
    }
    $output .= '</ul>';
  }
  else {
    $output = '<p>' . t('You have not created any content types yet. Go to the <a href="@create-content">content type creation page</a> to add a new content type.', array('@create-content' => url('admin/structure/types/add'))) . '</p>';
  }
  return $output;
}

/**
 * Overrides theme_admin_block_content().
 *
 * Use unordered list markup in both compact and extended mode.
 */
function seven_admin_block_content($variables) {
  $content = $variables['content'];
  $output = '';
  if (!empty($content)) {
    $output = '<ul class="admin-list">';
    foreach ($content as $item) {
      $output .= '<li class="leaf">';
      $output .= l($item['title'], $item['href'], $item['localized_options']);
      if (isset($item['description'])) {
        $output .= '<div class="description">' . filter_xss_admin($item['description']) . '</div>';
      }
      $output .= '</li>';
    }
    $output .= '</ul>';
  }
  return $output;
}

/**
 * Overrides theme_tablesort_indicator().
 *
 * Use our own image versions, so they show up as black and not gray on gray.
 */
function seven_tablesort_indicator($variables) {
  $style = $variables['style'];
  $theme_path = backdrop_get_path('theme', 'seven');
  if ($style == 'asc') {
    return theme('image', array('uri' => $theme_path . '/images/arrow-asc.png', 'alt' => t('sort ascending'), 'width' => 13, 'height' => 13, 'title' => t('sort ascending')));
  }
  else {
    return theme('image', array('uri' => $theme_path . '/images/arrow-desc.png', 'alt' => t('sort descending'), 'width' => 13, 'height' => 13, 'title' => t('sort descending')));
  }
}

/**
 * Implements hook_css_alter().
 */
function seven_css_alter(&$css) {
  // Use Seven's vertical tabs style instead of the default one.
  if (isset($css['core/misc/vertical-tabs.css'])) {
    $css['core/misc/vertical-tabs.css']['data'] = backdrop_get_path('theme', 'seven') . '/css/vertical-tabs.css';
    $css['core/misc/vertical-tabs.css']['type'] = 'file';
  }
  // Use Seven's jQuery UI theme style instead of the default one.
  if (isset($css['core/misc/ui/jquery.ui.theme.css'])) {
    $css['core/misc/ui/jquery.ui.theme.css']['data'] = backdrop_get_path('theme', 'seven') . '/css/jquery.ui.theme.css';
    $css['core/misc/ui/jquery.ui.theme.css']['type'] = 'file';
  }
}

/**
 * Override theme function for breadcrumb trail
 */
function seven_breadcrumb($variables) {
  $breadcrumb = $variables['breadcrumb'];
  $output = '';
  if (!empty($breadcrumb)) {
    $output .= '<nav role="navigation" class="breadcrumb">';
    // Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.
    $output .= '<h2 class="element-invisible">' . t('You are here') . '</h2>';
    // Using theme functiun for it's classes, last-child isn't supported in IE8
    $list_variables = array(
      'title' => '',
      'type'  => 'ol',
      'items' => $breadcrumb,
      'attributes' => array(),
    );
   $output .= theme_item_list($list_variables);
    $output .= '</nav>';
  }
  return $output;
}

/**
 * Override theme_item_list()
 * Removing div.item-list wrapper
 */
function seven_item_list($variables) {
  $items = $variables['items'];
  $title = $variables['title'];
  $type = $variables['type'];
  $list_attributes = $variables['attributes'];

  $output = '';
  if ($items) {
    $output .= '<' . $type . backdrop_attributes($list_attributes) . '>';

    $num_items = count($items);
    $i = 0;
    foreach ($items as $key => $item) {
      $i++;
      $attributes = array();

      if (is_array($item)) {
        $value = '';
        if (isset($item['data'])) {
          $value .= $item['data'];
        }
        $attributes = array_diff_key($item, array('data' => 0, 'children' => 0));

        // Append nested child list, if any.
        if (isset($item['children'])) {
          // HTML attributes for the outer list are defined in the 'attributes'
          // theme variable, but not inherited by children. For nested lists,
          // all non-numeric keys in 'children' are used as list attributes.
          $child_list_attributes = array();
          foreach ($item['children'] as $child_key => $child_item) {
            if (is_string($child_key)) {
              $child_list_attributes[$child_key] = $child_item;
              unset($item['children'][$child_key]);
            }
          }
          $value .= theme('item_list', array(
            'items' => $item['children'],
            'type' => $type,
            'attributes' => $child_list_attributes,
          ));
        }
      }
      else {
        $value = $item;
      }

      $attributes['class'][] = ($i % 2 ? 'odd' : 'even');
      if ($i == 1) {
        $attributes['class'][] = 'first';
      }
      if ($i == $num_items) {
        $attributes['class'][] = 'last';
      }

      $output .= '<li' . backdrop_attributes($attributes) . '>' . $value . '</li>';
    }
    $output .= "</$type>";
  }

  // Only output the list container and title, if there are any list items.
  // Check to see whether the block title exists before adding a header.
  // Empty headers are not semantic and present accessibility challenges.
  if ($output !== '') {
    if (isset($title) && $title !== '') {
      $title = '<h3>' . $title . '</h3>';
    }
    $output = $title . $output;
  }

  return $output;
}
