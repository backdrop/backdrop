<?php
/**
 * Tests for URL generation functions.
 */
class ProfileCachePrepareTestCase extends BackdropWebTestCase {
  protected $profile = 'minimal';

  /**
   * Setup databasePrefix and profile to prepare cache tables and config directories.
   */
  public function setProfile($profile) {
    $this->profile = $profile;
    // Create the database prefix for this test.
    $this->databasePrefix = 'simpletest_cache_' . $this->profile . '_';
  }

  /**
   * Check if cache folder already exists.
   *
   * @return
   *   TRUE if cache exists, FALSE if no cache for current profile.
   */
  function isCached(){
    $file_public_path = config_get('system.core', 'file_public_path', 'files');
    $cache_dir = $file_public_path . '/simpletest/' . substr($this->databasePrefix, 0, -1);
    if(is_dir($cache_dir)){
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Prepare cache tables and config dirs. Speed up test speed by converting tables to MyISAM.
   */
  function prepareCache() {
    $this->setUp();
    $this->alterToMyISAM();
    $this->tearDown();
  }

  /**
   * Prepares the current environment for running the test.
   *
   * Backups various current environment variables and resets them, so they do
   * not interfere with the Backdrop site installation in which tests are executed
   * and can be restored in tearDown().
   *
   * Also sets up new resources for the testing environment, such as the public
   * filesystem and configuration directories.
   *
   * @see BackdropWebTestCase::setUp()
   * @see BackdropWebTestCase::tearDown()
   */
  protected function prepareEnvironment() {
    global $user, $language, $settings, $config_directories;

    // Store necessary current values before switching to prefixed database.
    $this->originalLanguage = $language;
    $this->originalLanguageDefault = config_get('system.core', 'language_default');
    $this->originalConfigDirectories = $config_directories;
    $this->originalFileDirectory = config_get('system.core', 'file_public_path', 'files');
    $this->originalProfile = backdrop_get_profile();
    $this->originalCleanUrl = config_get('system.core', 'clean_url');
    $this->originalUser = $user;
    $this->originalSettings = $settings;

    // Set to English to prevent exceptions from utf8_truncate() from t()
    // during install if the current language is not 'en'.
    // The following array/object conversion is copied from language_default().
    $language = (object) array(
      'langcode' => 'en',
      'name' => 'English',
      'direction' => 0,
      'enabled' => 1,
      'weight' => 0,
    );

    // Save and clean the shutdown callbacks array because it is static cached
    // and will be changed by the test run. Otherwise it will contain callbacks
    // from both environments and the testing environment will try to call the
    // handlers defined by the original one.
    $callbacks = &backdrop_register_shutdown_function();
    $this->originalShutdownCallbacks = $callbacks;
    $callbacks = array();

    // Create test directory ahead of installation so fatal errors and debug
    // information can be logged during installation process.
    // Use temporary files directory with the same prefix as the database.
    $this->public_files_directory = $this->originalFileDirectory . '/simpletest/' . substr($this->databasePrefix, 0, -1);
    $this->private_files_directory = $this->public_files_directory . '/private';
    $this->temp_files_directory = $this->private_files_directory . '/temp';

    // Create the directories.
    file_prepare_directory($this->public_files_directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
    file_prepare_directory($this->private_files_directory, FILE_CREATE_DIRECTORY);
    file_prepare_directory($this->temp_files_directory, FILE_CREATE_DIRECTORY);
    $this->generatedTestFiles = FALSE;

    // Set the new config directories. During test execution, these values are
    // manually set directly in config_get_config_directory().
    $config_base_path = 'files/simpletest/' . substr($this->databasePrefix, 0, -1) . '/config_';
    $config_directories['active'] = $config_base_path . 'active';
    $config_directories['staging'] = $config_base_path . 'staging';
    $active_directory = config_get_config_directory('active');

    $staging_directory = config_get_config_directory('staging');
    file_prepare_directory($active_directory, FILE_CREATE_DIRECTORY);
    file_prepare_directory($staging_directory, FILE_CREATE_DIRECTORY);

    // Log fatal errors.
    ini_set('log_errors', 1);
    ini_set('error_log', $this->public_files_directory . '/error.log');

    // Set the test information for use in other parts of Backdrop.
    $test_info = &$GLOBALS['backdrop_test_info'];
    $test_info['test_run_id'] = $this->databasePrefix;
    $test_info['in_child_site'] = FALSE;

    // Disable Drupal compatibility for test runs.
    $settings['backdrop_drupal_compatibility'] = FALSE;

    // Indicate the environment was set up correctly.
    $this->setupEnvironment = TRUE;
  }

  /**
   * Sets up a Backdrop site for running functional and integration tests.
   *
   *
   * Iinstall Backdrop with the specified installation profile in $profile into the
   * prefixed database. 
   *
   * After installation all caches are flushed and several configuration values
   * are reset to the values of the parent site executing the test, since the
   * default values may be incompatible with the environment in which tests are
   * being executed.
   *
   * @see BackdropWebTestCase::prepareDatabasePrefix()
   * @see BackdropWebTestCase::changeDatabasePrefix()
   * @see BackdropWebTestCase::prepareEnvironment()
   */
  protected function setUp(){
    global $conf;

    // Prepare the environment for running tests.
    $this->prepareEnvironment();
    if (!$this->setupEnvironment) {
      return FALSE;
    }

    // Reset all statics and variables to perform tests in a clean environment.
    $conf = array();
    backdrop_static_reset();

    // Change the database prefix.
    // All static variables need to be reset before the database prefix is
    // changed, since BackdropCacheArray implementations attempt to
    // write back to persistent caches when they are destructed.
    $this->changeDatabasePrefix();
    if (!$this->setupDatabasePrefix) {
      return FALSE;
    }

    // Preset the 'install_profile' system variable, so the first call into
    // system_rebuild_module_data() (in backdrop_install_system()) will register
    // the test's profile as a module. Without this, the installation profile of
    // the parent site (executing the test) is registered, and the test
    // profile's hook_install() and other hook implementations are never invoked.
    config_install_default_config('system');
    config_set('system.core', 'install_profile', $this->profile);

    // Perform the actual Backdrop installation.
    include_once BACKDROP_ROOT . '/core/includes/install.inc';
    backdrop_install_system(); // System install is 0.6 sec.

    // Set path variables.
    $core_config = config('system.core');
    $core_config->set('file_default_scheme', 'public');
    $core_config->set('file_public_path', $this->public_files_directory);
    $core_config->set('file_private_path', $this->private_files_directory);
    $core_config->set('file_temporary_path', $this->temp_files_directory);
    $core_config->save();

    // Ensure schema versions are recalculated.
    backdrop_static_reset('backdrop_get_schema_versions');

    // Include the testing profile.
    config_set('system.core', 'install_profile', $this->profile);
    $profile_details = install_profile_info($this->profile, 'en');


    // Install the modules specified by the testing profile.
    module_enable($profile_details['dependencies'], FALSE);  // install profile modules 2.2 sec

    // Run the profile tasks.
    $install_profile_module_exists = db_query("SELECT 1 FROM {system} WHERE type = 'module' AND name = :name", array(
      ':name' => $this->profile,
    ))->fetchField();
    if ($install_profile_module_exists) {
      module_enable(array($this->profile), FALSE); // probably we don't need this part because we already installed modules. 1.9 sec
    }

  }

  /**
   * Reset the database prefix and global config.
   */
  protected function tearDown() {
    global $user, $language, $settings, $config_directories;
    // Get back to the original connection.
    Database::removeConnection('default');
    Database::renameConnection('simpletest_original_default', 'default');

    // Set the configuration direcotires back to the originals.
    $config_directories = $this->originalConfigDirectories;

    // Restore the original settings.
    $settings = $this->originalSettings;

    // Restore original shutdown callbacks array to prevent original
    // environment of calling handlers from test run.
    $callbacks = &backdrop_register_shutdown_function();
    $callbacks = $this->originalShutdownCallbacks;

    // Return the user to the original one.
    $user = $this->originalUser;
    backdrop_save_session(TRUE);

    // Ensure that internal logged in variable and cURL options are reset.
    $this->loggedInUser = FALSE;
    $this->additionalCurlOptions = array();

    // Reload module list and implementations to ensure that test module hooks
    // aren't called after tests.
    module_list(TRUE);
    module_implements_reset();

    // Reset the Field API.
    field_cache_clear();

    // Rebuild caches.
    $this->refreshVariables();

    // Reset public files directory.
    $GLOBALS['conf']['file_public_path'] = $this->originalFileDirectory;

    // Reset language.
    $language = $this->originalLanguage;
    if ($this->originalLanguageDefault) {
      $GLOBALS['conf']['language_default'] = $this->originalLanguageDefault;
    }

    // Close the CURL handler.
    $this->curlClose();
  }

  /**
   * Alter tables to MyISAM engine to speed up tests.
   *
   * MyISAM is faster to delete and copy tables. It gives small adventage when /var/lib/mysql on SHM (memory) device, but much bigger when tests run on regular device.
   */
  protected function alterToMyISAM() {
    $skip_alter = array(
      'taxonomy_term_data',
      'node',
      'node_access',
      'node_revision',
      'node_comment_statistics',
    );
    $tables = db_find_tables($this->databasePrefix . '%');
    foreach ($tables as $table) {
      $original_table_name = substr($table, strlen($this->databasePrefix));
      if(!in_array($original_table_name, $skip_alter)){
        db_query('ALTER TABLE ' . $table . ' ENGINE=MyISAM');
      }
    }
  }
}
