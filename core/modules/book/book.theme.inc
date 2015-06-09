<?php
/**
 * @file
 * Theme functions for the Book module.
 */

/**
 * Processes variables for book-all-books-block.tpl.php.
 *
 * All non-renderable elements are removed so that the template has full access
 * to the structured data but can also simply iterate over all elements and
 * render them (as in the default template).
 *
 * @param $variables
 *   An associative array containing the following key:
 *   - book_menus
 *
 * @see book-all-books-block.tpl.php
 */
function template_preprocess_book_all_books_block(&$variables) {
  // Remove all non-renderable elements.
  $elements = $variables['book_menus'];
  $variables['book_menus'] = array();
  foreach (element_children($elements) as $index) {
    $variables['book_menus'][$index] = $elements[$index];
  }
}

/**
 * Processes variables for book-navigation.tpl.php.
 *
 * @param $variables
 *   An associative array containing the following key:
 *   - book_link
 *
 * @see book-navigation.tpl.php
 */
function template_preprocess_book_navigation(&$variables) {
  $book_link = $variables['book_link'];

  // Provide extra variables for themers. Not needed by default.
  $variables['book_id'] = $book_link['bid'];
  $variables['book_title'] = check_plain($book_link['link_title']);
  $variables['book_url'] = 'node/' . $book_link['bid'];
  $variables['current_depth'] = $book_link['depth'];
  $variables['tree'] = '';

  if ($book_link['mlid']) {
    // use book_link to see if we can obtain info about it from cache
    $data = FALSE;
    // generate a unique call signature for bid and mlid
    $cid = $book_link['bid'] . ':' . $book_link['mlid'];
    // statically cache future calls
    $data = &backdrop_static(__FUNCTION__ . $cid);
    // @ignore druplart_conditional_assignment
    if (!$data && $cache = cache_get($cid, 'cache_book')) {
      $data = $cache->data;
    }
    // we got data from a previous source so use that instead
    if ($data) {
      $prev = $data['prev'];
      $next = $data['next'];
      $tree = $data['tree'];
      $parent = $data['parent'];
    }
    else {
      // cache miss, rebuild manually
      $prev = book_prev($book_link);
      $next = book_next($book_link);
      $tree = book_children($book_link);
      $parent = book_link_load($book_link['plid']);
      $data = array(
        'prev' => $prev,
        'next' => $next,
        'tree' => $tree,
        'parent' => $parent,
      );
      cache_set($cid, $data, 'cache_book');
    }
    $variables['tree'] = $tree;
    $variables['book_id'] = $book_link['bid'];
    // check for prev link
    if ($prev) {
      $prev_href = url($prev['href']);
      backdrop_add_html_head_link(array('rel' => 'prev', 'href' => $prev_href));
      $variables['prev_url'] = $prev_href;
      $variables['prev_title'] = check_plain($prev['title']);
    }
    if ($book_link['plid'] && $parent) {
      $parent_href = url($parent['href']);
      backdrop_add_html_head_link(array('rel' => 'up', 'href' => $parent_href));
      $variables['parent_url'] = $parent_href;
      $variables['parent_title'] = check_plain($parent['title']);
    }
    // check for a next link
    if ($next) {
      $next_href = url($next['href']);
      backdrop_add_html_head_link(array('rel' => 'next', 'href' => $next_href));
      $variables['next_url'] = $next_href;
      $variables['next_title'] = check_plain($next['title']);
    }
  }

  $variables['has_links'] = FALSE;
  // Link variables to filter for values and set state of the flag variable.
  $links = array('prev_url', 'prev_title', 'parent_url', 'parent_title', 'next_url', 'next_title');
  foreach ($links as $link) {
    if (isset($variables[$link])) {
      // Flag when there is a value.
      $variables['has_links'] = TRUE;
    }
    else {
      // Set empty to prevent notices.
      $variables[$link] = '';
    }
  }
}

