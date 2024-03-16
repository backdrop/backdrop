<?php
/**
 * @file
 * This is the API for configuration storage.
 */

/**
 * Retrieves a configuration object.
 *
 * This is the main entry point to the configuration API. Calling
 * @code config(book.admin) @endcode will return a configuration object in which
 * the book module can store its administrative settings.
 *
 * @param string $config_file
 *   The name of the configuration object to retrieve. The name corresponds to
 *   an JSON configuration file. For @code config(book.admin) @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param string $type
 *   (optional) The type of config directory to return. Backdrop core provides
 *   'active' and 'staging'. Defaults to 'active'.
 *
 * @return Config
 *   A Config object containing the specified configuration settings.
 *
 */
function config($config_file, $type = 'active') {
  // Use the advanced backdrop_static() pattern, since this is called very often.
  static $backdrop_static_fast;
  if (!isset($backdrop_static_fast)) {
    $backdrop_static_fast['loaded_configs'] = &backdrop_static(__FUNCTION__);
  }
  $loaded_configs = &$backdrop_static_fast['loaded_configs'];

  if (!isset($loaded_configs[$type][$config_file])) {
    $storage = config_get_config_storage($type);
    $config = new Config($config_file, $storage);
    $config->load();
    $cache = $config->get('_config_static');
    if ($cache) {
      $loaded_configs[$type][$config_file] = $config;
    }
  }
  else {
    $config = $loaded_configs[$type][$config_file];
  }

  return $config;
}

/**
 * A shortcut function to delete a single value from a config file.
 *
 * Note that this function immediately writes the config file to disk and clears
 * associated caches related to the new config. If deleting a number of options
 * in the same configuration file, it is better to create a config object
 * directly, delete all the necessary values, and then save the config file
 * minus new options all at once.
 *
 * @param string $config_file
 *   The name of the configuration object to retrieve. The name corresponds to
 *   an JSON configuration file. For @code config(book.admin) @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param string $option
 *   The name of the config option within the file to delete. The config option
 *   may contain periods to indicate levels within the config file.
 *
 * @see config_set()
 * @see config_get()
 *
 * @since 1.7.0
 */
function config_clear($config_file, $option) {
  $config = config($config_file);
  $config->clear($option);
  $config->save();
}

/**
 * A shortcut function to check if a value is overridden within a config file.
 *
 * @param string $config_file
 *   The name of the configuration object to check. The name corresponds to
 *   an JSON configuration file. For @code config(book.admin) @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param string $option
 *   The name of the config option within the file to check. The config option
 *   may contain periods to indicate levels within the config file.
 *
 * @return bool
 *   TRUE if the config option is overridden. FALSE otherwise.
 *
 * @see Config::isOverridden()
 */
function config_is_overridden($config_file, $option) {
  return config($config_file)->isOverridden($option);
}

/**
 * A shortcut function to load and retrieve a single value from a config file.
 *
 * @param string $config_file
 *   The name of the configuration object to retrieve. The name corresponds to
 *   an JSON configuration file. For @code config(book.admin) @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param string $option
 *   The name of the config option within the file to read. The config option
 *   may contain periods to indicate levels within the config file. If NULL is
 *   passed in, the entire contents of the config file will be returned.
 *
 * @return mixed
 *   The contents of the requested config option. Returns NULL if the specified
 *   option was not found in the file at all.
 *
 * @see config_set()
 * @see config_clear()
 */
function config_get($config_file, $option = NULL) {
  $config = config($config_file);
  return $config->get($option);
}

/**
 * A shortcut function to load and retrieve a single translated value from a config file.
 *
 * @param string $config_file
 *   The name of the configuration object to retrieve. The name corresponds to
 *   an JSON configuration file. For @code config('book.admin') @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param string $option
 *   The name of the config option within the file to read. The config option
 *   may contain periods to indicate levels within the config file. If NULL is
 *   passed in, the entire contents of the config file will be returned.
 * @param array $args
 *   An associative array of replacements to make. Replacements are made in the
 *   same way as the t() function.
 * @param array $options
 *   An associative array of additional options, with the following elements:
 *   - 'langcode' (defaults to the current language): The language code to
 *     translate to a language other than what is used to display the page.
 *   - 'context' (defaults to the empty context): The context the source string
 *     belongs to.
 *
 * @return string
 *   The translated contents of the requested config option. Returns default if no
 *   translation is found. Returns NULL if the specified option was not found in
 *   the file at all or if not a string.
 *
 * @see t()
 * @see config_get()
 */
function config_get_translated($config_file, $option = NULL, $args = array(), $options = array()) {
  $config = config($config_file);
  return $config->getTranslated($option, $args, $options);
}

/**
 * A shortcut function to set and save a single value in a config file.
 *
 * Note that this function immediately writes the config file to disk and clears
 * associated caches related to the new config. If writing a number of options
 * to the same configuration file, it is better to create a config object
 * directly, set all the new values, and then save the config file with all the
 * new options all at once.
 *
 * @param string $config_file
 *   The name of the configuration object to retrieve. The name corresponds to
 *   an JSON configuration file. For @code config(book.admin) @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param string $option
 *   The name of the config option within the file to set. The config option
 *   may contain periods to indicate levels within the config file.
 * @param mixed $value
 *   The value to save into the config file.
 *
 * @see config_get()
 * @see config_clear()
 */
function config_set($config_file, $option, $value) {
  $config = config($config_file);
  $config->set($option, $value);
  $config->save();
}

/**
 * A shortcut function to set and save multiple values in a config file.
 *
 * Note that this function immediately writes the config file to disk and clears
 * associated caches related to the new config.  Unlike config_set() which would
 * need to be called once per value, this function updates all the values and
 * then saves the object.
 *
 * @param string $config_file
 *   The name of the configuration object to retrieve. The name corresponds to
 *   an JSON configuration file. For @code config(book.admin) @endcode, the
 *   config object returned will contain the contents of book.admin.json.
 * @param array $options
 *   A keyed array of configuration option names mapping their values.
 */
function config_set_multiple($config_file, $options) {
  $config = config($config_file);
  $config->setMultiple($options);
  $config->save();
}

/**
 * Returns the path of a configuration directory for config stored in files.
 *
 * @param string $type
 *   (optional) The type of config directory to return. Backdrop core provides
 *   'active' and 'staging'. Defaults to 'active'.
 *
 * @return string
 *   The configuration directory path.
 *
 * @throws ConfigException
 */
function config_get_config_directory($type = 'active') {
  global $config_directories;

  if ($test_prefix = backdrop_valid_test_ua()) {
    // See BackdropWebTestBase::setUp().
    $path = conf_path() . '/files/simpletest/' . substr($test_prefix, 10) . '/config_' . $type;
  }
  elseif (!empty($config_directories[$type])) {
    $path = $config_directories[$type];

    // If the path starts with a slash or dot, assume a normal path. If just
    // a directory name is provided, make it relative to the settings.php file.
    $first_character = substr($path, 0, 1);
    if (!in_array($first_character, array('.', '/', '\\'))) {
      $path = conf_path() . '/' . $path;
    }
  }
  else {
    throw new ConfigException(format_string('The configuration directory type "@type" does not exist.', array('@type' => $type)));
  }
  return $path;
}

/**
 * Retrieves all configurations starting with a particular prefix.
 *
 * @param string $prefix
 *   The prefix of the configuration names to retrieve.
 * @param string $type
 *   The configuration type, either "staging" or "active".
 *
 * @return array
 *   An array containing matching configuration object names.
 */
function config_get_names_with_prefix($prefix, $type = 'active') {
  $storage = config_get_config_storage($type);
  return $storage->listAll($prefix);
}

/**
 * Loads configuration objects by name.
 *
 * @param array $names
 *   The list of configuration names to load.
 * @param string $type
 *   The configuration type, either "staging" or "active".
 *
 * @return array
 *   An array containing matching configuration objects.
 */
function config_load_multiple($names, $type = 'active') {
  $storage = config_get_config_storage($type);
  return $storage->readMultiple($names);
}

/**
 * Moves the default config supplied by a project to the live config directory.
 *
 * @param string $project
 *   The name of the project we are installing.
 * @param string|NULL $config_name
 *   (optional) If wanting to copy just a single configuration file from the
 *   project, specify the configuration file name without the extension.
 *
 * @since 1.26.0 First parameter changed from $module to $project.
 */
function config_install_default_config($project, $config_name = NULL) {
  $project_path = NULL;
  foreach (array('module', 'theme') as $project_type) {
    if ($project_path = backdrop_get_path($project_type, $project)) {
      break;
    }
  }
  $project_config_dir = $project_path . '/config';
  if (is_dir($project_config_dir)) {
    $storage = new ConfigFileStorage($project_config_dir);
    $files = glob($project_config_dir . '/*.json');
    foreach ($files as $file) {
      // Load config data into the active store and write it out to the
      // file system in the Backdrop config directory. Note the config name
      // needs to be the same as the file name WITHOUT the extension.
      $parts = explode('/', $file);
      $file = array_pop($parts);
      $file_config_name = str_replace('.json', '', $file);
      if (is_null($config_name) || $file_config_name === $config_name) {
        $data = $storage->read($file_config_name);
        $config = config($file_config_name);
        // We only create new configs, and do not overwrite existing ones.
        if ($config->isNew()) {
          $config->setData($data);
          module_invoke_all('config_create', $config);
          $config->save();
        }
      }
    }
  }
}

/**
 * Uninstall all the configuration provided by a project.
 *
 * @param string $project
 *   The name of the project we are uninstalling.
 * @param string|NULL $config_name
 *   (optional) If wanting to remove just a single configuration file from the
 *   project, specify the configuration file name without the extension.
 *
 * @since 1.26.0 First parameter changed from $module to $project.
 */
function config_uninstall_config($project, $config_name = NULL) {
  // If this is a theme key, load the matching template.php file.
  if (!backdrop_load('module', $project) && $theme_path = backdrop_get_path('theme', $project)) {
    if (file_exists($theme_path . '/template.php')) {
      include_once $theme_path . '/template.php';
    }
  }

  if ($configs = module_invoke($project, 'config_info')) {
    foreach ($configs as $config_name => $config_info) {
      if (isset($config_info['name_key'])) {
        $sub_names = config_get_names_with_prefix($config_name . '.');
        foreach ($sub_names as $sub_name) {
          config($sub_name)->delete();
        }
      }
      else {
        config($config_name)->delete();
      }
    }
  }
}

/**
 * Get the storage object for the specified configuration type
 *
 * @param string $type
 *   (optional) The type of config directory to return. Backdrop core provides
 *   'active' and 'staging'. Defaults to 'active'.
 *
 * @return ConfigStorageInterface
 *   A ConfigStorageInterface object managing the specified configuration type.
 */
function config_get_config_storage($type = 'active') {
  $class = settings_get('config_' . $type . '_class', 'ConfigFileStorage');
  switch ($class) {
    case 'ConfigDatabaseStorage':
      $config_location = 'db:/default/' . $type . '_config';
      break;
    case 'ConfigFileStorage':
    default:
      $config_location = config_get_config_directory($type);
  }
  return new $class($config_location);
}

/**
 * A base exception thrown in any configuration system operations.
 */
class ConfigException extends Exception {}

/**
 * Exception thrown when a config object name is invalid.
 */
class ConfigNameException extends ConfigException {}

/**
 * Exception thrown when a config object has a validation error before saving.
 *
 * Messages thrown using ConfigValidateException should be translated, as they
 * are passed directly to end-users during form validations.
 */
class ConfigValidateException extends ConfigException {}

/**
 * Exception thrown by classes implementing ConfigStorageInterface.
 */
class ConfigStorageException extends ConfigException {}

/**
 * Exception thrown when attempting to read a config file fails.
 */
class ConfigStorageReadException extends ConfigStorageException {}

/**
 * Defines the default configuration object.
 */
class Config {

  /**
   * The maximum length of a configuration object name.
   *
   * Many filesystems (including HFS, NTFS, and ext4) have a maximum file name
   * length of 255 characters. To ensure that no configuration objects
   * incompatible with this limitation are created, we enforce a maximum name
   * length of 250 characters (leaving 5 characters for the file extension).
   *
   * @see http://en.wikipedia.org/wiki/Comparison_of_file_systems
   */
  const MAX_NAME_LENGTH = 250;

  /**
   * The name of the configuration object.
   *
   * @var string
   */
  protected $name;

  /**
   * Whether the configuration object is new or has been saved to the storage.
   *
   * @var bool
   */
  protected $isNew = TRUE;

  /**
   * The data of the configuration object.
   *
   * @var array
   */
  protected $data;

  /**
   * Any overrides specified for this configuration object.
   *
   * @var array
   */
  protected $overrides;

  /**
   * The state of validation on this object.
   *
   * This value is set to TRUE by Config::validate() and is reset to FALSE after
   * any change to the $data variable.
   */
  protected $validated = FALSE;

  /**
   * The storage used to load and save this configuration object.
   *
   * @var ConfigStorageInterface
   */
  protected $storage;

  /**
   * The configuration context used for this configuration object.
   *
   * @var ConfigStorageInterface
   */
  protected $context;

  /**
   * Whether the configuration object has already been loaded.
   *
   * @var bool
   */
  protected $isLoaded = FALSE;

  /**
   * Constructs a configuration object.
   *
   * @param string $name
   *   The name of the configuration object being constructed.
   * @param ConfigStorageInterface $storage
   *   A storage controller object to use for reading and writing the
   *   configuration data.
   */
  public function __construct($name, ConfigStorageInterface $storage) {
    global $config;

    $this->name = $name;
    $this->storage = $storage;
    $this->overrides = isset($config[$name]) ? $config[$name] : NULL;
  }

  /**
   * Initializes a configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function init() {
    $this->isLoaded = FALSE;
    return $this;
  }

  /**
   * Initializes a configuration object with pre-loaded data.
   *
   * @param array $data
   *   Array of loaded data for this configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function initWithData(array $data) {
    $this->isLoaded = TRUE;
    $this->isNew = FALSE;
    $this->replaceData($data);
    return $this;
  }

  /**
   * Returns the name of this configuration object.
   *
   * @return string
   *   The name of the configuration object.
   */
  public function getName() {
    return $this->name;
  }

  /**
   * Sets the name of this configuration object.
   *
   * @param string $name
   *   The name of the configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function setName($name) {
    $this->name = $name;
    return $this;
  }

  /**
   * Validates the configuration object name.
   *
   * @param string $name
   *   The name of the configuration object.
   *
   * @throws ConfigNameException
   *
   * @see Config::MAX_NAME_LENGTH
   */
  public static function validateName($name) {
    // The name must be namespaced by owner.
    if (strpos($name, '.') === FALSE) {
      throw new ConfigNameException(format_string('Missing namespace in Config object name @name.', array(
        '@name' => $name,
      )));
    }
    // The name must be shorter than Config::MAX_NAME_LENGTH characters.
    if (strlen($name) > self::MAX_NAME_LENGTH) {
      throw new ConfigNameException(format_string('Config object name @name exceeds maximum allowed length of @length characters.', array(
        '@name' => $name,
        '@length' => self::MAX_NAME_LENGTH,
      )));
    }

    // The name must not contain any of the following characters:
    // ? * < > " ' / \ :
    if (preg_match('/[:?*<>"\'\/\\\\]/', $name)) {
      throw new ConfigNameException(format_string('Invalid character in Config object name @name.', array(
        '@name' => $name,
      )));
    }
  }

  /**
   * Validate the full contents of the configuration data.
   *
   * This method is not automatically called when Config::setData() is called.
   * Because validation is a potentially expensive operation, you should call
   * this only when expecting potential problems in the provided data, such as
   * when validating user-provided imports.
   *
   * @throws ConfigValidateException
   */
  public function validateData() {
    if (!$this->validated) {
      $config_info = config_get_info($this->getName());
      module_invoke_all('config_data_validate', $this, $config_info);
      $this->validated = TRUE;
    }
  }

  /**
   * Returns whether this configuration object is new.
   *
   * @return bool
   *   TRUE if this configuration object does not exist in storage.
   */
  public function isNew() {
    if (!$this->isLoaded) {
      $this->load();
    }
    return $this->isNew;
  }

  /**
   * Check if a particular config key is overridden.
   *
   * @param string $key
   *   A string that maps to a key within the configuration data.
   *
   * @return bool
   *   TRUE if the $key is overridden. FALSE otherwise.
   */
  public function isOverridden($key) {
    return $this->getOverride($key) !== NULL;
  }

  /**
   * Gets all data from this configuration object.
   *
   * This call provides API symmetry with Config::setData().
   *
   * @return array
   *   All of the data from this config object.
   *
   * @see Config::get()
   *
   * @since 1.23.0 Method added.
   */
  public function getData() {
    return $this->get();
  }

  /**
   * Gets data from this configuration object.
   *
   * @param string $key
   *   A string that maps to a key within the configuration data.
   *   For instance in the following configuration array:
   *   @code
   *   array(
   *     'foo' => array(
   *       'bar' => 'baz',
   *     ),
   *   );
   *   @endcode
   *   A key of 'foo.bar' would return the string 'baz'. However, a key of 'foo'
   *   would return array('bar' => 'baz').
   *   If no key is specified, then the entire data array is returned.
   *
   * @return mixed
   *   The data that was requested.
   */
  public function get($key = '') {
    $value = $this->getOverride($key);
    if (!isset($value)) {
      $value = $this->getOriginal($key);
    }
    return $value;
  }

  /**
   * Gets the current config value as specified in the written config storage.
   *
   * In most cases, Config::get() should be used to pull the config value and
   * also include any overrides to apply. This method should be used only when
   * explicitly wanting the currently saved value (as stored in a config file)
   * rather than what may be specified in an override (as provided in
   * settings.php).
   *
   * @param string $key
   *   The string that maps to a key with the configuration data. See
   *   Config::get() for full examples.
   *
   * @return mixed
   *   The data that was requested.
   *
   * @see Config::get()
   */
  public function getOriginal($key = '') {
    if (!$this->isLoaded) {
      $this->load();
    }
    if (empty($key)) {
      return $this->data;
    }
    else {
      $parts = explode('.', $key);
      if (count($parts) == 1) {
        return isset($this->data[$key]) ? $this->data[$key] : NULL;
      }
      else {
        $value = $this->data;
        $key_exists = FALSE;
        foreach ($parts as $part) {
          if (is_array($value) && array_key_exists($part, $value)) {
            $value = $value[$part];
            $key_exists = TRUE;
          }
          else {
            $key_exists = FALSE;
            break;
          }
        }
        return $key_exists ? $value : NULL;
      }
    }
  }

  /**
   * Checks if a config value has an override specified.
   *
   * @param string $key
   *   The string that maps to a key with the configuration data.
   *
   *   A key may be overridden at any level, for example if
   *   $key === 'foo.bar.key1', the matched value could be provided by an
   *   override from 'foo.bar.key1', 'foo.bar', or 'foo'. In settings.php any of
   *   the following would provide an overridden value for 'foo.bar.key1':
   *
   *   @code
   *   // Exact match.
   *   $config['example.settings']['foo.bar.key1'] = 'value1';
   *
   *   // One level up.
   *   $config['example.settings']['foo.bar'] = array(
   *     'key1' => 'value1',
   *   );
   *
   *   // A root level override.
   *   $config['example.settings']['foo'] = array(
   *     'bar' => array(
   *       'key1' => 'value1',
   *       'key2' => 'value2',
   *     ),
   *     'baz' => 'other value',
   *   );
   *   @endcode
   *
   * @see Config::get()
   * @see settings.php
   */
  public function getOverride($key) {
    $value = NULL;
    $parts = explode('.', (string) $key);
    $popped_parts = array();
    while ($parts) {
      $assembled_key = implode('.', $parts);
      if (isset($this->overrides[$assembled_key])) {
        // An override is matched at some level.
        $value = $this->overrides[$assembled_key];
        // Drill down back into the override value to get the requested key.
        foreach ($popped_parts as $popped_part) {
          if (isset($value[$popped_part])) {
            $value = $value[$popped_part];
          }
          else {
            $value = NULL;
            $popped_parts = array();
          }
        }
      }
      // If an override was not found at this key, go up a level in the key
      // and continue until there are no more $parts left.
      array_unshift($popped_parts, array_pop($parts));
    }

    return $value;
  }

  /**
   * Gets translated data from this configuration object.
   *
   * @param string $key
   *   A string that maps to a key within the configuration data.
   *   For instance in the following configuration array:
   *   @code
   *   array(
   *     'foo' => array(
   *       'bar' => 'baz',
   *     ),
   *   );
   *   @endcode
   *   A key of 'foo.bar' would return the string 'baz'. However, a key of 'foo'
   *   would return array('bar' => 'baz').
   *   If no key is specified, then an empty string is returned.
   *   If the key is not 'translatable' key then the original string is returned.
   *   If the key's value is not a string then an empty string is returned.
   *
   * @param array $args
   *   An associative array of replacements to make. Replacements are made in
   *   the same way as the t() function.
   * @param array $options
   *   An associative array of additional options, with the following elements:
   *   - 'langcode' (defaults to the current language): The language code to
   *     translate to a language other than what is used to display the page.
   *   - 'context' (defaults to the empty context): The context the source string
   *     belongs to.
   *
   * @return string|NULL
   *   The translated data that was requested.
   *
   * @see t()
   * @see get()
   */
  public function getTranslated($key = '', $args = array(), $options = array()) {
    if (empty($key)) {
      return NULL;
    }
    $value = $this->get($key);
    if (!is_string($value)) {
      return NULL;
    }
    if (!is_array($this->get('_config_translatables'))) {
      return $value;
    }
    // Ensure that it's a translatable key.
    if (!in_array($key, $this->get('_config_translatables'))) {
      return $value;
    }
    // Set a default context so we can differentiate between config strings.
    // Rather than by using isset() or empty(), checking by key existence
    // allows a NULL context to be used.
    if (!array_key_exists('context', $options)) {
      $options['context'] = 'config:' . $this->getName() . ':' . $key;
    }
    return t($value, $args, $options);
  }

  /**
   * Replaces the data of this configuration object.
   *
   * @param array $data
   *   The new configuration data.
   *
   * @return Config
   *   The configuration object.
   */
  public function setData(array $data) {
    $this->replaceData($data);
    // A load would destroy the data just set (for example on import).
    $this->isLoaded = TRUE;
    return $this;
  }

  /**
   * Replaces the data of this configuration object.
   *
   * This function is separate from setData() to avoid load() state tracking.
   * A load() would destroy the replaced data (for example on import). Do not
   * call set() when inside load().
   *
   * @param array $data
   *   The new configuration data.
   *
   * @return Config
   *   The configuration object.
   */
  protected function replaceData(array $data) {
    $this->data = $data;
    $this->validated = FALSE;
    return $this;
  }

  /**
   * Sets a value in this configuration object.
   *
   * Note that this will save a NULL value. If wanting to unset a key from the
   * configuration, use Config::clear($key).
   *
   * @param string $key
   *   Identifier to store value in configuration.
   * @param mixed $value
   *   Value to associate with identifier.
   * @param bool $include_overridden_value
   *   Set to TRUE to write the config value even if this key has been
   *   overridden (usually through settings.php). Overridden keys are ignored
   *   by default to prevent accidentally writing values that may be
   *   environment-specific or contain sensitive information that should not
   *   be written to config.
   *
   * @return Config
   *   The configuration object.
   */
  public function set($key, $value, $include_overridden_value = FALSE) {
    // If setting a value that matches an override-provided one, don't actually
    // write it to config.
    $override_value = $this->getOverride($key);
    if (isset($override_value) && ($override_value === $value) && !$include_overridden_value) {
      return $this;
    }

    if (!$this->isLoaded) {
      $this->load();
    }

    // The dot/period is a reserved character; it may appear between keys, but
    // not within keys.
    $parts = explode('.', $key);
    if (count($parts) == 1) {
      $this->data[$key] = $value;
    }
    else {
      $data = &$this->data;
      $last_key = array_pop($parts);
      foreach ($parts as $part) {
        if (!isset($data)) {
          $data[$part] = array();
        }
        $data = &$data[$part];
      }
      $data[$last_key] = $value;
    }
    $this->validated = FALSE;

    return $this;
  }

  /**
   * Sets multiple values in this configuration object.
   *
   * @param array $options
   *   A keyed array of identifiers mapping to the associated values.
   *
   * @return Config
   *   The configuration object.
   */
  public function setMultiple($options) {
    foreach ($options as $key => $value) {
      $this->set($key, $value);
    }
    return $this;
  }

  /**
   * Sets overrides for this configuration object.
   *
   * @param array $overrides
   *   A list of overrides keyed by strings.
   *
   * @see Config::getOverride()
   */
  public function setOverrides(array $overrides) {
    $this->overrides = $overrides;
  }

  /**
   * Unsets a value in this configuration object.
   *
   * @param string $key
   *   Name of the key whose value should be unset.
   *
   * @return Config
   *   The configuration object.
   */
  public function clear($key) {
    if (!$this->isLoaded) {
      $this->load();
    }
    $parts = explode('.', $key);
    if (count($parts) == 1) {
      unset($this->data[$key]);
    }
    else {
      $data = &$this->data;
      $last_key = array_pop($parts);
      foreach ($parts as $part) {
        $data = &$data[$part];
      }
      unset($data[$last_key]);
    }
    $this->validated = FALSE;
    return $this;
  }

  /**
   * Loads configuration data into this object.
   *
   * @return Config
   *   The configuration object.
   */
  public function load() {
    $this->isLoaded = FALSE;
    $data = $this->storage->read($this->name);
    if ($data === FALSE) {
      $this->isNew = TRUE;
      $this->replaceData(array());
    }
    else {
      $this->isNew = FALSE;
      $this->replaceData($data);
    }
    $this->isLoaded = TRUE;
    return $this;
  }

  /**
   * Saves the configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function save() {
    // Validate the configuration object name before saving.
    static::validateName($this->name);
    if (!$this->isLoaded) {
      $this->load();
    }
    // Ensure config name is saved in the result.
    $this->data = array_merge(array('_config_name' => $this->name), $this->data);
    $this->storage->write($this->name, $this->data);
    $this->isNew = FALSE;

    // Empty static caches of this config file.
    if ($static = &backdrop_static('config')) {
      foreach ($static as $type => $configs) {
        if (array_key_exists($this->name, $configs)) {
          unset($static[$type][$this->name]);
        }
      }
    }

    return $this;
  }

  /**
   * Deletes the configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function delete() {
    $this->data = array();
    $this->storage->delete($this->name);
    $this->isNew = TRUE;
    return $this;
  }

  /**
   * Retrieves the storage used to load and save this configuration object.
   *
   * @return ConfigStorageInterface
   *   The configuration storage object.
   */
  public function getStorage() {
    return $this->storage;
  }
}

/**
 * Defines an interface for configuration storage controllers.
 *
 * Classes implementing this interface allow reading and writing configuration
 * data from and to the storage.
 */
interface ConfigStorageInterface {

  /**
   * Perform any steps needed to create and initialize the storage.  This may
   * include creating directories or database tables, and initializing data
   * structures.
   *
   * This function does not re-initialize already initialized storage.
   *
   * @return bool
   *   TRUE on success, FALSE in case of an error.
   *
   * @throws ConfigStorageException
   */
  public function initializeStorage();

  /**
   * Check that the storage managed by this object is present and functional.
   *
   * @return bool
   *   TRUE on success, FALSE in case of an error.
   */
  public function isInitialized();

  /**
   * Returns whether a configuration object exists.
   *
   * @param string $name
   *   The name of a configuration object to test.
   *
   * @return bool
   *   TRUE if the configuration object exists, FALSE otherwise.
   */
  public function exists($name);

  /**
   * Reads configuration data from the storage.
   *
   * @param string $name
   *   The name of a configuration object to load.
   *
   * @throws ConfigStorageReadException
   *
   * @return array|bool
   *   The configuration data stored for the configuration object name. If no
   *   configuration data exists for the given name, FALSE is returned.
   */
  public function read($name);

  /**
   * Reads configuration data from the storage.
   *
   * @param array $name
   *   List of names of the configuration objects to load.
   *
   * @throws ConfigStorageException
   *
   * @return array
   *   A list of the configuration data stored for the configuration object name
   *   that could be loaded for the passed list of names.
   */
  public function readMultiple(array $names);

  /**
   * Writes configuration data to the storage.
   *
   * @param string $name
   *   The name of a configuration object to save.
   * @param array $data
   *   The configuration data to write.
   *
   * @return bool
   *   TRUE on success, FALSE in case of an error.
   */
  public function write($name, array $data);

  /**
   * Deletes a configuration object from the storage.
   *
   * @param string $name
   *   The name of a configuration object to delete.
   *
   * @return bool
   *   TRUE on success, FALSE otherwise.
   */
  public function delete($name);

  /**
   * Renames a configuration object in the storage.
   *
   * @param string $name
   *   The name of a configuration object to rename.
   * @param string $new_name
   *   The new name of a configuration object.
   *
   * @return bool
   *   TRUE on success, FALSE otherwise.
   */
  public function rename($name, $new_name);


  /**
   * Returns a timestamp indicating the last time a configuration was modified.
   *
   * @param string $name
   *   The name of a configuration object on which the time will be checked.
   *
   * @return int|false
   *   A timestamp indicating the last time the configuration was modified or
   *   FALSE if the named configuration object doesn't exist.
   */
  public function getModifiedTime($name);

  /**
   * Encodes configuration data into the storage-specific format.
   *
   * @param array $data
   *   The configuration data to encode.
   *
   * @return string
   *   The encoded configuration data.
   *
   *   This is a publicly accessible static method to allow for alternative
   *   usages in data conversion scripts and also tests.
   */
  public function encode($data);

  /**
   * Decodes configuration data from the storage-specific format.
   *
   * @param string $raw
   *   The raw configuration data string to decode.
   *
   * @return array
   *   The decoded configuration data as an associative array.
   *
   *   This is a publicly accessible static method to allow for alternative
   *   usages in data conversion scripts and also tests.
   */
  public function decode($raw);

  /**
   * Gets configuration object names starting with a given prefix.
   *
   * Given the following configuration objects:
   * - node.type.post
   * - node.type.page
   *
   * Passing the prefix 'node.type.' will return an array containing the above
   * names.
   *
   * @param string $prefix
   *   (optional) The prefix to search for. If omitted, all configuration object
   *   names that exist are returned.
   *
   * @return array
   *   An array containing matching configuration object names.
   */
  public function listAll($prefix = '');

  /**
   * Deletes configuration objects whose names start with a given prefix.
   *
   * Given the following configuration object names:
   * - node.type.post
   * - node.type.page
   *
   * Passing the prefix 'node.type.' will delete the above configuration
   * objects.
   *
   * @param string $prefix
   *   (optional) The prefix to search for. If omitted, all configuration
   *   objects that exist will be deleted.
   *
   * @return bool
   *   TRUE on success, FALSE otherwise.
   */
  public function deleteAll($prefix = '');

  /**
   * Import an archive of configuration files into the config storage managed
   * by this object.
   *
   * @param string $file_uri
   *   The URI of the tar archive file to import.
   *
   * @throws ConfigStorageException
   *
   * @return bool
   *   TRUE on success, FALSE otherwise.
   */
  public function importArchive($file_uri);

  /**
   * Export an archive of configuration files from the config storage managed
   * by this object.
   *
   * @param string $file_uri
   *   The URI of the tar archive file to create.
   *
   * @throws ConfigStorageException
   *
   * @return bool
   *   TRUE on success, FALSE otherwise.
   */
  public function exportArchive($file_uri);
}

/**
 * Defines the database storage controller.
 */
class ConfigDatabaseStorage implements ConfigStorageInterface {

  /**
   * The database table to use for configuration objects.
   *
   * @var string
   */
  protected $table = '';

  /**
   * The database connection to use.
   *
   * @var string
   */
  protected $database = '';

  /**
   * The database schema.
   */
  protected function schema($name) {
    $schema = array(
      'description' => 'Stores the configuration of the site in database tables.',
      'fields' => array(
        'name' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
          'description' => 'The top-level name for the full key.  Would be the filename in the ConfigFileStore, minus the .json extension',
        ),
        'data' => array(
          'type' => 'text',
          'size' => 'big',
          'not null' => TRUE,
          'description' => 'The JSON encoded value for the key.',
        ),
        'ctime' => array(
          'type' => 'int',
          'not null' => TRUE,
          'default' => 0,
          'description' => 'The node changed time (see stat(2) for details.)',
        ),
      ),
      'primary key' => array('name'),
    );

    return $schema;
  }

  /**
   * Constructs a new ConfigDatabaseStorage controller.
   *
   * @param string $dburl
   *   A URL that references a backdrop connection (optional) and table. The
   *   string has a format of db:/<database connection>/<database table>.
   *
   *   Examples:
   *    * db:/default/config_active
   *    * db:config_active
   *    * db://config_active
   *    * db:/config_store/config_stage
   *
   *   If no database connection is specified, then the 'default' connection is
   *   used.
   */
  public function __construct($dburl) {
    $matches = array();
    preg_match('/^db:(\/(\w*)\/)?(\w+)$/', trim($dburl), $matches);
    if (count($matches) != 4) {
      throw new ConfigStorageException(t('Invalid database specifier: @db', array('@db' => $dburl)));
    }
    $this->table = $matches[3];
    $this->database = $matches[2] ? $matches[2] : 'default';

    // Bootstrap the database if it is not yet available.
    if (!function_exists('db_query') || backdrop_get_bootstrap_phase() < BACKDROP_BOOTSTRAP_DATABASE) {
      require_once BACKDROP_ROOT . '/core/includes/database/database.inc';
      backdrop_bootstrap(BACKDROP_BOOTSTRAP_DATABASE, FALSE);
    }
  }

  /**
   * Create the database table it does not already exist.
   *
   * @return bool
   *   TRUE on success, FALSE in case of an error.
   *
   * @throws ConfigStorageException
   */
  public function initializeStorage() {
    // db_table_exists() does not prefix our tables, so we have to do it
    // manually when we do this check.
    $connection_info = Database::getConnectionInfo($this->database);
    $prefix = $connection_info[$this->database]['prefix']['default'];
    if (!db_table_exists($prefix . $this->table)) {
      try {
        db_create_table($this->table, $this->schema($this->table));
      }
      catch (\Exception $e) {
        throw new ConfigStorageException(format_string('The database table %table could not be created: @error', array(
          '%table' => $this->table,
          '@error' => $e->getMessage(),
        )), 0, $e);
      }
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function isInitialized() {
    return db_table_exists($this->table, array('target' => $this->database));
  }

  /**
   * {@inheritdoc}
   */
  public function exists($name) {
    try {
      $query = db_select($this->table, 'c', array('target' => $this->database))
        ->condition('c.name', $name);
      $query->addExpression('1');
      $value = $query->execute()
        ->fetchField();
    }
    catch (\Exception $e) {
      // Happens where there is no database.  Return FALSE.
      $value = FALSE;
    }

    return $value ? TRUE : FALSE;
  }

  /**
   * {@inheritdoc}
   */
  public function read($name) {
    if (!$this->exists($name)) {
      return FALSE;
    }
    $data = db_select($this->table, 'c', array('target' => $this->database))
      ->fields('c', array('data'))
      ->condition('c.name', $name)
      ->execute()
      ->fetchField();
    try {
      $data = $this->decode($data);
      // Remove the config name from the read configuration.
      if (isset($data['_config_name'])) {
        unset($data['_config_name']);
      }
    }
    // If an error occurs, catch and rethrow with the file name in the message.
    catch (ConfigStorageException $e) {
      throw new ConfigStorageReadException(format_string("The configuration file \"@filename\" is not properly formatted JSON.\n\nContents:\n<pre>@contents</pre>\n", array(
        '@filename' => $name,
        '@contents' => $data,
      )));
    }
    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function readMultiple(array $names) {
    $list = array();
    foreach ($names as $name) {
      if ($data = $this->read($name)) {
        $list[$name] = $data;
      }
    }
    return $list;
  }

  /**
   * {@inheritdoc}
   */
  public function write($name, array $data) {
    // Ensure that the config name is included in the written file.
    $data = array_merge(array('_config_name' => $name), $data);
    $data = $this->encode($data) . "\n";
    try {
      $result = db_merge($this->table, array('target' => $this->database))
        ->key(array('name' => $name))
        ->fields(array(
          'name' => $name,
          'data' => $data,
        ))
        ->execute();
      if ($result == MergeQuery::STATUS_INSERT) {
        db_update($this->table, array('target' => $this->database))
          ->fields(array('ctime' => time()))
          ->condition('name', $name)
          ->condition('ctime', 0)
          ->execute();
      }
    }
    catch (\Exception $e) {
      throw new ConfigStorageException('Failed to write configuration to the database: ' . $this->$name, 0, $e);
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function delete($name) {
    if (!$this->exists($name)) {
      return FALSE;
    }
    db_delete($this->table, array('target' => $this->database))
      ->condition('name', $name)
      ->execute();
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function rename($name, $new_name) {
    try {
      db_delete($this->table, array('target' => $this->database))
        ->condition('name', $new_name)
        ->execute();
      db_update($this->table, array('target' => $this->database))
        ->fields(array('name' => $new_name))
        ->condition('name', $name)
        ->execute();
    }
    catch (\Exception $e) {
      throw new ConfigStorageException('Failed to rename configuration from: ' . $name . ' to: ' . $new_name, 0, $e);
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function getModifiedTime($name) {
    $data = db_select($this->table, 'c', array('target' => $this->database))
      ->fields('c', array('ctime'))
      ->condition('c.name', $name)
      ->execute()
      ->fetchField();

    return empty($data) ? FALSE : $data;
  }

  /**
   * {@inheritdoc}
   */
  public function encode($data) {
    $contents = backdrop_json_encode($data, TRUE);
    if ($contents === FALSE) {
      throw new ConfigStorageException(t('The configuration string could not be parsed.'));
    }
    return $contents;
  }

  /**
   * {@inheritdoc}
   */
  public function decode($raw) {
    // Use json_decode() directly for efficiency.
    $contents = json_decode($raw, TRUE);
    if (is_null($contents)) {
      throw new ConfigStorageException('The configuration string could not be parsed.');
    }
    return $contents;
  }

  /**
   * {@inheritdoc}
   */
  public function listAll($prefix = '') {
    $results = db_select($this->table, 'c', array('target' => $this->database))
      ->fields('c', array('name'))
      ->condition('c.name', $prefix . '%', 'LIKE')
      ->execute()
      ->fetchAllAssoc('name', PDO::FETCH_ASSOC);
    return array_column($results, 'name');
  }

  /**
   * {@inheritdoc}
   */
  public function deleteAll($prefix = '') {
    $query = db_delete($this->table, array('target' => $this->database));
    if ($prefix) {
      $query->condition('name', $prefix . '%', 'LIKE');
    }
    $query->execute();
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function importArchive($file_uri) {
    $realpath = backdrop_realpath($file_uri);

    try {
      $archiver = new ArchiverTar($realpath);
      $archiver->extract(file_directory_temp(), $archiver->listContents());
    }
    catch (\Exception $e) {
      watchdog('config', 'Could not extract the archive @uri: @error', array(
        '@uri' => $file_uri,
        '@error' => $e->getMessage(),
      ), WATCHDOG_ERROR);
      throw new ConfigStorageException($e->getMessage(), 0, $e);
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function exportArchive($archive_file_uri) {
    $tmp_config_dir_path = file_create_filename('config', file_directory_temp());
    file_prepare_directory($tmp_config_dir_path, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
    $data = db_select($this->table, 'c', array('target' => $this->database))
      ->fields('c')
      ->execute()
      ->fetchAllAssoc('name', PDO::FETCH_ASSOC);
    foreach($data as $config_name => $row) {
      file_put_contents($tmp_config_dir_path . '/' . $config_name . '.json', $row['data']);
    }

    // And tar it up.
    $realpath = backdrop_realpath($archive_file_uri);
    $archiver = new ArchiverTar($archive_file_uri);
    $config_files = array();
    foreach (array_keys($data) as $config_name) {
      $config_files[] = $tmp_config_dir_path . '/' . $config_name . '.json';
    }
    $archiver->getArchive()->createModify($config_files, '', $tmp_config_dir_path);
  }
}

/**
 * Defines the file storage controller.
 */
class ConfigFileStorage implements ConfigStorageInterface {

  /**
   * The filesystem path for configuration objects.
   *
   * @var string
   */
  protected $directory = '';

  /**
   * Constructs a new FileStorage controller.
   *
   * @param string $directory
   *   A directory path to use for reading and writing of configuration files.
   */
  public function __construct($directory) {
    $this->directory = $directory;
  }

  /**
   * Create a configuration directory, if it does not already exist, and ensure
   * it is writable by the site. Additionally, protect it with a .htaccess file.
   *
   * @return bool
   *   TRUE on success, FALSE in case of an error.
   *
   * @throws ConfigStorageException
   */
  public function initializeStorage() {
    if (!file_prepare_directory($this->directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS)) {
      throw new ConfigStorageException(format_string('The directory <code>@directory</code> could not be created or could not be made writable.', array(
        '@directory' => $this->directory,
      )));
    }
    // No need to create htaccess files if not running Apache.
    if (backdrop_is_apache()) {
      file_save_htaccess($this->directory);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function isInitialized() {
    return is_dir($this->directory);
  }

  /**
   * Returns the path to the configuration file.
   *
   * @return string
   *   The path to the configuration file.
   */
  public function getFilePath($name) {
    return $this->directory . '/' . $name . '.json';
  }

  /**
   * {@inheritdoc}
   */
  public function exists($name) {
    return file_exists($this->getFilePath($name));
  }

  /**
   * {@inheritdoc}
   */
  public function read($name) {
    if (!$this->exists($name)) {
      return FALSE;
    }
    $data = file_get_contents($this->getFilePath($name));
    try {
      $data = $this->decode($data);
      // Remove the config name from the read configuration.
      if (isset($data['_config_name'])) {
        unset($data['_config_name']);
      }
    }
    // If an error occurs, catch and rethrow with the file name in the message.
    catch (ConfigStorageException $e) {
      throw new ConfigStorageReadException(format_string("The configuration file \"@filename\" is not properly formatted JSON.\n\nContents:\n<pre>@contents</pre>\n", array(
        '@filename' => $name,
        '@contents' => $data,
      )));
    }
    return $data;
  }

  /**
   * {@inheritdoc}
   */
  public function readMultiple(array $names) {
    $list = array();
    foreach ($names as $name) {
      if ($data = $this->read($name)) {
        $list[$name] = $data;
      }
    }
    return $list;
  }

  /**
   * {@inheritdoc}
   */
  public function write($name, array $data) {
    // Ensure that the config name is included in the written file.
    $data = array_merge(array('_config_name' => $name), $data);
    $data = $this->encode($data) . "\n";
    $file_path = $this->getFilePath($name);
    $status = @file_put_contents($file_path, $data);
    if ($status === FALSE) {
      throw new ConfigStorageException('Failed to write configuration file: ' . $this->getFilePath($name));
    }
    clearstatcache(FALSE, $file_path);
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function delete($name) {
    if (!$this->exists($name)) {
      if (!file_exists($this->directory)) {
        throw new ConfigStorageException($this->directory . '/ not found.');
      }
      return FALSE;
    }
    $file_path = $this->getFilePath($name);
    $status = backdrop_unlink($file_path);
    clearstatcache(FALSE, $file_path);
    return $status;
  }

  /**
   * {@inheritdoc}
   */
  public function rename($name, $new_name) {
    $status = @rename($this->getFilePath($name), $this->getFilePath($new_name));
    if ($status === FALSE) {
      throw new ConfigStorageException('Failed to rename configuration file from: ' . $this->getFilePath($name) . ' to: ' . $this->getFilePath($new_name));
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function getModifiedTime($name) {
    return filectime($this->getFilePath($name));
  }

  /**
   * {@inheritdoc}
   */
  public function encode($data) {
    $contents = backdrop_json_encode($data, TRUE);
    if ($contents === FALSE) {
      throw new ConfigStorageException(t('The configuration string could not be parsed.'));
    }
    return $contents;
  }

  /**
   * {@inheritdoc}
   */
  public function decode($raw) {
    // Use json_decode() directly for efficiency.
    $contents = json_decode($raw, TRUE);
    if (is_null($contents)) {
      throw new ConfigStorageException('The configuration string could not be parsed.');
    }
    return $contents;
  }

  /**
   * {@inheritdoc}
   */
  public function listAll($prefix = '') {
    // glob() silently ignores the error of a non-existing search directory,
    // even with the GLOB_ERR flag.
    if (!file_exists($this->directory)) {
      throw new ConfigStorageException($this->directory . '/ not found.');
    }
    $extension = '.json';
    $files = glob($this->directory . '/' . $prefix . '*' . $extension);
    $clean_name = function ($value) use ($extension) {
      return basename($value, $extension);
    };
    return array_map($clean_name, $files);
  }

  /**
   * {@inheritdoc}
   */
  public function deleteAll($prefix = '') {
    $success = TRUE;
    $files = $this->listAll($prefix);
    foreach ($files as $name) {
      if (!$this->delete($name) && $success) {
        $success = FALSE;
      }
    }

    return $success;
  }

  /**
   * {@inheritdoc}
   */
  public function importArchive($file_uri) {
    $realpath = backdrop_realpath($file_uri);

    try {
      $archiver = new ArchiverTar($realpath);
      // Only extract JSON files, ignoring anything else in the archive.
      $file_list = preg_grep('/.json$/', $archiver->listContents());
      if ($file_list) {
        $archiver->extract($this->directory, $file_list);
      }
    }
    catch (\Exception $e) {
      watchdog('config', 'Could not extract the archive @uri: @error', array('@uri' => $file_uri, '@error' => $e->getMessage()), WATCHDOG_ERROR);
      throw new ConfigStorageException($e->getMessage(), 0, $e);
    }
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function exportArchive($file_uri) {
    $archiver = new ArchiverTar($file_uri);
    $config_files = array();
    foreach ($this->listAll() as $config_name) {
      $config_files[] = $this->directory . '/' . $config_name . '.json';
    }
    $archiver->getArchive()->createModify($config_files, '', $this->directory);
  }
}
