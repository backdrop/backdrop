<?php
/**
 * @file
 * Contains class and interface wrappers for Drupal 7.x compatibility.
 */

/**
 * @addtogroup drupal_compatibility Drupal compatibility layer.
 * @{
 */

/**
 * @see bootstrap.inc
 */
abstract class DrupalCacheArray extends BackdropCacheArray {}

/**
 * @see bootstrap.inc
 */
class DrupalUpdateException extends BackdropUpdateException {}

/**
 * @see cache-install.inc
 */
class DrupalFakeCache extends BackdropFakeCache {}

/**
 * @see cache.inc
 */
class DrupalDatabaseCache extends BackdropDatabaseCache {}

/**
 * @see cache.inc
 */
class DrupalNullCache extends BackdropNullCache {}

/**
 * @see stream_wrappers.inc
 */
abstract class DrupalLocalStreamWrapper extends BackdropLocalStreamWrapper {}

/**
 * @see stream_wrappers.inc
 */
class DrupalPublicStreamWrapper extends BackdropPublicStreamWrapper {}

/**
 * @see stream_wrappers.inc
 */
class DrupalPrivateStreamWrapper extends BackdropPrivateStreamWrapper {}

/**
 * @see stream_wrappers.inc
 */
class DrupalTemporaryStreamWrapper extends BackdropTemporaryStreamWrapper {}

/**
 * @see stream_wrappers.inc
 */
interface DrupalStreamWrapperInterface extends BackdropStreamWrapperInterface {}

/**
 * @see updater.inc
 */
interface DrupalUpdaterInterface extends BackdropUpdaterInterface {}
/**
 * @see system.queue.inc
 */
class DrupalQueue extends BackdropQueue {}

/**
 * @see system.queue.inc
 */
interface DrupalQueueInterface extends BackdropQueueInterface {}

/**
 * @see system.queue.inc
 */
interface DrupalReliableQueueInterface extends BackdropReliableQueueInterface {}

/**
 * @see backdrop_web_test_case.php
 */
abstract class DrupalTestCase extends BackdropTestCase {}

/**
 * @see backdrop_web_test_case.php
 */
class DrupalUnitTestCase extends BackdropUnitTestCase {}

/**
 * @see backdrop_web_test_case.php
 */
class DrupalWebTestCase extends BackdropWebTestCase {

  /**
   * Get node by title.
   */
  public function drupalGetNodeByTitle($title, $reset) {
    return $this->backdropGetNodeByTitle($title, $reset);
  }

  /**
   * Create node.
   */
  protected function drupalCreateNode($settings = array()) {
    return $this->backdropCreateNode($settings);
  }

  /**
   * Create content type.
   */
  protected function drupalCreateContentType($settings = array()) {
    return $this->backdropCreateContentType($settings);
  }

  /**
   * Get test files.
   */
  protected function drupalGetTestFiles($type, $size = NULL) {
    return $this->backdropGetTestFiles($type, $size);
  }

  /**
   * Compare files.
   */
  protected function drupalCompareFiles($file1, $file2) {
    return $this->backdropCompareFiles($file1, $file2);
  }

  /**
   * Create user.
   */
  protected function drupalCreateUser(array $permissions = array()) {
    return $this->backdropCreateUser($permissions);
  }

  /**
   * Create role.
   */
  protected function drupalCreateRole(array $permissions, $name = NULL) {
    return $this->backdropCreateRole($permissions, $name);
  }

  /**
   * Login.
   */
  protected function drupalLogin($user) {
    $this->backdropLogin($user);
  }

  /**
   * Get token.
   */
  protected function drupalGetToken($value = '') {
    return $this->backdropGetToken($value);
  }

  /**
   * Logout.
   */
  protected function drupalLogout() {
    $this->backdropLogout();
  }

  /**
   * Retrieves a path.
   */
  protected function drupalGet($path, array $options = array(), array $headers = array()) {
    return $this->backdropGet($path, $options, $headers);
  }

  /**
   * Retrieves a path via AJAX.
   */
  protected function drupalGetAJAX($path, array $options = array(), array $headers = array()) {
    return $this->backdropGetAJAX($path, $options, $headers);
  }

  /**
   * Execute a POST request.
   */
  protected function drupalPost($path, $edit, $submit, array $options = array(), array $headers = array(), $form_html_id = NULL, $extra_post = NULL) {
    return $this->backdropPost($path, $edit, $submit, $options, $headers, $form_html_id, $extra_post);
  }

  /**
   * Execute an AJAX submission.
   */
  protected function drupalPostAJAX($path, $edit, $triggering_element, $ajax_path = NULL, array $options = array(), array $headers = array(), $form_html_id = NULL, $ajax_settings = NULL) {
    return $this->backdropPostAJAX($path, $edit, $triggering_element, $ajax_path, $options, $headers, $form_html_id, $ajax_settings);
  }

  /**
   * Get HTTP response headers.
   */
  protected function drupalGetHeaders($all_requests = FALSE) {
    return $this->backdropGetHeaders($all_requests);
  }

  /**
   * Get the value of the HTTP response header.
   */
  protected function drupalGetHeader($name, $all_requests = FALSE) {
    return $this->backdropGetHeader($name, $all_requests);
  }

  /**
   * Gets the raw HTML content of the requested page.
   */
  protected function drupalGetContent() {
    return $this->backdropGetContent();
  }

  /**
   * Gets the value of the Backdrop.settings JavaScript variable for the
   * currently loaded page.
   */
  protected function drupalGetSettings() {
    return $this->backdropGetSettings();
  }

  /**
   * Gets an array containing all emails sent during this test case.
   */
  protected function drupalGetMails($filter = array()) {
    return $this->backdropGetMails($filter);
  }

  /**
   * Sets the raw HTML content.
   */
  protected function drupalSetContent($content, $url = 'internal:') {
    $this->backdropSetContent($content, $url);
  }

  /**
   * Sets the value of the Backdrop.settings JavaScript variable for the
   * currently loaded page.
   */
  protected function drupalSetSettings($settings) {
    $this->backdropSetSettings($settings);
  }

  /**
   * Retrieves only the headers for a Backdrop path or an absolute path.
   */
  protected function drupalHead($path, array $options = array(), array $headers = array()) {
    return $this->backdropHead($path, $options, $headers);
  }
}

/**
 * Base entity controller.
 *
 * @see entity.controller.inc
 */
class DrupalDefaultEntityController extends DefaultEntityController {}
/**
 * Common entity controller.
 *
 * @see entity.controller.inc
 */
interface DrupalEntityControllerInterface extends EntityControllerInterface {}

/**
 * @}
 */
