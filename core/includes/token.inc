<?php
/**
 * @file
 * Backdrop placeholder/token replacement system.
 *
 * API functions for replacing placeholders in text with meaningful values.
 *
 * For example: When configuring automated emails, an administrator enters
 * standard text for the email. Variables like the title of a node and the date
 * the email was sent can be entered as placeholders like [node:title] and
 * [date:short]. When a Backdrop module prepares to send the email, it can call
 * the token_replace() function, passing in the text. The token system will
 * scan the text for placeholder tokens, give other modules an opportunity to
 * replace them with meaningful text, then return the final product to the
 * original module.
 *
 * Tokens follow the form: [$type:$name], where $type is a general class of
 * tokens like 'node', 'user', or 'comment' and $name is the name of a given
 * placeholder. For example, [node:title] or [node:created:since].
 *
 * In addition to raw text containing placeholders, modules may pass in an array
 * of objects to be used when performing the replacement. The objects should be
 * keyed by the token type they correspond to. For example:
 *
 * @code
 * // Load a node and a user, then replace tokens in the text.
 * $text = 'On [date:short], [user:name] read [node:title].';
 * $node = node_load(1);
 * $user = user_load(1);
 *
 * // [date:...] tokens use the current date automatically.
 * $data = array('node' => $node, 'user' => $user);
 * return token_replace($text, $data);
 * @endcode
 *
 * Some tokens may be chained in the form of [$type:$pointer:$name], where $type
 * is a normal token type, $pointer is a reference to another token type, and
 * $name is the name of a given placeholder. For example, [node:author:mail]. In
 * that example, 'author' is a pointer to the 'user' account that created the
 * node, and 'mail' is a placeholder available for any 'user'.
 *
 * @see token_replace()
 * @see hook_tokens()
 * @see hook_token_info()
 */

/**
 * Replaces all tokens in a given string with appropriate values.
 *
 * @param $text
 *   A string potentially containing replaceable tokens.
 * @param $data
 *   (optional) An array of keyed objects. For simple replacement scenarios
 *   'node', 'user', and others are common keys, with an accompanying node or
 *   user object being the value. Some token types, like 'site', do not require
 *   any explicit information from $data and can be replaced even if it is
 *   empty.
 * @param $options
 *   (optional) A keyed array of settings and flags to control the token
 *   replacement process. Supported options are:
 *   - language: A language object to be used when generating locale-sensitive
 *     tokens.
 *   - callback: A callback function that will be used to post-process the array
 *     of token replacements after they are generated. For example, a module
 *     using tokens in a text-only email might provide a callback to strip HTML
 *     entities from token values before they are inserted into the final text.
 *   - clear: A boolean flag indicating that tokens should be removed from the
 *     final text if no replacement value can be generated.
 *   - sanitize: A boolean flag indicating that tokens should be sanitized for
 *     display to a web browser. Defaults to TRUE. Developers who set this
 *     option to FALSE assume responsibility for running filter_xss(),
 *     check_plain() or other appropriate scrubbing functions before displaying
 *     data to users.
 *
 * @return
 *   Text with tokens replaced.
 */
function token_replace($text, array $data = array(), array $options = array()) {
  $text_tokens = token_scan($text);
  if (empty($text_tokens)) {
    return $text;
  }

  $replacements = array();
  foreach ($text_tokens as $type => $tokens) {
    $replacements += token_generate($type, $tokens, $data, $options);
    if (!empty($options['clear'])) {
      $replacements += array_fill_keys($tokens, '');
    }
  }

  // Optionally alter the list of replacement values.
  if (!empty($options['callback'])) {
    $function = $options['callback'];
    $function($replacements, $data, $options);
  }

  $tokens = array_keys($replacements);
  $values = array_values($replacements);

  return str_replace($tokens, $values, $text);
}

/**
 * Builds a list of all token-like patterns that appear in the text.
 *
 * @param $text
 *   The text to be scanned for possible tokens.
 *
 * @return
 *   An associative array of discovered tokens, grouped by type.
 */
function token_scan($text) {
  // Matches tokens with the following pattern: [$type:$name]
  // $type and $name may not contain [ ] characters.
  // $type may not contain : or whitespace characters, but $name may.
  preg_match_all('/
    \[             # [ - pattern start
    ([^\s\[\]:]*)  # match $type not containing whitespace : [ or ]
    :              # : - separator
    ([^\[\]]*)     # match $name not containing [ or ]
    \]             # ] - pattern end
    /x', $text, $matches);

  $types = $matches[1];
  $tokens = $matches[2];

  // Iterate through the matches, building an associative array containing
  // $tokens grouped by $types, pointing to the version of the token found in
  // the source text. For example, $results['node']['title'] = '[node:title]';
  $results = array();
  for ($i = 0; $i < count($tokens); $i++) {
    $results[$types[$i]][$tokens[$i]] = $matches[0][$i];
  }

  return $results;
}

/**
 * Generates replacement values for a list of tokens.
 *
 * @param $type
 *   The type of token being replaced. 'node', 'user', and 'date' are common.
 * @param $tokens
 *   An array of tokens to be replaced, keyed by the literal text of the token
 *   as it appeared in the source text.
 * @param $data
 *   (optional) An array of keyed objects. For simple replacement scenarios
 *   'node', 'user', and others are common keys, with an accompanying node or
 *   user object being the value. Some token types, like 'site', do not require
 *   any explicit information from $data and can be replaced even if it is
 *   empty.
 * @param $options
 *   (optional) A keyed array of settings and flags to control the token
 *   replacement process. Supported options are:
 *   - language: A language object to be used when generating locale-sensitive
 *     tokens.
 *   - callback: A callback function that will be used to post-process the
 *     array of token replacements after they are generated. Can be used when
 *     modules require special formatting of token text, for example URL
 *     encoding or truncation to a specific length.
 *   - sanitize: A boolean flag indicating that tokens should be sanitized for
 *     display to a web browser. Developers who set this option to FALSE assume
 *     responsibility for running filter_xss(), check_plain() or other
 *     appropriate scrubbing functions before displaying data to users.
 *
 * @return
 *   An associative array of replacement values, keyed by the original 'raw'
 *   tokens that were found in the source text. For example:
 *   $results['[node:title]'] = 'My new node';
 *
 * @see hook_tokens()
 * @see hook_tokens_alter()
 */
function token_generate($type, array $tokens, array $data = array(), array $options = array()) {
  $options += array('sanitize' => TRUE);
  $replacements = module_invoke_all('tokens', $type, $tokens, $data, $options);

  // Allow other modules to alter the replacements.
  $context = array(
    'type' => $type,
    'tokens' => $tokens,
    'data' => $data,
    'options' => $options,
  );
  backdrop_alter('tokens', $replacements, $context);

  return $replacements;
}

/**
 * Returns a list of tokens that begin with a specific prefix.
 *
 * Used to extract a group of 'chained' tokens (such as [node:author:name])
 * from the full list of tokens found in text. For example:
 * @code
 *   $data = array(
 *     'author:name' => '[node:author:name]',
 *     'title'       => '[node:title]',
 *     'created'     => '[node:created]',
 *   );
 *   $results = token_find_with_prefix($data, 'author');
 *   $results == array('name' => '[node:author:name]');
 * @endcode
 *
 * @param $tokens
 *   A keyed array of tokens, and their original raw form in the source text.
 * @param $prefix
 *   A textual string to be matched at the beginning of the token.
 * @param $delimiter
 *   An optional string containing the character that separates the prefix from
 *   the rest of the token. Defaults to ':'.
 *
 * @return
 *   An associative array of discovered tokens, with the prefix and delimiter
 *   stripped from the key.
 */
function token_find_with_prefix(array $tokens, $prefix, $delimiter = ':') {
  $results = array();
  foreach ($tokens as $token => $raw) {
    $parts = explode($delimiter, $token, 2);
    if (count($parts) == 2 && $parts[0] == $prefix) {
      $results[$parts[1]] = $raw;
    }
  }
  return $results;
}

/**
 * Returns metadata describing supported tokens.
 *
 * The metadata array contains token type, name, and description data as well
 * as an optional pointer indicating that the token chains to another set of
 * tokens.
 *
 * For example:
 * @code
 *   $data['types']['node'] = array(
 *     'name' => t('Nodes'),
 *     'description' => t('Tokens related to node objects.'),
 *   );
 *   $data['tokens']['node']['title'] = array(
 *     'name' => t('Title'),
 *     'description' => t('The title of the current node.'),
 *   );
 *   $data['tokens']['node']['author'] = array(
 *     'name' => t('Author'),
 *     'description' => t('The author of the current node.'),
 *     'type' => 'user',
 *   );
 * @endcode
 *
 * @return
 *   An associative array of token information, grouped by token type.
 */
function token_info() {
  $data = &backdrop_static(__FUNCTION__);
  if (!isset($data)) {
    $data = module_invoke_all('token_info');
    backdrop_alter('token_info', $data);
  }
  return $data;
}

/**
 * Retrieve, sort, store token info from the cache.
 *
 * @param $token_type
 *   The optional token type that if specified will return
 *   $info['types'][$token_type].
 * @param $token
 *   The optional token name if specified will return
 *   $info['tokens'][$token_type][$token].
 *
 * @return
 *   An array of all token information from hook_token_info(), or the array
 *   of a token type or specific token.
 *
 * @see hook_token_info()
 * @see hook_token_info_alter()
 */
function token_get_info($token_type = NULL, $token = NULL) {
  global $language_content;

  $cache_token = cache('token');

  // Use the advanced backdrop_static() pattern, since this is called very often.
  static $backdrop_static_fast;
  if (!isset($backdrop_static_fast)) {
    $backdrop_static_fast['token_info'] = &backdrop_static(__FUNCTION__);
  }
  $token_info = &$backdrop_static_fast['token_info'];

  if (empty($token_info)) {
    $cid = "info:{$language_content->langcode}";

    if ($cache = $cache_token->get($cid)) {
      $token_info = $cache->data;
    }
    else {
      $token_info = token_info();
      foreach (array_keys($token_info['types']) as $type_key) {
        if (isset($token_info['types'][$type_key]['type'])) {
          $base_type = $token_info['types'][$type_key]['type'];
          // If this token type extends another token type, then merge in
          // the base token type's tokens.
          if (isset($token_info['tokens'][$base_type])) {
            $token_info['tokens'] += array($type_key => array());
            $token_info['tokens'][$type_key] += $token_info['tokens'][$base_type];
          }
        }
        else {
          // Add a 'type' value to each token type so we can properly use
          // token_type_load().
          $token_info['types'][$type_key]['type'] = $type_key;
        }
      }

      // Pre-sort tokens.
      backdrop_sort($token_info['types'], array('name' => SORT_STRING));
      foreach (array_keys($token_info['tokens']) as $type) {
        backdrop_sort($token_info['types'], array('name' => SORT_STRING));
      }

      // Store info in cache for future use.
      $cache_token->set($cid, $token_info);
    }
  }

  if (isset($token_type) && isset($token)) {
    return isset($token_info['tokens'][$token_type][$token]) ? $token_info['tokens'][$token_type][$token] : NULL;
  }
  elseif (isset($token_type)) {
    return isset($token_info['types'][$token_type]) ? $token_info['types'][$token_type] : NULL;
  }
  else {
    return $token_info;
  }
}

/**
 * Get a list of token types that can be used without any context (global).
 *
 * @return
 *   An array of global token types.
 */
function token_get_global_token_types() {
  $global_types = &backdrop_static(__FUNCTION__, array());

  if (empty($global_types)) {
    $token_info = token_get_info();
    foreach ($token_info['types'] as $type => $type_info) {
      // If the token types has not specified that 'needs-data' => TRUE, then
      // it is a global token type that will always be replaced in any context.
      if (empty($type_info['needs-data'])) {
        $global_types[] = $type;
      }
    }
  }

  return $global_types;
}

/**
 * Clear token caches and static variables.
 */
function token_clear_cache() {
  if (db_table_exists('cache_token')) {
    cache_clear_all('*', 'cache_token', TRUE);
  }
  backdrop_static_reset('token_get_info');
  backdrop_static_reset('token_get_global_token_types');
  backdrop_static_reset('token_build_tree');
  backdrop_static_reset('token_menu_link_load');
  backdrop_static_reset('token_book_link_load');
  backdrop_static_reset('token_node_menu_link_load');
  backdrop_static_reset('_token_field_info');
}

/**
 * Get token problems.
 */
function token_get_token_problems() {
  $token_info = token_info();
  $token_problems = array(
    'not-array' => array(
      'label' => t('The following tokens or token types are not defined as arrays:'),
    ),
    'missing-info' => array(
      'label' => t('The following tokens or token types are missing required name and/or description information:'),
    ),
    'type-no-tokens' => array(
      'label' => t('The following token types do not have any tokens defined:'),
    ),
    'tokens-no-type' => array(
      'label' => t('The following token types are not defined but have tokens:'),
    ),
    'duplicate' => array(
      'label' => t('The following token or token types are defined by multiple modules:')
    ),
  );

  // Check token types for problems.
  foreach ($token_info['types'] as $type => $type_info) {
    $real_type = !empty($type_info['type']) ? $type_info['type'] : $type;
    if (!is_array($type_info)) {
      $token_problems['not-array']['problems'][] = "\$info['types']['$type']";
      continue;
    }
    elseif (!isset($type_info['name']) || !isset($type_info['description'])) {
      $token_problems['missing-info']['problems'][] = "\$info['types']['$type']";
    }
    elseif (empty($token_info['tokens'][$real_type])) {
      $token_problems['type-no-tokens']['problems'][] = "\$info['tokens']['$real_type']";
    }
  }

  // Check tokens for problems.
  foreach ($token_info['tokens'] as $type => $tokens) {
    if (!is_array($tokens)) {
      $token_problems['not-array']['problems'][] = "\$info['tokens']['$type']";
      continue;
    }
    else {
      foreach (array_keys($tokens) as $token) {
        if (!is_array($tokens[$token])) {
          $token_problems['not-array']['problems'][] = "\$info['tokens']['$type']['$token']";
          continue;
        }
        elseif (!isset($tokens[$token]['name']) || !isset($tokens[$token]['description'])) {
          $token_problems['missing-info']['problems'][] = "\$info['tokens']['$type']['$token']";
        }
        elseif (is_array($tokens[$token]['name']) || is_array($tokens[$token]['description'])) {
          $token_problems['duplicate']['problems'][] = "\$info['tokens']['$type']['$token']";
        }
      }
    }
    if (!isset($token_info['types'][$type])) {
      $token_problems['tokens-no-type']['problems'][] = "\$info['types']['$type']";
    }
  }

  return $token_problems;
}

/**
 * Loads a token type.
 *
 * @param $token_type
 *   A token type name.
 *
 * @return
 *  An array representing the token type. FALSE if no type found.
 */
function token_type_load($token_type) {
  $info = token_get_info();
  return isset($info['types'][$token_type]) ? $info['types'][$token_type] : FALSE;
}

/**
 * Build a tree array of tokens used for theming or information.
 *
 * @param $token_type
 *   The token type.
 *
 * @param $flat_tree
 *   A boolean if TRUE will only make a flat array of tokens, otherwise
 *   child tokens will be inside the 'children' parameter of a token.
 * @param $show_restricted
 *   A boolean if TRUE will show restricted tokens. Otherwise they will be
 *   hidden. Default is FALSE.
 * @param $recursion_limit
 *   An integer with the maximum number of token levels to recurse.
 * @param $parents
 *   An optional array with the current parents of the tokens.
 */
function token_build_tree($token_type, array $options = array()) {
  global $language_content;

  $cache_token = cache('token');

  // Static cache of already built token trees.
  $trees = &backdrop_static(__FUNCTION__, array());

  $options += array(
    'restricted' => FALSE,
    'depth' => 4,
    'data' => array(),
    'values' => FALSE,
    'flat' => FALSE,
  );

  // Do not allow past the maximum token information depth.
  $options['depth'] = min($options['depth'], TOKEN_MAX_DEPTH);

  $tree_cid = "tree:{$token_type}:{$language_content->langcode}:{$options['depth']}";

  // If we do not have this base tree in the static cache, check {cache_token}
  // otherwise generate and store it in the cache.
  if (!isset($trees[$tree_cid])) {
    if ($cache = $cache_token->get($tree_cid)) {
      $trees[$tree_cid] = $cache->data;
    }
    else {
      $options['parents'] = array();
      $trees[$tree_cid] = _token_build_tree($token_type, $options);
      cache_set($tree_cid, $trees[$tree_cid], 'cache', CACHE_PERMANENT);
    }
  }

  $tree = $trees[$tree_cid];

  // If the user has requested a flat tree, convert it.
  if (!empty($options['flat'])) {
    $tree = token_flatten_tree($tree);
  }

  // Fill in token values.
  if (!empty($options['values'])) {
    $token_values = array();
    foreach ($tree as $token => $token_info) {
      if (!empty($token_info['dynamic']) || !empty($token_info['restricted'])) {
        continue;
      }
      elseif (!isset($token_info['value'])) {
        $token_values[$token_info['token']] = $token;
      }
    }
    if (!empty($token_values)) {
      $token_values = token_generate($token_type, $token_values, $options['data']);
      foreach ($token_values as $token => $replacement) {
        $tree[$token]['value'] = $replacement;
      }
    }
  }

  return $tree;
}

/**
 * Flatten a token tree.
 */
function token_flatten_tree($tree) {
  $result = array();
  foreach ($tree as $token => $token_info) {
    $result[$token] = $token_info;
    if (isset($token_info['children']) && is_array($token_info['children'])) {
      $result += token_flatten_tree($token_info['children']);
      // unset($result[$token]['children']);
    }
  }
  return $result;
}

/**
 * Generate a token tree.
 */
function _token_build_tree($token_type, array $options) {
  $options += array(
    'parents' => array(),
  );

  $info = token_get_info();
  if ($options['depth'] <= 0 || !isset($info['types'][$token_type]) || !isset($info['tokens'][$token_type])) {
    return array();
  }

  $tree = array();
  foreach ($info['tokens'][$token_type] as $token => $token_info) {
    // Build the raw token string.
    $token_parents = $options['parents'];
    if (empty($token_parents)) {
      // If the parents array is currently empty, assume the token type is its
      // parent.
      $token_parents[] = $token_type;
    }
    elseif (in_array($token, array_slice($token_parents, 1))) {
      // Prevent duplicate recursive tokens. For example, this will prevent
      // the tree from generating the following tokens or deeper:
      // [comment:parent:parent]
      // [comment:parent:root:parent]
      continue;
    }

    $token_parents[] = $token;
    if (!empty($token_info['dynamic'])) {
      $token_parents[] = '?';
    }
    $raw_token = '[' . implode(':', $token_parents) . ']';
    $tree[$raw_token] = $token_info;
    $tree[$raw_token]['raw token'] = $raw_token;

    // Add the token's real name (leave out the base token type).
    $tree[$raw_token]['token'] = implode(':', array_slice($token_parents, 1));

    // Add the token's parent as its raw token value.
    if (!empty($options['parents'])) {
      $tree[$raw_token]['parent'] = '[' . implode(':', $options['parents']) . ']';
    }

    // Fetch the child tokens.
    if (!empty($token_info['type'])) {
      $child_options = $options;
      $child_options['depth']--;
      $child_options['parents'] = $token_parents;
      $tree[$raw_token]['children'] = _token_build_tree($token_info['type'], $child_options);
    }
  }

  return $tree;
}

/**
 * Validate a form element that should have tokens in it.
 *
 * Form elements that want to add this validation should have the #token_types
 * parameter defined.
 *
 * For example:
 * @code
 * $form['my_node_text_element'] = array(
 *   '#type' => 'textfield',
 *   '#title' => t('Some text to token-ize that has a node context.'),
 *   '#default_value' => 'The title of this node is [node:title].',
 *   '#element_validate' => array('token_element_validate'),
 *   '#token_types' => array('node'),
 *   '#min_tokens' => 1,
 *   '#max_tokens' => 10,
 * );
 * @endcode
 */
function token_element_validate(&$element, &$form_state) {
  $value = isset($element['#value']) ? $element['#value'] : $element['#default_value'];

  if (!backdrop_strlen($value)) {
    // Empty value needs no further validation since the element should depend
    // on using the '#required' FAPI property.
    return $element;
  }

  $tokens = token_scan($value);
  $title = empty($element['#title']) ? $element['#parents'][0] : $element['#title'];
  // @todo Find old Drupal 6 style tokens and add them to invalid tokens.

  // Validate if an element must have a minimum number of tokens.
  if (isset($element['#min_tokens']) && count($tokens) < $element['#min_tokens']) {
    // @todo Change this error message to include the minimum number.
    $error = format_plural($element['#min_tokens'], 'The %element-title cannot contain fewer than one token.', 'The %element-title must contain at least @count tokens.', array('%element-title' => $title));
    form_error($element, $error);
  }

  // Validate if an element must have a maximum number of tokens.
  if (isset($element['#max_tokens']) && count($tokens) > $element['#max_tokens']) {
    // @todo Change this error message to include the maximum number.
    $error = format_plural($element['#max_tokens'], 'The %element-title must contain as most one token.', 'The %element-title must contain at most @count tokens.', array('%element-title' => $title));
    form_error($element, $error);
  }

  // Check if the field defines specific token types.
  if (isset($element['#token_types'])) {
    $invalid_tokens = token_get_invalid_tokens_by_context($tokens, $element['#token_types']);
    if ($invalid_tokens) {
      form_error($element, t('The %element-title is using the following invalid tokens: @invalid-tokens.', array('%element-title' => $title, '@invalid-tokens' => implode(', ', $invalid_tokens))));
    }
  }

  return $element;
}

/**
 * Prepare a string for use as a valid token name.
 *
 * @param $name
 *   The token name to clean.
 * @return
 *   The cleaned token name.
 */
function token_clean_token_name($name) {
  static $names = array();

  if (!isset($names[$name])) {
    $cleaned_name = strtr($name, array(' ' => '-', '_' => '-', '/' => '-', '[' => '-', ']' => ''));
    $cleaned_name = preg_replace('/[^\w\-]/i', '', $cleaned_name);
    $cleaned_name = trim($cleaned_name, '-');
    $names[$name] = $cleaned_name;
  }

  return $names[$name];
}

/**
 * Validate an tokens in raw text based on possible contexts.
 *
 * @param $value
 *   A string with the raw text containing the raw tokens, or an array of
 *   tokens from token_scan().
 * @param $tokens
 *   An array of token types that will be used when token replacement is
 *   performed.
 * @return
 *   An array with the invalid tokens in their original raw forms.
 */
function token_get_invalid_tokens_by_context($value, $valid_types = array()) {
  if (in_array('all', $valid_types)) {
    $info = token_get_info();
    $valid_types = array_keys($info['types']);
  }
  else {
    // Add the token types that are always valid in global context.
    $valid_types = array_merge($valid_types, token_get_global_token_types());
  }

  $invalid_tokens = array();
  $value_tokens = is_string($value) ? token_scan($value) : $value;

  foreach ($value_tokens as $type => $tokens) {
    if (!in_array($type, $valid_types)) {
      // If the token type is not a valid context, its tokens are invalid.
      $invalid_tokens = array_merge($invalid_tokens, array_values($tokens));
    }
    else {
      // Check each individual token for validity.
      $invalid_tokens = array_merge($invalid_tokens, token_get_invalid_tokens($type, $tokens));
    }
  }

  array_unique($invalid_tokens);
  return $invalid_tokens;
}

/**
 * Validate an array of tokens based on their token type.
 *
 * @param $type
 *   The type of tokens to validate (e.g. 'node', etc.)
 * @param $tokens
 *   A keyed array of tokens, and their original raw form in the source text.
 * @return
 *   An array with the invalid tokens in their original raw forms.
 */
function token_get_invalid_tokens($type, $tokens) {
  $token_info = token_get_info();
  $invalid_tokens = array();

  foreach ($tokens as $token => $full_token) {
    // Split token up if it has chains.
    $parts = explode(':', $token, 2);

    if (!isset($token_info['tokens'][$type][$parts[0]])) {
      // This is an invalid token (not defined).
      $invalid_tokens[] = $full_token;
    }
    elseif (count($parts) == 2) {
      $sub_token_info = $token_info['tokens'][$type][$parts[0]];
      if (!empty($sub_token_info['dynamic'])) {
        // If this token has been flagged as a dynamic token, skip it.
        continue;
      }
      elseif (empty($sub_token_info['type'])) {
        // If the token has chains, but does not support it, it is invalid.
        $invalid_tokens[] = $full_token;
      }
      else {
        // Resursively check the chained tokens.
        $sub_tokens = token_find_with_prefix(array($token => $full_token), $parts[0]);
        $invalid_tokens = array_merge($invalid_tokens, token_get_invalid_tokens($sub_token_info['type'], $sub_tokens));
      }
    }
  }

  return $invalid_tokens;
}

/**
 * Get a translated menu link by its mlid, without access checking.
 *
 * This function is a copy of menu_link_load() but with its own cache and a
 * simpler query to load the link. This also skips normal menu link access
 * checking by using _token_menu_link_translate().
 *
 * @param $mlid
 *   The mlid of the menu item.
 *
 * @return
 *   A menu link translated for rendering.
 *
 * @see menu_link_load()
 * @see _token_menu_link_translate()
 */
function token_menu_link_load($mlid) {
  $cache = &backdrop_static(__FUNCTION__, array());

  if (!is_numeric($mlid)) {
    return FALSE;
  }

  if (!isset($cache[$mlid])) {
    $item = db_query("SELECT * FROM {menu_links} ml LEFT JOIN {menu_router} m ON m.path = ml.router_path WHERE ml.mlid = :mlid", array(':mlid' => $mlid))->fetchAssoc();
    if (!empty($item)) {
      _token_menu_link_translate($item);
    }
    $cache[$mlid] = $item;
  }

  return $cache[$mlid];
}

function _token_menu_link_translate(&$item) {
  $map = array();

  if (!is_array($item['options'])) {
    $item['options'] = unserialize($item['options']);
  }

  if ($item['external']) {
    $item['access'] = 1;
    $item['href'] = $item['link_path'];
    $item['title'] = $item['link_title'];
    $item['localized_options'] = $item['options'];
  }
  else {
    // Complete the path of the menu link with elements from the current path,
    // if it contains dynamic placeholders (%).
    $map = explode('/', $item['link_path']);
    if (strpos($item['link_path'], '%') !== FALSE) {
      // Invoke registered to_arg callbacks.
      if (!empty($item['to_arg_functions'])) {
        _menu_link_map_translate($map, $item['to_arg_functions']);
      }
    }
    $item['href'] = implode('/', $map);

    // Skip links containing untranslated arguments.
    if (strpos($item['href'], '%') !== FALSE) {
      $item['access'] = FALSE;
      return FALSE;
    }

    $item['access'] = TRUE;
    _menu_item_localize($item, $map, TRUE);
  }

  // Allow other customizations - e.g. adding a page-specific query string to the
  // options array. For performance reasons we only invoke this hook if the link
  // has the 'alter' flag set in the options array.
  if (!empty($item['options']['alter'])) {
    backdrop_alter('translated_menu_link', $item, $map);
  }

  return $map;
}

/**
 * Load the preferred menu link associated with a node.
 *
 * @param $node
 *   A node object.
 *
 * @return
 *   A menu link array from token_menu_link_load() or FALSE if a menu link was
 *   not found.
 *
 * @see menu_node_prepare()
 * @see token_menu_link_load()
 */
function token_node_menu_link_load($node) {
  $cache = &backdrop_static(__FUNCTIon__, array());

  if (!isset($cache[$node->nid])) {
    $mlid = FALSE;

    // Nodes do not have their menu links loaded via menu_node_load().
    if (!isset($node->menu)) {
      // We need to clone the node as menu_node_prepare() may cause data loss.
      // @see http://drupal.org/node/1317926
      $menu_node = clone $node;
      menu_node_prepare($menu_node);
      $mlid = !empty($menu_node->menu['mlid']) ? $menu_node->menu['mlid'] : FALSE;
    }
    else {
      $mlid = !empty($node->menu['mlid']) ? $node->menu['mlid'] : FALSE;
    }

    $cache[$node->nid] = $mlid;
  }

  return $cache[$node->nid] ? token_menu_link_load($cache[$node->nid]) : FALSE;
}

/**
 * Renders a render array for display as a token.
 *
 * @param $array
 *   Render array.
 * @param $options
 *   Array of options for rendering.  Values can include:
 *   - sanitize: Runs check_plain() on the rendered output.
 *   - join: Joins rendered items together with a comma.
 *
 * @return
 *   HTML of rendered item.
 *
 * @see token_render_array_value()
 */
function token_render_array(array $array, array $options = array()) {
  $rendered = array();
  foreach ($array as $key => $value) {
    if ($key[0] === '#') {
      continue;
    }
    $rendered[] = is_array($value) ? render($value) : (string) $value;
  }
  if (!empty($options['sanitize'])) {
    $rendered = array_map('check_plain', $rendered);
  }
  $join = isset($options['join']) ? $options['join'] : ', ';
  return implode($join, $rendered);
}

/**
 * Renders a render array for display as a token.
 *
 * @param $value
 *   Value OR Render array.
 * @param $options
 *   Array of options for rendering.  Values can include:
 *   - sanitize: Runs check_plain() on the rendered output.
 *
 * @return
 *   HTML of rendered item.
 *
 * @see token_render_array_value()
 */
function token_render_array_value($value, array $options = array()) {
  $rendered = is_array($value) ? render($value) : (string) $value;
  if (!empty($options['sanitize'])) {
    $rendered = check_plain($rendered);
  }
  return $rendered;
}

/**
 * Load all menu parents given a Menu Link ID.
 *
 * @param $mlid
 *   Menu Link ID.
 *
 * @return
 *   Array of link titles keyed by menu link ID.
 */
function token_menu_link_load_all_parents($mlid) {
  $cache = &backdrop_static(__FUNCTION__, array());

  if (!is_numeric($mlid)) {
    return array();
  }

  if (!isset($cache[$mlid])) {
    $cache[$mlid] = array();
    $plid = db_query("SELECT plid FROM {menu_links} WHERE mlid = :mlid", array(':mlid' => $mlid))->fetchField();
    while ($plid && $parent = token_menu_link_load($plid)) {
      $cache[$mlid] = array($plid => $parent['title']) + $cache[$mlid];
      $plid = $parent['plid'];
    }
  }

  return $cache[$mlid];
}

/**
 * Load all term parents given a Taxonomy term ID.
 *
 * @param $tid
 *   Taxonomy term ID.
 *
 * @return
 *   Array of taxonomy term names keyed by term ID.
 */
function token_taxonomy_term_load_all_parents($tid) {
  $cache = &backdrop_static(__FUNCTION__, array());

  if (!is_numeric($tid)) {
    return array();
  }

  if (!isset($cache[$tid])) {
    $cache[$tid] = array();
    $parents = taxonomy_term_load_parents_all($tid);
    array_shift($parents); // Remove this term from the array.
    $parents = array_reverse($parents);
    foreach ($parents as $term) {
      $cache[$tid][$term->tid] = entity_label('taxonomy_term', $term);
    }
  }

  return $cache[$tid];
}

/**
 * Return the module responsible for a token.
 *
 * @param string $type
 *   The token type.
 * @param string $name
 *   The token name.
 *
 * @return mixed
 *   The value of $info['tokens'][$type][$name]['module'] from token_get_info(),
 *   or NULL if the value does not exist.
 */
function _token_module($type, $name) {
  $token_info = token_get_info($type, $name);
  return isset($token_info['module']) ? $token_info['module'] : NULL;
}
