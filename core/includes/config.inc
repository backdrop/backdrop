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
 *   (optional) The type of config directory to return. Drupal core provides
 *   'active' and 'staging'. Defaults to 'active'.
 *
 * @return
 *   An instance of the class specified in the $class parameter.
 *
 */
function config($config_file, $type = 'active') {
  $directory = config_get_config_directory($type);
  $storage = new ConfigFileStorage($directory);
  return new Config($config_file, $storage);
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
 *   may contain periods to indicate levels within the config file.
 *
 * @return mixed
 *   The contents of the requested config option. Returns NULL if the specified
 *   option was not found in the file at all.
 */
function config_get($config_file, $option) {
  $config = config($config_file);
  return $config->get($option);
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
 * @return mixed
 *   The contents of the requested config option. Returns NULL if the specified
 *   option was not found in the file at all.
 */
function config_set($config_file, $option, $value) {
  $config = new config($config_file);
  $config->set($option, $value);
  $config->save();
}


/**
 * Returns the path of a configuration directory.
 *
 * @param string $type
 *   (optional) The type of config directory to return. Drupal core provides
 *   'active' and 'staging'. Defaults to 'active'.
 *
 * @return string
 *   The configuration directory path.
 */
function config_get_config_directory($type = 'active') {
  global $config_directories;

  if (!empty($config_directories[$type])) {
    // Allow a configuration directory path to be outside of webroot.
    if (empty($config_directories[$type]['absolute'])) {
      $path = conf_path() . '/files/' . $config_directories[$type]['path'];
    }
    else {
      $path = $config_directories[$type]['path'];
    }
  }
  else {
    throw new Exception(format_string('The configuration directory type "@type" does not exist.', array('@type' => $type)));
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
  $directory = config_get_config_directory($type);
  $storage = new ConfigFileStorage($directory);
  return $storage->listAll($prefix);
}

/**
 * Moves the default config supplied by a module to the live config directory.
 *
 * @param
 *   The name of the module we are installing.
 */
function config_install_default_config($module) {
  $module_config_dir = drupal_get_path('module', $module) . '/config';
  $storage = new ConfigFileStorage($module_config_dir);
  if (is_dir(drupal_get_path('module', $module) . '/config')) {
    $files = glob($module_config_dir . '/' . '*.json');
    foreach ($files as $file) {
      // Load config data into the active store and write it out to the
      // file system in the drupal config directory. Note the config name
      // needs to be the same as the file name WITHOUT the extension.
      $parts = explode('/', $file);
      $file = array_pop($parts);
      $config_name = str_replace('.json', '', $file);

      $data = $storage->read($config_name);
      $config = config($config_name);
      $config->setData($data);
      $config->save();
    }
  }
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
 */
class ConfigValidateException extends ConfigException {}

/**
 * @todo
 */
class ConfigStorageException extends ConfigException {}

/**
 * @todo
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
    $this->name = $name;
    $this->storage = $storage;
  }

  /**
   * Initializes a configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function init() {
    $this->isLoaded = FALSE;
    $this->notify('init');
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
    $this->notify('init');
    $this->replaceData($data);
    $this->notify('load');
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
   *  The name of the configuration object.
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
   *  The name of the configuration object.
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
    // : ? * < > " ' / \
    if (preg_match('/[:?*<>"\'\/\\\\]/', $name)) {
      throw new ConfigNameException(format_string('Invalid character in Config object name @name.', array(
        '@name' => $name,
      )));
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
          if ($value[$part]) {
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
    return $this;
  }

  /**
   * Sets a value in this configuration object.
   *
   * @param string $key
   *   Identifier to store value in configuration.
   * @param mixed $value
   *   Value to associate with identifier.
   *
   * @return Config
   *   The configuration object.
   */
  public function set($key, $value) {
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
    return $this;
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
    $this->notify('load');
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
    $this->notify('save');
    return $this;
  }

  /**
   * Deletes the configuration object.
   *
   * @return Config
   *   The configuration object.
   */
  public function delete() {
    // @todo Consider to remove the pruning of data for Config::delete().
    $this->data = array();
    $this->storage->delete($this->name);
    $this->isNew = TRUE;
    $this->notify('delete');
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

  /**
   * Dispatches a configuration event.
   *
   * @param string $config_event_name
   *   The configuration event name.
   */
  protected function notify($config_event_name) {
    // @todo: Add a drupal_alter() or hook here?
    //$this->context->notify($config_event_name, $this);
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
   * @return int
   *   A timestamp indicating the last time the configuration was modified.
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
   * This is a publicly accessible static method to allow for alternative
   * usages in data conversion scripts and also tests.
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
   * This is a publicly accessible static method to allow for alternative
   * usages in data conversion scripts and also tests.
   */
  public function decode($raw);

  /**
   * Gets configuration object names starting with a given prefix.
   *
   * Given the following configuration objects:
   * - node.type.article
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
   * - node.type.article
   * - node.type.page
   *
   * Passing the prefix 'node.type.' will delete the above configuration
   * objects.
   *
   * @param string $prefix
   *   (optional) The prefix to search for. If omitted, all configuration
   *   objects that exist will be deleted.
   *
   * @return boolean
   *   TRUE on success, FALSE otherwise.
   */
  public function deleteAll($prefix = '');

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
      // Ensure config name is loaded the result.
      $data = array_merge(array('_config_name' => $name), $this->decode($data));
    }
    // If an error occurs, catch and rethrow with the file name in the message.
    catch (ConfigStorageException $e) {
      throw new ConfigStorageReadException(format_string('The configuration file "@filename" could not be read.', array('@filename' => $name)));
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
    $data = $this->encode($data);
    $status = @file_put_contents($this->getFilePath($name), $data);
    if ($status === FALSE) {
      throw new ConfigStorageException('Failed to write configuration file: ' . $this->getFilePath($name));
    }
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
    return drupal_unlink($this->getFilePath($name));
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
    $contents = drupal_json_encode($data, TRUE);
    if ($contents === FALSE) {
      throw new ConfigStorageException(t('The configuration string could not be parsed.'));
    }
    return $contents;
  }

  /**
   * {@inheritdoc}
   */
  public function decode($raw) {
    $contents = drupal_json_decode($raw);
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
}
