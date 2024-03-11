<?php
/**
 * @file
 * Theme functions for the Layout module.
 */

/**
 * Turn the rearrange form into a table with table sorting displayed.
 */
function theme_layout_reorder_layouts($variables) {
  $element = $variables['element'];
  $rows = array();

  // Assemble the data for a table from everything in $form['handlers']
  foreach (element_children($element) as $layout_name) {
    if (isset($element[$layout_name]['title'])) {
      $row = array();
      $row[] = array(
        'data' => render($element[$layout_name]['title']),
        'class' => array('layout-name'),
      );
      $element[$layout_name]['weight']['#attributes']['class'][] = 'weight';
      $row[] = render($element[$layout_name]['weight']);
      $rows[] = array('data' => $row, 'class' => array('draggable'));
    }
  }

  $header = array(
    array('data' => t('Layout'), 'class' => array('layout-name')),
    t('Weight'),
  );

  backdrop_add_tabledrag('layout-reorder', 'order', 'sibling', 'weight');

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'layout-reorder')));
  $output .= backdrop_render_children($element);
  return $output;
}

/**
 * Outputs information about a layout. Used in the layout listing page.
 *
 * @param $variables
 *   An array with a single value: "layout", which contains the layout object.
 *
 * @return string
 */
function theme_layout_info($variables) {
  $layout = $variables['layout'];
  $info = layout_get_layout_template_info($layout->layout_template);

  // Build template thumbnail preview
  $preview_path = $info['path'] . '/' . $info['preview'];
  $preview_img = theme('image', array('path' => $preview_path));
  $preview = l($preview_img, 'admin/structure/layouts/manage/' . $layout->name, array('html' => TRUE));

  // Get the label and machine name.
  $label = theme('label_machine_name__layout', array(
    'label' => $layout->title,
    'machine_name' => $layout->name,
  ));

  $output = '<div class="layout-info">';
  $output .= $preview;
  $output .= '<div class="layout-detail">' . $label . '</div>';
  $output .= '</div>';

  return $output;
}

/**
 * Outputs a list of conditions for a layout. Used in the layout listing page.
 *
 * @param $variables
 *   An array with a single value: "layout", which contains the layout object.
 *
 * @return string
 */
function theme_layout_condition_info($variables) {
  /* @var Layout $layout */
  $layout = $variables['layout'];
  // Build a list of visibility conditions on this layout
  $conditions = array();
  foreach ($layout->conditions as $access_key => $layout_access) {
    $conditions[] = $layout_access->summary();
  }
  $output  = '<div class="layout-conditions-list">';
  if (empty($conditions)) {
    $output .= t('No conditions set.');
  }
  else {
    $output .= theme('item_list', array('items' => $conditions));
  }
  $output .= '</div>';

  return $output;
}

/**
 * Outputs an individual option label for selecting a layout.
 */
function theme_layout_template_option($variables) {
  $template_info = $variables['template_info'];
  $icon_path = $template_info['path'] . '/' . $template_info['preview'];

  $output = '';
  $output .= '<div class="layout-icon">';
  $output .= theme('image', array(
    'uri' => $icon_path,
    'attributes' => array('title' => $template_info['title']),
  ));
  $output .= '<div class="layout-caption">' . check_plain($template_info['title']) . '</div>';
  $output .= '</div>';
  return $output;
}

/**
 * Outputs information about a layout template . Used in the layout template
 * listing page.
 *
 * @param $variables
 *   An array with a single value: "template_info", which contains information
 *   about the layout template.
 *
 * @return string
 */
function theme_layout_template_info($variables) {
  $template_info = $variables['template_info'];

  // Build template thumbnail preview
  $preview_path = $template_info['path'] . '/' . $template_info['preview'];
  $preview_img = theme('image', array('path' => $preview_path));

  // Get the label and machine name.
  $label = theme('label_machine_name__layout_template', array(
    'label' => $template_info['title'],
    'machine_name' => $template_info['name'],
  ));

  $output = '<div class="layout-info">';
  $output .= $preview_img;
  $output .= '<div class="layout-detail">' . $label . '</div>';
  $output .= '</div>';

  return $output;
}

/**
 * Outputs a representation of the column widths of a flexible layout template
 * row.
 */
function theme_layout_flexible_template_style_option($variables) {
  $row_style = $variables['row_style'];
  if ($row_style['region_count'] == 1) {
    $regions = array('12');
    $percent = array('100');
  }
  else {
    $regions = explode(':', $row_style['bootstrap']);
    $percent = explode(':', $row_style['split']);
  }
  $output = '';
  $output .= '<div class="flexible-icon container">';
  $output .= '<div class="flexible-icon-row row">';
  foreach ($regions as $index => $region) {
    $output .= '<div class="flexible-icon-region col-md-' . $region . '">' . $percent[$index] . '%</div>';
  }
  $output .= '</div>';
  $output .= '</div>';
  return $output;
}

/**
 * Output the arguments table for layout_menu_item_arguments_form().
 */
function theme_layout_menu_item_arguments_table($variables) {
  $element = $variables['element'];
  $path = $element['#layout_path'];
  $parts = explode('/', $path);
  $header = array(
    t('Argument position'),
    t('Context'),
    t('Operations'),
  );
  $rows = array();
  foreach (element_children($element) as $key) {
    $argument_parts = $parts;
    $argument_parts[$key] = '<strong>' . $argument_parts[$key] . '</strong>';
    $row = array();
    $row[] = t('Position @index:', array('@index' => $key + 1)) . ' ' . implode('/', $argument_parts);
    $row[] = backdrop_render($element[$key]['context']);
    $row[] = backdrop_render($element[$key]['operations']);
    $rows[] = $row;
  }
  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'layout-arguments')));
}

/**
 * Output the table for layout_settings_form() contexts.
 */
function theme_layout_settings_context_table($variables) {
  $element = $variables['element'];
  $path = $element['#layout_path'];
  $parts = explode('/', $path);
  $header = array(
    t('Context'),
    t('Type'),
    t('Operations'),
  );
  $rows = array();
  foreach (element_children($element) as $key) {
    $argument_parts = $parts;
    $row = array();
    $row[] = backdrop_render($element[$key]['summary']);
    $row[] = backdrop_render($element[$key]['plugin']);
    if (isset($element[$key]['operations'])) {
      $row[] = backdrop_render($element[$key]['operations']);
    }
    else {
      $row[] = array();
    }

    $rows[] = $row;
  }
  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('id' => 'layout-context-table'),
    'empty' => t('No contexts'),
  ));
}

/**
 * Output a list of action links for layouts.
 */
function theme_layout_action_links($variables) {
  $element = $variables['element'];
  $attributes = array('class' => array('action-links', 'layout-action-links'));

  $output = '<ul ' . backdrop_attributes($attributes) . '>';
  foreach (element_children($element) as $key) {
    $output .= '<li>' . backdrop_render($element[$key]) . '</li>';
  }
  $output .= '</ul>';
  return $output;
}


/**
 * Output the list of visibility conditions for layouts.
 */
function theme_layout_conditions($variables) {
  $element = $variables['element'];

  $output = '<ul>';
  foreach (element_children($element) as $key) {
    $output .= '<li>' . backdrop_render($element[$key]) . '</li>';
  }
  $output .= '</ul>';
  return $output;
}

/**
 * Output region content, including title and/or wrapping tag as specified in
 * the layout UI.
 */
function theme_layout_region_inner($variables) {
  $tag = $variables['tag'];
  $attributes['class'] = $variables['classes'];
  $title = $variables['title'];
  $title_tag = $variables['title_tag'];
  $title_attributes['class'] = $variables['title_classes'];

  if ($title != '') {
    $output = '<' . $title_tag;
    // Region titles do not have any default class, so conditionally render them
    // if any has been configured in the layout UI.
    $output .= !empty($title_attributes['class']) ? backdrop_attributes($title_attributes) : '';
    $output .= '>' . $title . '</' . $title_tag . '>';
  }

  $output .= implode('', $variables['blocks']);

  if ($tag != '') {
    $output = '<' . $tag . backdrop_attributes($attributes) . '>' . $output . '</' . $tag . '>';
  }

  return $output;
}

/**
 * Prepares variables for layout templates.
 *
 * This uses [module_name]_preprocess_layout() instead of
 * template_preprocess_layout() so that it can run last. Layout module is given
 * a weight of 60 in hook_install(), so it should run last compared to other
 * modules.
 */
function layout_preprocess_layout(&$variables) {
  $settings = $variables['layout']->settings;

  // Check if any Title Component blocks have been placed. These will override
  // the variables here.
  $block_exists = _layout_page_component_blocks_exist($variables);

  $variables += array(
    'action_links' => NULL,
    'tabs' => NULL,
    'messages' => NULL,
    'title' => NULL,
  );

  if (!$variables['admin']) {
    if (!isset($variables['title']) && !$block_exists['title']) {
      if ($settings['title_display'] === LAYOUT_TITLE_NONE) {
        $variables['title'] = NULL;
      }
      else {
        $variables['title'] = backdrop_get_title();
      }
    }

    // Generate messages last in order to capture as many as possible for the
    // current page.
    if (!isset($variables['messages']) && !$block_exists['messages']) {
      $variables['messages'] = theme('status_messages');
    }
  }
  else {
    $variables['title'] = isset($variables['content']['title']) ? $variables['content']['title'] : '';
  }

  if (!$variables['admin']) {
    if (!$block_exists['action_links']) {
      $variables['action_links'] = menu_local_actions();
    }
    if (!$block_exists['tabs']) {
      $variables['tabs'] = menu_local_tabs();
    }
  }

  // Build HTML for displaying flexible templates.
  if (isset($variables['layout_info']['template']) && ($variables['layout_info']['template'] == 'layout--flexible')) {
    if (is_a($variables['renderer'], 'LayoutRendererFlexible')) {
      $variables['flexible_editor'] = TRUE;
      $flexible_layout = layout_flexible_tempstore_load($variables['layout_info']['name']);
    }
    else {
      $variables['flexible_editor'] = FALSE;
      $flexible_layout = layout_flexible_template_load($variables['layout_info']['name']);
    }
    $variables['region_buttons'] = array();
    if (isset($variables['renderer']->region_buttons)) {
      $variables['region_buttons'] = $variables['renderer']->region_buttons;
    }

    $variables['column_data'] = layout_flexible_row_styles();

    $variables['rows'] = $flexible_layout->rows;
    $variables['row_data'] = array();

    $variables['row_widths'] = array(
      'container' => t('Fixed maximum width'),
      'container_fluid' => t('Fluid width'),
      'no_container' => t('Full width'),
    );
    foreach ($variables['rows'] as $name => $row) {
      $container = ($row['container'] == 'container') ? 'container container-fluid' : (($row['container'] == 'container_fluid') ? 'container-fluid' : 'no-container');
      $row['row_class'] = $container . ' flexible-row--' . $name . ' ' . $row['contains'] . ' ' . $row['classes'];
      $row['row_id'] = $variables['flexible_editor'] ? 'id = "flexible-row--' . $name . '"' : '';
      $row['element'] = !empty($row['element']) ? $row['element'] : 'div';
      $row['row_label'] = t('@element row', array('@element' => "<$row[element]>"));
      if ($row['contains'] == 'region_12') {
        $region_classes = '';
        if (!empty($row['region_names']['region_0'])) {
          $region_name = $row['region_names']['region_0']['label'];
          $class_name = isset($row['region_names']['region_0']['name']) ? $row['region_names']['region_0']['name'] : backdrop_html_class($row['region_names']['region_0']['label']);
          $region_classes .= 'l-region--' . $class_name;
          $custom_classes = !empty($row['region_names']['region_0']['classes']) ? ' ' . _layout_clean_custom_css($row['region_names']['region_0']['classes']) : '';
          $region_classes .= !empty($row['region_names']['region_0']['region_class_enable']) ? $custom_classes : '';
        }
        else {
          $region_name = $name;
          $region_classes .= 'l-region--' . backdrop_html_class($name);
        }
        $row['regions'][0]['region_md'] = '12';
        $row['regions'][0]['region_name'] = $region_name;
        $row['regions'][0]['region_classes'] = $region_classes;
        $row['regions'][0]['content_key'] = $variables['flexible_editor'] ? $name : $name . '--0';
      }
      else {
        $col_info = $variables['column_data'][$row['contains']];
        $split = explode(':', $col_info['bootstrap']);
        $i = 0;
        foreach ($split as $col) {
          $region_classes = '';
          if (!empty($row['region_names']['region_' . $i])) {
            $region_name = $row['region_names']['region_' . $i]['label'];
            $class_name = isset($row['region_names']['region_' . $i]['name']) ? $row['region_names']['region_' . $i]['name'] : backdrop_html_class($row['region_names']['region_' . $i]['label']);
            $region_classes .= 'l-region--' . $class_name;
            $custom_classes = !empty($row['region_names']['region_' . $i]['classes']) ? ' ' . _layout_clean_custom_css($row['region_names']['region_' . $i]['classes']) : '';
            $region_classes .= !empty($row['region_names']['region_' . $i]['region_class_enable']) ? $custom_classes : '';
          }
          else {
            $region_name = $name . ' ' . $i;
            $region_classes .= 'l-region--' . backdrop_html_class($name . ' ' . $i);
          }
          $row['regions'][] = array(
            'region_md' => $col,
            'region_name' => $region_name,
            'region_classes' => $region_classes,
            'content_key' => $variables['flexible_editor'] ? $name : $name . '--' . $i,
          );
          $i++;
        }
      }
      $variables['row_data'][$name] = $row;
    }

  }
}

/**
 * Utility function to return if a title component block is in a layout.
 */
function _layout_page_component_blocks_exist($variables) {
  $active_component_blocks = array();
  foreach ($variables['layout']->content as $block) {
    if ($block->delta == 'page_components') {
      $active_component_blocks[] = $block->childDelta;
    }
  }

  $block_exists['title'] = (in_array('title_combo', $active_component_blocks) || in_array('title', $active_component_blocks));
  $block_exists['messages'] = (in_array('title_combo', $active_component_blocks) || in_array('messages', $active_component_blocks));
  $block_exists['tabs'] = (in_array('title_combo', $active_component_blocks) || in_array('tabs', $active_component_blocks));
  $block_exists['action_links'] = (in_array('title_combo', $active_component_blocks) || in_array('action_links', $active_component_blocks));

  return $block_exists;
}

/**
 * Prepares variables for block templates.
 */
function template_preprocess_block(&$variables) {
  $content = $variables['content'];
  $block = $variables['block'];
  $style = $block->style;

  $variables['classes'] = array();
  $variables['attributes'] = array();
  $variables['title_prefix'] = '';
  $variables['title_suffix'] = '';

  // Basic classes.
  $variables['classes'][] = 'block';
  $variables['classes'][] = backdrop_html_class('block-' . $block->module . '-' . (isset($block->childDelta) ? $block->childDelta : $block->delta));

  // Add the hero background image inline. This could also be added in a
  // specific template_preprocess_block__layout__hero() function, but we add
  // it here just to cut down on unnecessary separation.
  if (get_class($block) == 'BlockHero') {
    $variables['classes'][] = 'block-hero';
    if (isset($block->settings['image_path'])) {
      $variables['attributes']['style'] = 'background-image:url(' . $block->settings['image_path'] . ');';
      $variables['classes'][] = 'block-hero-image';
    }
    else {
      $variables['classes'][] = 'block-hero-no-image';
    }
  }

  // Add custom classes if sent in.
  if (!empty($style->settings['classes'])) {
    $new_classes = explode(' ', $style->settings['classes']);
    foreach ($new_classes as $class) {
      $variables['classes'][] = backdrop_clean_css_identifier($class, array());
    }
    $variables['classes'] = array_filter($variables['classes']);
  }

  if (module_exists('contextual') && user_access('access contextual links')) {
    $links = array();
    // These are specified by the content.
    if (!empty($content->admin_links)) {
      $links += $content->admin_links;
    }

    // Take and contextual links that may have been returned in the block render
    // array and move them up into the block wrapper's contextual links.
    if (is_array($content->content) && isset($content->content['#contextual_links'])) {
      $element = array(
        '#type' => 'contextual_links',
        '#contextual_links' => $content->content['#contextual_links'],
        '#element' => $content->content,
      );
      unset($content->content['#contextual_links']);

      $element = contextual_pre_render_links($element);
      $links += $element['#links'];
    }

    if ($links) {
      $build = array(
        '#prefix' => '<div class="contextual-links-wrapper">',
        '#suffix' => '</div>',
        '#theme' => 'links__contextual',
        '#links' => $links,
        '#attributes' => array('class' => array('contextual-links')),
        '#attached' => array(
          'library' => array(array('contextual', 'contextual-links')),
        ),
      );
      $variables['classes'][] = 'contextual-links-region';
      $variables['title_prefix'] = backdrop_render($build);
    }
  }

  // Add a NID class to node blocks.
  if ($block->plugin == 'node:content') {
    $variables['classes'][] = 'block-node-content-' . $block->settings['nid'];
  }

  // Set up some placeholders for constructing template file names.
  $base = 'block';
  $delimiter = '__';

  // Add template file suggestion for content type and sub-type.
  $variables['theme_hook_suggestions'][] = $base . $delimiter . strtr($block->plugin, '-', '_');

  $variables['title'] = !empty($content->title) ? $content->title : '';
  $variables['content'] = !empty($content->content) ? $content->content : '';
}

/**
 * Prepares variables for block-dynamic templates.
 */
function template_preprocess_block_dynamic(&$variables) {
  // Re-use all the preprocessing provided by the default block template.
  template_preprocess_block($variables);

  $block = $variables['block'];
  $style = $block->style;

  // Add the wrapper tag. Classes are set in template_preprocess_block().
  $variables['wrapper_tag'] = $style->settings['wrapper_tag'];

  // Add title tag and classes.
  $title_classes = explode(' ', $style->settings['title_classes']);
  foreach ($title_classes as $n => $class) {
    $title_classes[$n] = backdrop_clean_css_identifier($class, array());
  }
  $title_classes = array_filter($title_classes);
  $variables['title_tag'] = $style->settings['title_tag'];
  $variables['title_attributes'] = array();
  if (!empty($title_classes)) {
    $variables['title_attributes']['class'] = $title_classes;
  }

  // Add content tag and classes.
  $content_classes = explode(' ', $style->settings['content_classes']);
  foreach ($content_classes as $n => $class) {
    $content_classes[$n] = backdrop_clean_css_identifier($class, array());
  }
  $content_classes = array_filter($content_classes);
  $variables['content_tag'] = $style->settings['content_tag'];
  $variables['content_attributes'] = array();
  if (!empty($content_classes)) {
    $variables['content_attributes']['class'] = $content_classes;
  }
}
