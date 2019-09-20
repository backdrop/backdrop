<?php
/**
 * @file
 * Hooks provided by the Search module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define a custom search type.
 *
 * This hook allows a module to tell search.module that it wishes to perform
 * searches on content it defines (custom node types, users, or comments for
 * example) when a site search is performed.
 *
 * In order for the search to do anything, your module must also implement
 * hook_search_execute(), which is called when someone requests a search
 * on your module's type of content. If you want to have your content
 * indexed in the standard search index, your module should also implement
 * hook_update_index(). If your search type has settings, you can implement
 * hook_search_admin() to add them to the search settings page. You can use
 * hook_form_FORM_ID_alter(), with FORM_ID set to 'search_form', to add fields
 * to the search form (see node_form_search_form_alter() for an example).
 * You can use hook_search_access() to limit access to searching,
 * and hook_search_page() to override how search results are displayed.
 *
 * @return
 *   Array with optional keys:
 *   - title: Title for the tab on the search page for this module. Title must
 *     be untranslated, but to register it within translation database pass it
 *     through the t() function separately.
 *   - path: Path component after 'search/' for searching with this module.
 *     Defaults to the module name if not given.
 *   - conditions_callback: An implementation of callback_search_conditions().
 *
 * @ingroup search
 */
function hook_search_info() {
  // If the title string 'Content' will need to be translated, register it in
  // the translation system by calling t() before returning the untranslated
  // string below.
  t('Content');

  return array(
    'title' => 'Content',
    'path' => 'node',
    'conditions_callback' => 'callback_search_conditions',
  );
}

/**
 * Define access to a custom search routine.
 *
 * This hook allows a module to define permissions for a search tab.
 *
 * @ingroup search
 */
function hook_search_access() {
  return user_access('access content');
}

/**
 * Take action when the search index is going to be rebuilt.
 *
 * Modules that use hook_update_index() should update their indexing
 * bookkeeping so that it starts from scratch the next time
 * hook_update_index() is called.
 *
 * @ingroup search
 */
function hook_search_reset() {
  db_update('search_dataset')
    ->fields(array('reindex' => REQUEST_TIME))
    ->condition('type', 'node')
    ->execute();
}

/**
 * Report the status of indexing.
 *
 * The core search module only invokes this hook on active modules.
 * Implementing modules do not need to check whether they are active when
 * calculating their return values.
 *
 * @return
 *  An associative array with the key-value pairs:
 *  - 'remaining': The number of items left to index.
 *  - 'total': The total number of items to index.
 *
 * @ingroup search
 */
function hook_search_status() {
  $total = db_query('SELECT COUNT(*) FROM {node} WHERE status = 1')->fetchField();
  $remaining = db_query("SELECT COUNT(*) FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE n.status = 1 AND d.sid IS NULL OR d.reindex <> 0")->fetchField();
  return array('remaining' => $remaining, 'total' => $total);
}

/**
 * Add elements to the search settings form.
 *
 * @return
 *   Form array for the Search settings page at admin/config/search/settings.
 *
 * @ingroup search
 */
function hook_search_admin() {
  // Output form for defining rank factor weights.
  $form['content_ranking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content ranking'),
  );
  $form['content_ranking']['#theme'] = 'node_search_admin';
  $form['content_ranking']['info'] = array(
    '#markup' => '<p><em>' . t('Influence is a numeric multiplier used in ordering search results. A higher number means the corresponding factor has more influence on search results; zero means the factor is ignored. Changing these numbers does not require the search index to be rebuilt. Changes take effect immediately.') . '</em></p>'
  );

  // Note: reversed to reflect that higher number = higher ranking.
  $config = config('search.settings');
  $options = backdrop_map_assoc(range(0, 10));
  foreach (module_invoke_all('ranking') as $var => $values) {
    $form['content_ranking']['factors']['node_rank_' . $var] = array(
      '#title' => $values['title'],
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $config->get('node_rank_' . $var),
    );
  }
  return $form;
}

/**
 * Execute a search for a set of key words.
 *
 * Use database API with the 'PagerDefault' query extension to perform your
 * search.
 *
 * If your module uses hook_update_index() and search_index() to index its
 * items, use table 'search_index' aliased to 'i' as the main table in your
 * query, with the 'SearchQuery' extension. You can join to your module's table
 * using the 'i.sid' field, which will contain the $sid values you provided to
 * search_index(). Add the main keywords to the query by using method
 * searchExpression(). The functions search_expression_extract() and
 * search_expression_insert() may also be helpful for adding custom search
 * parameters to the search expression.
 *
 * See node_search_execute() for an example of a module that uses the search
 * index, and user_search_execute() for an example that doesn't use the search
 * index.
 *
 * @param $keys
 *   The search keywords as entered by the user.
 * @param $conditions
 *   An optional array of additional conditions, such as filters.
 *
 * @return
 *   An array of search results. To use the default search result
 *   display, each item should have the following keys':
 *   - 'link': Required. The URL of the found item.
 *   - 'type': The type of item (such as the content type).
 *   - 'title': Required. The name of the item.
 *   - 'user': The author of the item.
 *   - 'date': A timestamp when the item was last modified.
 *   - 'extra': An array of optional extra information items.
 *   - 'snippet': An excerpt or preview to show with the result (can be
 *     generated with search_excerpt()).
 *   - 'language': Language code for the item (usually two characters).
 *
 * @ingroup search
 */
function hook_search_execute($keys = NULL, $conditions = NULL) {
  // Build matching conditions
  $query = db_select('search_index', 'i', array('target' => 'slave'))->extend('SearchQuery')->extend('PagerDefault');
  $query->join('node', 'n', 'n.nid = i.sid');
  $query
    ->condition('n.status', 1)
    ->addTag('node_access')
    ->searchExpression($keys, 'node');

  // Insert special keywords.
  $query->setOption('type', 'n.type');
  $query->setOption('language', 'n.language');
  if ($query->setOption('term', 'ti.tid')) {
    $query->join('taxonomy_index', 'ti', 'n.nid = ti.nid');
  }
  // Only continue if the first pass query matches.
  if (!$query->executeFirstPass()) {
    return array();
  }

  // Add the ranking expressions.
  _node_rankings($query);

  // Load results.
  $find = $query
    ->limit(10)
    ->execute();
  $results = array();
  foreach ($find as $item) {
    // Build the node body.
    $node = node_load($item->sid);
    node_build_content($node, 'search_result');
    $node->body = backdrop_render($node->content);

    // Fetch comments for snippet.
    $node->rendered .= ' ' . module_invoke('comment', 'node_update_index', $node);
    // Fetch terms for snippet.
    $node->rendered .= ' ' . module_invoke('taxonomy', 'node_update_index', $node);

    $extra = module_invoke_all('node_search_result', $node);

    $results[] = array(
      'link' => url('node/' . $item->sid, array('absolute' => TRUE)),
      'type' => check_plain(node_type_get_name($node)),
      'title' => $node->title,
      'user' => theme('username', array('account' => $node)),
      'date' => $node->changed,
      'node' => $node,
      'extra' => $extra,
      'score' => $item->calculated_score,
      'snippet' => search_excerpt($keys, $node->body),
    );
  }
  return $results;
}

/**
 * Override the rendering of search results.
 *
 * A module that implements hook_search_info() to define a type of search may
 * implement this hook in order to override the default markup of search
 * results.
 *
 * @param $results
 *   An array of search results.
 *
 * @return
 *   A renderable array, which will render the formatted search results with a
 *   pager included.
 *
 * @see search-results.tpl.php
 */
function hook_search_page($results) {
  return array(
    '#prefix' => '<h2>Test page text is here</h2>',
    '#theme' => 'search_results',
    '#results' => $results,
    '#module' => 'search_extra_type',
  );
}

/**
 * Preprocess text for search.
 *
 * This hook is called to preprocess both the text added to the search index and
 * the keywords users have submitted for searching.
 *
 * Possible uses:
 * - Adding spaces between words of Chinese or Japanese text.
 * - Stemming words down to their root words to allow matches between, for
 *   instance, walk, walked, walking, and walks in searching.
 * - Expanding abbreviations and acronymns that occur in text.
 *
 * @param $text
 *   The text to preprocess. This is a single piece of plain text extracted
 *   from between two HTML tags or from the search query. It will not contain
 *   any HTML entities or HTML tags.
 *
 * @return
 *   The text after preprocessing. Note that if your module decides not to alter
 *   the text, it should return the original text. Also, after preprocessing,
 *   words in the text should be separated by a space.
 *
 * @ingroup search
 */
function hook_search_preprocess($text) {
  // Do processing on $text
  return $text;
}

/**
 * Update the search index for this module.
 *
 * This hook is called every cron run if search.module is enabled, your
 * module has implemented hook_search_info(), and your module has been set as
 * an active search module on the Search settings page
 * (admin/config/search/settings). It allows your module to add items to the
 * built-in search index using search_index(), or to add them to your module's
 * own indexing mechanism.
 *
 * When implementing this hook, your module should index content items that
 * were modified or added since the last run. PHP has a time limit
 * for cron, though, so it is advisable to limit how many items you index
 * per run using config_get('search.settings', 'search_cron_limit') (see
 * example below). Also, since the cron run could time out and abort in the
 * middle of your run, you should update your module's internal bookkeeping on
 * when items have last been indexed as you go rather than waiting to the end
 * of indexing.
 *
 * @ingroup search
 */
function hook_update_index() {
  $limit = (int)config_get('search.settings', 'search_cron_limit');

  $result = db_query_range("SELECT n.nid FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, $limit);

  foreach ($result as $node) {
    $node = node_load($node->nid);

    // Save the changed time of the most recent indexed node, for the search
    // results half-life calculation.
    state_set('node_cron_last', $node->changed);

    // Render the node.
    node_build_content($node, 'search_index');
    $node->rendered = backdrop_render($node->content);

    $text = '<h1>' . check_plain($node->title) . '</h1>' . $node->rendered;

    // Fetch extra data normally not visible
    $extra = module_invoke_all('node_update_index', $node);
    foreach ($extra as $t) {
      $text .= $t;
    }

    // Update index
    search_index($node->nid, 'node', $text);
  }
}
/**
 * @} End of "addtogroup hooks".
 */

/**
 * Provide search query conditions.
 *
 * Callback for hook_search_info().
 *
 * This callback is invoked by search_view() to get an array of additional
 * search conditions to pass to search_data(). For example, a search module
 * may get additional keywords, filters, or modifiers for the search from
 * the query string.
 *
 * This example pulls additional search keywords out of the $_REQUEST variable,
 * (i.e. from the query string of the request). The conditions may also be
 * generated internally - for example based on a module's settings.
 *
 * @param $keys
 *   The search keywords string.
 *
 * @return
 *   An array of additional conditions, such as filters.
 *
 * @ingroup callbacks
 * @ingroup search
 */
function callback_search_conditions($keys) {
  $conditions = array();

  if (!empty($_REQUEST['keys'])) {
    $conditions['keys'] = $_REQUEST['keys'];
  }
  if (!empty($_REQUEST['sample_search_keys'])) {
    $conditions['sample_search_keys'] = $_REQUEST['sample_search_keys'];
  }
  if ($force_keys = config('sample_search.settings')->get('force_keywords')) {
    $conditions['sample_search_force_keywords'] = $force_keys;
  }
  return $conditions;
}
