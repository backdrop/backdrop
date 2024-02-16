<?php
/**
 * @file
 * Theme functions for the Node module.
 */

/**
 * Returns HTML for node_preview_banner_form().
 *
 * @param $variables
 *   An associative array containing:
 *   - form: A render element representing the form.
 *
 * @see node_preview_banner_form()
 * @ingroup themeable
 */
function theme_node_preview_banner_form($variables) {
  $form = $variables['form'];

  $output = '<div class="node-preview-banner-form">';
  $output .= '<div class="node-preview-banner-form-actions">';
  $output .= backdrop_render($form['preview_node_submit']);
  $output .= backdrop_render($form['backlink']);
  $output .= '</div>';
  $output .= '<div class="node-preview-banner-form-view-mode">';
  $output .= backdrop_render($form['view_mode']);
  $output .= backdrop_render($form['view_mode_submit']);
  $output .= '</div>';
  $output .= '</div>';

  $output .= backdrop_render_children($form);
  return $output;
}

/**
 * Returns HTML for the content ranking part of the search settings admin page.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the content rankings portion of the form.
 *
 * @see node_search_admin()
 * @ingroup themeable
 */
function theme_node_search_factors($variables) {
  $element = $variables['element'];

  $rows = array();
  $header = array(t('Factor'), t('Influence'));
  foreach (element_children($element) as $key) {
    $row = array();
    $row[] = $element[$key]['#title'];
    $element[$key]['#title_display'] = 'invisible';
    $row[] = backdrop_render($element[$key]);
    $rows[] = $row;
  }
  $output = '';
  $output .= theme('help', array('markup' => $element['#help']));
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= backdrop_render_children($element);
  return $output;
}

/**
 * Returns HTML for a list of recent content.
 *
 * @param $variables
 *   An associative array containing:
 *   - nodes: An array of recent node entities.
 *
 * @ingroup themeable
 */
function theme_node_recent_block($variables) {
  $rows = array();
  $output = '';

  $l_options = array('query' => backdrop_get_destination());
  foreach ($variables['nodes'] as $node) {
    $row = array();
    $row[] = array(
      'data' => theme('node_recent_content', array('node' => $node)),
      'class' => 'title-author',
    );
    $row[] = array(
      'data' => node_access('update', $node) ? l(t('edit'), 'node/' . $node->nid . '/edit', $l_options) : '',
      'class' => 'edit',
    );
    $row[] = array(
      'data' => node_access('delete', $node) ? l(t('delete'), 'node/' . $node->nid . '/delete', $l_options) : '',
      'class' => 'delete',
    );
    $rows[] = $row;
  }

  if ($rows) {
    $output = theme('table', array('rows' => $rows));
    if (user_access('access content overview')) {
      $output .= theme('more_link', array('url' => 'admin/content', 'title' => t('Show more content')));
    }
  }

  return $output;
}

/**
 * Returns HTML for a recent node to be displayed in the recent content block.
 *
 * @param $variables
 *   An associative array containing:
 *   - node: A node entity.
 *
 * @ingroup themeable
 */
function theme_node_recent_content($variables) {
  $node = $variables['node'];

  $output = '<div class="node-title">';
  $output .= l($node->title, 'node/' . $node->nid);
  $output .= theme('mark', array('type' => node_mark($node->nid, $node->changed)));
  $output .= '</div><div class="node-author">';
  $output .= theme('username', array('account' => user_load($node->uid)));
  $output .= '</div>';

  return $output;
}

/**
 * Returns HTML for a list of available node types for node creation.
 *
 * @param $variables
 *   An associative array containing:
 *   - content: An array of content types.
 *
 * @see node_add_page()
 * @ingroup themeable
 */
function theme_node_add_list($variables) {
  $content = $variables['content'];
  $output = '';

  if ($content) {
    $output = '<dl class="node-type-list">';
    foreach ($content as $item) {
      $output .= '<dt>' . l($item['title'], $item['href'], $item['localized_options']) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($item['description']) . '</dd>';
    }
    $output .= '</dl>';
  }
  else {
    $output = '<p>' . t('You have not created any content types yet. Go to the <a href="@create-content">content type creation page</a> to add a new content type.', array('@create-content' => url('admin/structure/types/add'))) . '</p>';
  }
  return $output;
}

/**
 * Returns HTML for a node type description for the content type admin page.
 *
 * @param $variables
 *   An associative array containing:
 *   - name: The human-readable name of the content type.
 *   - type: An object containing the 'type' (machine name) and 'description' of
 *     the content type.
 *
 * @deprecated as of 1.19.4. Labels are now output using the
 * theme_label_machine_name() function, and descriptions are displayed in a
 * separate column.
 *
 * @ingroup themeable
 */
function theme_node_admin_overview($variables) {
  watchdog_deprecated_function('theme', __FUNCTION__);
  $name = $variables['name'];
  $type = $variables['type'];

  $output = check_plain($name);
  $output .= ' <small>' . t('(Machine name: @type)', array('@type' => $type->type)) . '</small>';
  $output .= '<div class="description">' . filter_xss_admin($type->description) . '</div>';
  return $output;
}

/**
 * Processes variables for node.tpl.php.
 *
 * Most themes utilize their own copy of node.tpl.php. The default is located
 * inside "modules/node/node.tpl.php". Look in there for the full list of
 * variables.
 *
 * @param $variables
 *   An array containing the following arguments:
 *   - $node
 *   - $view_mode
 *   - $page
 *
 * @see node.tpl.php
 */
function template_preprocess_node(&$variables) {
  $variables['view_mode'] = $variables['elements']['#view_mode'];
  // Provide a distinct $teaser boolean.
  $variables['teaser'] = $variables['view_mode'] == 'teaser';
  $variables['node'] = $variables['elements']['#node'];
  $node = $variables['node'];

  $variables['date']      = format_date($node->created);
  $variables['name']      = theme('username', array(
    'account' => $node,
    'link_attributes' => array('rel' => 'author'),
  ));

  $uri = entity_uri('node', $node);
  $variables['node_url']  = url($uri['path'], $uri['options']);
  $variables['title']     = check_plain($node->title);
  $variables['page']      = ($variables['view_mode'] == 'full' && node_is_page($node)) || (!empty($node->in_preview));

  // Flatten the node entity's member fields.
  $variables = array_merge((array) $node, $variables);

  // Helpful $content variable for templates.
  $variables += array('content' => array());
  foreach (element_children($variables['elements']) as $key) {
    $variables['content'][$key] = $variables['elements'][$key];
  }

  // Comments are coded directly into the node template, but comment module
  // may not be enabled. Set a variable to ensure it exists. Comment module will
  // add it's own replacements over the top of this.
  $variables['comments'] = FALSE;

  // Make the field variables available with the appropriate language.
  field_attach_preprocess('node', $node, $variables['content'], $variables);

  // Display post information only on certain node types.
  $node_type = node_type_get_type($node->type);

  $variables['display_submitted'] = FALSE;
  $variables['submitted'] = '';
  $variables['user_picture'] = '';

  if ($node_type->settings['node_submitted']) {
    if (!isset($variables['elements']['#display_submitted']) || $variables['elements']['#display_submitted'] == TRUE) {
      $variables['display_submitted'] = TRUE;
      $variables['submitted'] = token_replace($node_type->settings['node_submitted_format'], array('node' => $node));
      $variables['user_picture'] = '';
      if ($node_type->settings['node_user_picture']) {
        $variables['user_picture'] = theme('user_picture', array('account' => $node));
      }
    }
  }

  // Gather node classes.
  $variables['classes'][] = backdrop_html_class('node-' . $node->type);
  if ($variables['promote']) {
    $variables['classes'][] = 'promoted';
  }
  if ($variables['sticky']) {
    $variables['classes'][] = 'sticky';
  }
  if (!$variables['status']) {
    $variables['classes'][] = 'unpublished';
  }
  if ($variables['view_mode']) {
    $variables['classes'][] = backdrop_html_class('view-mode-' . $variables['view_mode']);
  }
  if (!$node->uid) {
    $variables['classes'][] = 'node-by-anonymous';
  }
  elseif ($node->uid == $variables['user']->uid) {
    $variables['classes'][] = 'node-by-viewer';
  }

  // Remove the link to the node if this type has a hidden path.
  if ($node_type->settings['hidden_path'] && !user_access('view hidden paths')) {
    // Empty the $node_url variable so the node title won't be linked.
    $variables['node_url'] = FALSE;
  }

  // Add an extra theme hook suggestion for node id.
  $variables['theme_hook_suggestions'][] = 'node__' . $node->nid;

  // Add a legacy theme hook suggestion for content block. @todo remove in 2.x.
  if (strstr($variables['theme_hook_original'], 'node_block')) {
    $variables['theme_hook_suggestions'][] = 'node__block_' . $node->nid;
  }
}
