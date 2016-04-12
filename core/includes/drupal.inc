<?php
/**
 * @file
 * Contains constants and function wrappers for Drupal 7.x compatibility.
 */

/**
 * @defgroup drupal_compatibility Drupal compatibility layer.
 * @{
 * These functions and classes are wrappers around Backdrop functions that allow
 * for Drupal 7 modules to work with Backdrop easily. All of these functions
 * are made available if the "backdrop_drupal_compatibility" option is enabled
 * in your site's settings.php file.
 */

/**
 * The current system version.
 */
define('VERSION', '7.0-dev');

/**
 * Core API compatibility.
 */
define('DRUPAL_CORE_COMPATIBILITY', '7.x');

// Constants from bootstrap.inc.
define('DRUPAL_ROOT', BACKDROP_ROOT);
define('DRUPAL_BOOTSTRAP_CONFIGURATION', BACKDROP_BOOTSTRAP_CONFIGURATION);
define('DRUPAL_BOOTSTRAP_PAGE_CACHE', BACKDROP_BOOTSTRAP_PAGE_CACHE);
define('DRUPAL_BOOTSTRAP_DATABASE', BACKDROP_BOOTSTRAP_DATABASE);
define('DRUPAL_BOOTSTRAP_VARIABLES', BACKDROP_BOOTSTRAP_VARIABLES);
define('DRUPAL_BOOTSTRAP_SESSION', BACKDROP_BOOTSTRAP_SESSION);
define('DRUPAL_BOOTSTRAP_PAGE_HEADER', BACKDROP_BOOTSTRAP_PAGE_HEADER);
define('DRUPAL_BOOTSTRAP_LANGUAGE', BACKDROP_BOOTSTRAP_LANGUAGE);
define('DRUPAL_BOOTSTRAP_FULL', BACKDROP_BOOTSTRAP_FULL);
define('DRUPAL_ANONYMOUS_RID', BACKDROP_ANONYMOUS_ROLE);
define('DRUPAL_AUTHENTICATED_RID', BACKDROP_AUTHENTICATED_ROLE);
define('DRUPAL_KILOBYTE', BACKDROP_KILOBYTE);
define('DRUPAL_PHP_FUNCTION_PATTERN', BACKDROP_PHP_FUNCTION_PATTERN);
define('DRUPAL_MINIMUM_PHP', BACKDROP_MINIMUM_PHP);
define('DRUPAL_MINIMUM_PHP_MEMORY_LIMIT', BACKDROP_MINIMUM_PHP_MEMORY_LIMIT);

// Constants from common.inc.
define('DRUPAL_NO_CACHE', -1);
define('DRUPAL_CACHE_CUSTOM', -2);
define('DRUPAL_CACHE_PER_ROLE', 0x0001);
define('DRUPAL_CACHE_PER_USER', 0x0002);
define('DRUPAL_CACHE_PER_PAGE', 0x0004);
define('DRUPAL_CACHE_GLOBAL', 0x0008);

// Constants from password.inc.
define('DRUPAL_HASH_COUNT', 16);
define('DRUPAL_MIN_HASH_COUNT', 7);
define('DRUPAL_MAX_HASH_COUNT', 30);
define('DRUPAL_HASH_LENGTH', 55);

// Constants from system.module.
define('DRUPAL_MAXIMUM_TEMP_FILE_AGE', 21600);
define('DRUPAL_CRON_DEFAULT_THRESHOLD', 10800);
define('DRUPAL_USER_TIMEZONE_DEFAULT', 0);
define('DRUPAL_USER_TIMEZONE_EMPTY', 1);
define('DRUPAL_USER_TIMEZONE_SELECT', 2);
define('DRUPAL_DISABLED', 0);
define('DRUPAL_OPTIONAL', 1);
define('DRUPAL_REQUIRED', 2);
define('DRUPAL_WEIGHT_SELECT_MAX', 100);

// Functions from bootstrap.inc.
function drupal_override_server_variables($variables = array()) {
  return backdrop_override_server_variables($variables);
}
function drupal_environment_initialize() {
  return backdrop_environment_initialize();
}
function drupal_valid_http_host($host) {
  return backdrop_valid_http_host($host);
}
function drupal_settings_initialize() {
  return backdrop_settings_initialize();
}
function drupal_get_filename($type, $name, $filename = NULL) {
  return backdrop_get_filename($type, $name, $filename);
}
function drupal_page_get_cache($check_only = FALSE) {
  return backdrop_page_get_cache($check_only);
}
function drupal_page_is_cacheable($allow_caching = NULL) {
  return backdrop_page_is_cacheable($allow_caching);
}
function drupal_load($type, $name) {
  return backdrop_load($type, $name);
}
function drupal_add_http_header($name, $value, $append = FALSE) {
  return backdrop_add_http_header($name, $value, $append);
}
function drupal_get_http_header($name = NULL) {
  return backdrop_get_http_header($name);
}
function drupal_send_headers($default_headers = array(), $only_default = FALSE) {
  return backdrop_send_headers($default_headers, $only_default);
}
function drupal_page_header() {
  return backdrop_page_header();
}
function drupal_serve_page_from_cache(stdClass $cache) {
  return backdrop_serve_page_from_cache($cache);
}
function drupal_unpack($obj, $field = 'data') {
  return backdrop_unpack($obj, $field);
}
function drupal_validate_utf8($text) {
  return backdrop_validate_utf8($text);
}
function drupal_set_message($message = NULL, $type = 'status', $repeat = TRUE) {
  return backdrop_set_message($message, $type, $repeat);
}
function drupal_get_messages($type = NULL, $clear_queue = TRUE) {
  return backdrop_get_messages($type, $clear_queue);
}
function drupal_get_title() {
  return backdrop_get_title();
}
function drupal_set_title($title = NULL, $output = CHECK_PLAIN) {
  return backdrop_set_title($title, $output);
}
function drupal_random_key($byte_count = 32) {
  return backdrop_random_key($byte_count);
}
function drupal_base64_encode($string) {
  return backdrop_base64_encode($string);
}
function drupal_random_bytes($count) {
  return backdrop_random_bytes($count);
}
function drupal_hmac_base64($data, $key) {
  return backdrop_hmac_base64($data, $key);
}
function drupal_hash_base64($data) {
  return backdrop_hash_base64($data);
}
function drupal_array_merge_deep() {
  $args = func_get_args(); // pass $args to function
  return call_user_func_array('backdrop_array_merge_deep', $args);
}
function drupal_array_merge_deep_array($arrays) {
  return backdrop_array_merge_deep_array($arrays);
}
function drupal_anonymous_user() {
  return backdrop_anonymous_user();
}
function drupal_bootstrap($phase = NULL, $new_phase = TRUE) {
  return backdrop_bootstrap($phase, $new_phase);
}
function drupal_get_user_timezone() {
  return backdrop_get_user_timezone();
}
function drupal_get_bootstrap_phase() {
  return backdrop_get_bootstrap_phase();
}
function drupal_valid_test_ua() {
  return backdrop_valid_test_ua();
}
function drupal_generate_test_ua($prefix) {
  return backdrop_generate_test_ua($prefix);
}
function drupal_maintenance_theme() {
  return backdrop_maintenance_theme();
}
function drupal_fast_404() {
  return fast_404();
}
function drupal_installation_attempted() {
  return backdrop_installation_attempted();
}
function drupal_language_initialize() {
  return backdrop_language_initialize();
}
function drupal_get_schema($table = NULL, $rebuild = FALSE) {
  return backdrop_get_schema($table, $rebuild);
}
function drupal_get_complete_schema($rebuild = FALSE) {
  return backdrop_get_complete_schema($rebuild);
}
function drupal_autoload_interface($interface) {
  return backdrop_autoload($interface);
}
function drupal_autoload_class($class) {
  return backdrop_autoload($class);
}
function &drupal_static($name, $default_value = NULL, $reset = FALSE) {
  return backdrop_static($name, $default_value, $reset);
}
function drupal_static_reset($name = NULL) {
  return backdrop_static_reset($name);
}
function drupal_is_cli() {
  return backdrop_is_cli();
}
function drupal_placeholder($text) {
  return backdrop_placeholder($text);
}
function &drupal_register_shutdown_function($callback = NULL) {
  return backdrop_register_shutdown_function($callback);
}

// Functions from common.inc.
function drupal_add_region_content($region = NULL, $data = NULL) {
  return backdrop_add_region_content($region, $data);
}
function drupal_get_region_content($region = NULL, $delimiter = ' ') {
  return backdrop_get_region_content($region, $delimiter);
}
function drupal_get_profile() {
  return backdrop_get_profile();
}
function drupal_set_breadcrumb($breadcrumb = NULL) {
  return backdrop_set_breadcrumb($breadcrumb);
}
function drupal_get_breadcrumb() {
  return backdrop_get_breadcrumb();
}
function drupal_add_html_head($data = NULL, $key = NULL) {
  return backdrop_add_html_head($data, $key);
}
function drupal_get_html_head() {
  return backdrop_get_html_head();
}
function drupal_add_feed($url = NULL, $title = '') {
  return backdrop_add_feed($url, $title);
}
function drupal_get_feeds($delimiter = "\n") {
  return backdrop_get_feeds($delimiter);
}
function drupal_get_query_parameters(array $query = NULL, array $exclude = array('q'), $parent = '') {
  return backdrop_get_query_parameters($query, $exclude, $parent);
}
function drupal_get_query_array($query) {
  return backdrop_get_query_array($query);
}
function drupal_http_build_query(array $query, $parent = '') {
  return backdrop_http_build_query($query, $parent);
}
function drupal_get_destination() {
  return backdrop_get_destination();
}
function drupal_parse_url($url) {
  return backdrop_parse_url($url);
}
function drupal_encode_path($path) {
  return backdrop_encode_path($path);
}
function drupal_goto($path = '', array $options = array(), $http_response_code = 302) {
  return backdrop_goto($path, $options, $http_response_code);
}
function drupal_site_offline() {
  return backdrop_site_offline();
}
function drupal_not_found() {
  return backdrop_not_found();
}
function drupal_access_denied() {
  return backdrop_access_denied();
}
function drupal_http_request($url, array $options = array()) {
  return backdrop_http_request($url, $options);
}
function drupal_strip_dangerous_protocols($uri) {
  return backdrop_strip_dangerous_protocols($uri);
}
function drupal_http_header_attributes(array $attributes = array()) {
  return backdrop_http_header_attributes($attributes);
}
function drupal_attributes(array $attributes = array()) {
  return backdrop_attributes($attributes);
}
function drupal_deliver_page($page_callback_result, $default_delivery_callback = NULL) {
  return backdrop_deliver_page($page_callback_result, $default_delivery_callback);
}
function drupal_deliver_html_page($page_callback_result) {
  return backdrop_deliver_html_page($page_callback_result);
}
function drupal_page_footer() {
  return backdrop_page_footer();
}
function drupal_exit($destination = NULL) {
  return backdrop_exit($destination);
}
function drupal_map_assoc($array, $function = NULL) {
  return backdrop_map_assoc($array, $function);
}
function drupal_set_time_limit($time_limit) {
  return backdrop_set_time_limit($time_limit);
}
function drupal_get_path($type, $name) {
  return backdrop_get_path($type, $name);
}
function drupal_add_html_head_link($attributes, $header = FALSE) {
  return backdrop_add_html_head_link($attributes, $header);
}
function drupal_add_css($data = NULL, $options = NULL) {
  return backdrop_add_css($data, $options);
}
function drupal_get_css($css = NULL, $skip_alter = FALSE) {
  return backdrop_get_css($css, $skip_alter);
}
function drupal_group_css($css) {
  return backdrop_group_css($css);
}
function drupal_aggregate_css(&$css_groups) {
  return backdrop_aggregate_css($css_groups);
}
function drupal_pre_render_styles($elements) {
  return backdrop_pre_render_styles($elements);
}
function drupal_build_css_cache($css) {
  return backdrop_build_css_cache($css);
}
function drupal_load_stylesheet($file, $optimize = NULL, $reset_basepath = TRUE) {
  return backdrop_load_stylesheet($file, $optimize, $reset_basepath);
}
function drupal_load_stylesheet_content($contents, $optimize = FALSE) {
  return backdrop_load_stylesheet_content($contents, $optimize);
}
function drupal_clear_css_cache() {
  return backdrop_clear_css_cache();
}
function drupal_delete_file_if_stale($uri) {
  return backdrop_delete_file_if_stale($uri);
}
function drupal_clean_css_identifier($identifier, $filter = array(' ' => '-', '_' => '-', '/' => '-', '[' => '-', ']' => '')) {
  return backdrop_clean_css_identifier($identifier, $filter);
}
function drupal_html_class($class) {
  return backdrop_html_class($class);
}
function drupal_html_id($id) {
  return backdrop_html_id($id);
}
function drupal_region_class($region) {
  return backdrop_region_class($region);
}
function drupal_add_js($data = NULL, $options = NULL) {
  return backdrop_add_js($data, $options);
}
function drupal_js_defaults($data = NULL) {
  return backdrop_js_defaults($data);
}
function drupal_get_js($scope = 'header', $javascript = NULL, $skip_alter = FALSE) {
  return backdrop_get_js($scope, $javascript, $skip_alter);
}
function drupal_pre_render_scripts($elements) {
  return backdrop_pre_render_scripts($elements);
}
function drupal_group_js($javascript) {
  return backdrop_group_js($javascript);
}
function drupal_aggregate_js(&$js_groups) {
  return backdrop_aggregate_js($js_groups);
}
function drupal_process_attached($elements, $group = JS_DEFAULT, $dependency_check = FALSE, $every_page = NULL) {
  return backdrop_process_attached($elements, $group, $dependency_check, $every_page);
}
function drupal_process_states(&$elements) {
  return backdrop_process_states($elements);
}
function drupal_add_library($module, $name, $every_page = NULL) {
  // Compatibility with System libraries like "drupal.autocomplete".
  $name = str_replace('drupal.', 'backdrop.', $name);
  return backdrop_add_library($module, $name, $every_page);
}
function drupal_get_library($module, $name = NULL) {
  return backdrop_get_library($module, $name);
}
function drupal_add_tabledrag($table_id, $action, $relationship, $group, $subgroup = NULL, $source = NULL, $hidden = TRUE, $limit = 0) {
  return backdrop_add_tabledrag($table_id, $action, $relationship, $group, $subgroup, $source, $hidden, $limit);
}
function drupal_build_js_cache($files) {
  return backdrop_build_js_cache($files);
}
function drupal_clear_js_cache() {
  return backdrop_clear_js_cache();
}
function drupal_json_encode($var) {
  return backdrop_json_encode($var);
}
function drupal_json_decode($var) {
  return backdrop_json_decode($var);
}
function drupal_json_output($var = NULL) {
  return backdrop_json_output($var);
}
function drupal_get_hash_salt() {
  return backdrop_get_hash_salt();
}
function drupal_get_private_key() {
  return backdrop_get_private_key();
}
function drupal_get_token($value = '') {
  return backdrop_get_token($value);
}
function drupal_valid_token($token, $value = '', $skip_anonymous = FALSE) {
  return backdrop_valid_token($token, $value, $skip_anonymous);
}
function drupal_page_set_cache() {
  return backdrop_page_set_cache();
}
function drupal_cron_run() {
  return backdrop_cron_run();
}
function drupal_system_listing($mask, $directory, $key = 'name', $min_depth = 1) {
  return backdrop_system_listing($mask, $directory, $key, $min_depth);
}
function drupal_pre_render_conditional_comments($elements) {
  return backdrop_pre_render_conditional_comments($elements);
}
function drupal_pre_render_link($element) {
  return backdrop_pre_render_link($element);
}
function drupal_pre_render_links($element) {
  return backdrop_pre_render_links($element);
}
function drupal_pre_render_markup($elements) {
  return backdrop_pre_render_markup($elements);
}
function drupal_render_page($page) {
  return backdrop_render_page($page);
}
function drupal_render(&$elements) {
  return backdrop_render($elements);
}
function drupal_render_children(&$element, $children_keys = NULL) {
  return backdrop_render_children($element, $children_keys);
}
function drupal_render_cache_get($elements) {
  return backdrop_render_cache_get($elements);
}
function drupal_render_cache_set(&$markup, $elements) {
  return backdrop_render_cache_set($markup, $elements);
}
function drupal_render_collect_attached($elements, $return = FALSE) {
  return backdrop_render_collect_attached($elements, $return);
}
function drupal_render_cache_by_query($query, $function, $expire = CACHE_TEMPORARY, $granularity = NULL) {
  return backdrop_render_cache_by_query($query, $function, $expire, $granularity);
}
function drupal_render_cid_parts($granularity = NULL) {
  return backdrop_render_cid_parts($granularity);
}
function drupal_render_cid_create($elements) {
  return backdrop_render_cid_create($elements);
}

/**
 * Sorts a structured array by '#title' property.
 *
 * @deprecated since 1.0
 * @see backdrop_sort()
 */
function element_sort_by_title($a, $b) {
  $a_title = (is_array($a) && isset($a['#title'])) ? $a['#title'] : '';
  $b_title = (is_array($b) && isset($b['#title'])) ? $b['#title'] : '';
  return strnatcasecmp($a_title, $b_title);
}

/**
 * Sorts a structured array by '#weight' property.
 *
 * @deprecated since 1.0
 * @see backdrop_sort()
 */
function element_sort($a, $b) {
  $a_weight = (is_array($a) && isset($a['#weight'])) ? $a['#weight'] : 0;
  $b_weight = (is_array($b) && isset($b['#weight'])) ? $b['#weight'] : 0;
  if ($a_weight == $b_weight) {
    return 0;
  }
  return ($a_weight < $b_weight) ? -1 : 1;
}

/**
 * Sorts a structured array by 'weight' key (no # prefix).
 *
 * @deprecated since 1.0
 * @see backdrop_sort()
 */
function drupal_sort_weight($a, $b) {
  $a_weight = (is_array($a) && isset($a['weight'])) ? $a['weight'] : 0;
  $b_weight = (is_array($b) && isset($b['weight'])) ? $b['weight'] : 0;
  if ($a_weight == $b_weight) {
    return 0;
  }
  return ($a_weight < $b_weight) ? -1 : 1;
}

/**
 * Sorts a structured array by 'title' key (no # prefix).
 *
 * @deprecated since 1.0
 * @see backdrop_sort()
 */
function drupal_sort_title($a, $b) {
  if (!isset($b['title'])) {
    return -1;
  }
  if (!isset($a['title'])) {
    return 1;
  }
  return strcasecmp($a['title'], $b['title']);
}

function &drupal_array_get_nested_value(array &$array, array $parents, &$key_exists = NULL) {
  return backdrop_array_get_nested_value($array, $parents, $key_exists);
}
function drupal_array_set_nested_value(array &$array, array $parents, $value, $force = FALSE) {
  return backdrop_array_set_nested_value($array, $parents, $value, $force);
}
function drupal_array_unset_nested_value(array &$array, array $parents, &$key_existed = NULL) {
  return backdrop_array_unset_nested_value($array, $parents, $key_existed);
}
function drupal_array_nested_key_exists(array $array, array $parents) {
  return backdrop_array_nested_key_exists($array, $parents);
}
function drupal_common_theme() {
  return backdrop_common_theme();
}
function drupal_install_schema($module) {
  return backdrop_install_schema($module);
}
function drupal_uninstall_schema($module) {
  return backdrop_uninstall_schema($module);
}
function drupal_get_schema_unprocessed($module, $table = NULL) {
  return backdrop_get_schema_unprocessed($module, $table);
}
function drupal_schema_fields_sql($table, $prefix = NULL) {
  return backdrop_schema_fields_sql($table, $prefix);
}
function drupal_write_record($table, &$record, $primary_keys = array()) {
  return backdrop_write_record($table, $record, $primary_keys);
}
function drupal_parse_info_file($filename, $process_sections = FALSE) {
  return backdrop_parse_info_file($filename, $process_sections);
}
function drupal_parse_info_format($data, $process_sections = FALSE) {
  return backdrop_parse_info_format($data, $process_sections);
}
function drupal_explode_tags($tags) {
  return backdrop_explode_tags($tags);
}
function drupal_implode_tags($tags) {
  return backdrop_implode_tags($tags);
}
function drupal_flush_all_caches() {
  return backdrop_flush_all_caches();
}
function drupal_parse_dependency($dependency) {
  return backdrop_parse_dependency($dependency);
}
function drupal_check_incompatibility($v, $current_version) {
  return backdrop_check_incompatibility($v, $current_version);
}
function drupal_get_updaters() {
  return backdrop_get_updaters();
}
function drupal_get_filetransfer_info() {
  return backdrop_get_filetransfer_info();
}

/**
 * Format a username.
 *
 * @deprecated since 1.0
 */
function format_username($account) {
  return user_format_name($account);
}

// Functions from errors.inc.
function drupal_error_levels() {
  return backdrop_error_levels();
}
// Functions from file.inc.
function drupal_move_uploaded_file($filename, $uri) {
  return backdrop_move_uploaded_file($filename, $uri);
}
function drupal_chmod($uri, $mode = NULL) {
  return backdrop_chmod($uri, $mode);
}
function drupal_unlink($uri, $context = NULL) {
  return backdrop_unlink($uri, $context);
}
function drupal_realpath($uri) {
  return backdrop_realpath($uri);
}
function drupal_dirname($uri) {
  return backdrop_dirname($uri);
}
function drupal_basename($uri, $suffix = NULL) {
  return backdrop_basename($uri, $suffix);
}
function drupal_mkdir($uri, $mode = NULL, $recursive = FALSE, $context = NULL) {
  return backdrop_mkdir($uri, $mode, $recursive, $context);
}
function drupal_rmdir($uri, $context = NULL) {
  return backdrop_rmdir($uri, $context);
}
function drupal_tempnam($directory, $prefix) {
  return backdrop_tempnam($directory, $prefix);
}

/**
 * Creates a .htaccess file in the given directory.
 *
 * @deprecated since 1.0
 */
function file_create_htaccess($directory, $private = TRUE, $force_overwrite = FALSE) {
  return file_save_htaccess($directory, $private, $force_overwrite);
}

// Functions from form.inc.
function drupal_get_form() {
  $args = func_get_args(); // pass $args to form function
  return call_user_func_array('backdrop_get_form', $args);
}
function drupal_build_form($form_id, &$form_state) {
  return backdrop_build_form($form_id, $form_state);
}
function drupal_rebuild_form($form_id, &$form_state, $old_form = NULL) {
  return backdrop_rebuild_form($form_id, $form_state, $old_form);
}
function drupal_form_submit($form_id, &$form_state) {
  return backdrop_form_submit($form_id, $form_state);
}
function drupal_retrieve_form($form_id, &$form_state) {
  return backdrop_retrieve_form($form_id, $form_state);
}
function drupal_process_form($form_id, &$form, &$form_state) {
  return backdrop_process_form($form_id, $form, $form_state);
}
function drupal_prepare_form($form_id, &$form, &$form_state) {
  return backdrop_prepare_form($form_id, $form, $form_state);
}
function drupal_validate_form($form_id, &$form, &$form_state) {
  return backdrop_validate_form($form_id, $form, $form_state);
}
function drupal_redirect_form($form_state) {
  return backdrop_redirect_form($form_state);
}
// Functions from graph.inc.
function drupal_depth_first_search(&$graph) {
  return backdrop_depth_first_search($graph);
}
// Functions from install.core.inc.
function install_drupal() {
  return install_backdrop();
}
// Functions from install.inc.
function drupal_load_updates() {
  return backdrop_load_updates();
}
function drupal_get_schema_versions($module) {
  return backdrop_get_schema_versions($module);
}
function drupal_get_installed_schema_version($module, $reset = FALSE, $array = FALSE) {
  return backdrop_get_installed_schema_version($module, $reset, $array);
}
function drupal_set_installed_schema_version($module, $version) {
  return backdrop_set_installed_schema_version($module, $version);
}
function drupal_install_profile_distribution_name() {
  return backdrop_install_profile_distribution_name();
}
function drupal_detect_baseurl($file = 'core/install.php') {
  return backdrop_detect_baseurl($file);
}
function drupal_detect_database_types() {
  $installer = backdrop_load_database_driver();
  return array('mysql' => $installer->name());
}
function drupal_get_database_types() {
  $mysql = backdrop_load_database_driver();
  return array('mysql' => $mysql);
}
function drupal_rewrite_settings($settings = array()) {
  return backdrop_rewrite_settings($settings);
}
function drupal_verify_profile($install_state) {
  return backdrop_verify_profile($install_state);
}
function drupal_install_system() {
  return backdrop_install_system();
}
function drupal_uninstall_modules($module_list = array(), $uninstall_dependents = TRUE) {
  return backdrop_uninstall_modules($module_list, $uninstall_dependents);
}
function drupal_verify_install_file($file, $mask = NULL, $type = 'file') {
  return backdrop_verify_install_file($file, $mask, $type);
}
function drupal_install_mkdir($file, $mask, $message = TRUE) {
  return backdrop_install_mkdir($file, $mask, $message);
}
function drupal_current_script_url($query = array()) {
  return backdrop_current_script_url($query);
}
function drupal_requirements_url($severity) {
  return backdrop_requirements_url($severity);
}
function drupal_check_profile($profile) {
  return backdrop_check_profile($profile);
}
function drupal_requirements_severity(&$requirements) {
  return backdrop_requirements_severity($requirements);
}
function drupal_check_module($module) {
  return backdrop_check_module($module);
}
// Functions from mail.inc.
function drupal_mail($module, $key, $to, $language, $params = array(), $from = NULL, $send = TRUE) {
  return backdrop_mail($module, $key, $to, $language, $params, $from, $send);
}
function drupal_mail_system($module, $key) {
  return backdrop_mail_system($module, $key);
}
function drupal_wrap_mail($text, $indent = '') {
  return backdrop_wrap_mail($text, $indent);
}
function drupal_html_to_text($string, $allowed_tags = NULL) {
  return backdrop_html_to_text($string, $allowed_tags);
}
// Functions from module.inc.
function drupal_required_modules() {
  return backdrop_required_modules();
}
function drupal_alter($type, &$data, &$context1 = NULL, &$context2 = NULL) {
  return backdrop_alter($type, $data, $context1, $context2);
}
// Functions from path.inc.
function drupal_path_initialize() {
  return backdrop_path_initialize();
}
function drupal_lookup_path($action, $path = '', $langcode = NULL) {
  return backdrop_lookup_path($action, $path, $langcode);
}
function drupal_cache_system_paths() {
  return backdrop_cache_system_paths();
}
function drupal_get_path_alias($path = NULL, $langcode = NULL) {
  return backdrop_get_path_alias($path, $langcode);
}
function drupal_get_normal_path($path, $langcode = NULL) {
  return backdrop_get_normal_path($path, $langcode);
}
function drupal_is_front_page() {
  return backdrop_is_front_page();
}
function drupal_match_path($path, $patterns) {
  return backdrop_match_path($path, $patterns);
}
function drupal_path_alias_whitelist_rebuild($source = NULL) {
  return;
}
function drupal_valid_path($path, $dynamic_allowed = FALSE) {
  return backdrop_valid_path($path, $dynamic_allowed);
}
function drupal_clear_path_cache($source = NULL) {
  return backdrop_clear_path_cache($source);
}
// Functions from session.inc.
function drupal_session_initialize() {
  return backdrop_session_initialize();
}
function drupal_session_start() {
  return backdrop_session_start();
}
function drupal_session_commit() {
  return backdrop_session_commit();
}
function drupal_session_started($set = NULL) {
  return backdrop_session_started($set);
}
function drupal_session_regenerate() {
  return backdrop_session_regenerate();
}
function drupal_session_destroy_uid($uid) {
  return backdrop_session_destroy_uid($uid);
}
function drupal_save_session($status = NULL) {
  return backdrop_save_session($status);
}
// Functions from theme.inc.
function drupal_theme_access($theme) {
  return backdrop_theme_access($theme);
}
function drupal_theme_initialize() {
  return backdrop_theme_initialize();
}
function drupal_theme_rebuild() {
  return backdrop_theme_rebuild();
}
function drupal_find_base_themes($themes, $key, $used_keys = array()) {
  return backdrop_find_base_themes($themes, $key, $used_keys);
}
function drupal_find_theme_functions($cache, $prefixes) {
  return backdrop_find_theme_functions($cache, $prefixes);
}
function drupal_find_theme_templates($cache, $extension, $path) {
  return backdrop_find_theme_templates($cache, $extension, $path);
}
// Functions from unicode.inc.
function drupal_xml_parser_create(&$data) {
  return backdrop_xml_parser_create($data);
}
function drupal_convert_to_utf8($data, $encoding) {
  return backdrop_convert_to_utf8($data, $encoding);
}
function drupal_truncate_bytes($string, $len) {
  return backdrop_truncate_bytes($string, $len);
}
function drupal_strlen($text) {
  return backdrop_strlen($text);
}
function drupal_strtoupper($text) {
  return backdrop_strtoupper($text);
}
function drupal_strtolower($text) {
  return backdrop_strtolower($text);
}
function drupal_ucfirst($text) {
  return backdrop_ucfirst($text);
}
function drupal_substr($text, $start, $length = NULL) {
  return backdrop_substr($text, $start, $length);
}
// Functions from utility.inc.
function drupal_var_export($var, $prefix = '') {
  return backdrop_var_export($var, $prefix);
}

/**
 * Drupal-compatible wrapper for clearing caches.
 *
 * Backdrop no longer needs to "rebuild" its node types, which previously
 * were synced between the code and database in Drupal 7. In Backdrop, all
 * node types are always stored in config.
 *
 * @deprecated since 1.0
 */
function node_types_rebuild() {
  node_type_cache_reset();
}

/**
 * Sets up a form to save information automatically.
 *
 * This function adds a submit handler and a submit button to a form array. The
 * submit function saves all the data in the form, using variable_set(), to
 * variables named the same as the keys in the form array. Note that this means
 * you should normally prefix your form array keys with your module name, so
 * that they are unique when passed into variable_set().
 *
 * If you need to manipulate the data in a custom manner, you can either put
 * your own submission handler in the form array before calling this function,
 * or just use your own submission handler instead of calling this function.
 *
 * This function is deprecated in Backdrop because it saves to the old
 * "variables" table. In Backdrop sites, developers are encouraged to save
 * configuration using the config system. See config(), config_set(), and
 * config_get() for information on saving into configuration.
 *
 * @deprecated since 1.0
 */
function system_settings_form($form) {
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration'));

  if (!empty($_POST) && form_get_errors()) {
    backdrop_set_message(t('The settings have not been saved because of the errors.'), 'error');
  }
  $form['#submit'][] = 'system_settings_form_submit';
  if (!isset($form['#theme'])) {
    $form['#theme'] = 'system_settings_form';
  }
  return $form;
}

/**
 * Form submission handler for system_settings_form().
 *
 * If you want node type configure style handling of your checkboxes,
 * add an array_filter value to your form.
 *
 * Use of this function is discouraged because it is not compatible with the
 * configuration system. See config(), config_set(), and config_get() for
 * alternatives. Rather than reusing this submit handler throughout settings
 * forms, developers should create separate submit handlers for each form
 * to save the settings into various configuration files as needed.
 *
 * @deprecated since 1.0
 */
function system_settings_form_submit($form, &$form_state) {
  form_state_values_clean($form_state);

  foreach ($form_state['values'] as $key => $value) {
    if (is_array($value) && isset($form_state['values']['array_filter'])) {
      $value = array_keys(array_filter($value));
    }
    variable_set($key, $value);
  }

  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Alters Backdrop's list of defined date formats and attributes to return the
 * expected date types and attributes for Drupal's system_get_date_types()
 * function.
 *
 * @deprecated since 1.0
 */
function system_get_date_types($type = NULL) {
  $formats = system_get_date_formats($type);

  if ($type) {
    foreach ($formats as &$format) {
      $format['type'] = $format['name'];
      $format['title'] = $format['label'];
      $format['locked'] = isset($format['hidden']) ? $format['hidden'] : '0';
      if (!isset($format['module'])) {
        $format['module'] = '';
      }
      $format['is_new'] = FALSE;
    }
  }
  else {
    $formats['type'] = $formats['name'];
    $formats['title'] = $formats['label'];
    $formats['locked'] = isset($formats['hidden']) ? $formats['hidden'] : '0';
    if (!isset($formats['module'])) {
      $formats['module'] = '';
    }
    $formats['is_new'] = FALSE;
  }

  return $formats;
}

/**
 * @} End of "defgroup drupal_compatibility".
 */
