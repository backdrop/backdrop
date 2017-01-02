<?php

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
      $data = $this->decode($data);
      // Remove the config name from the read configuration.
      if (isset($data['_config_name'])) {
        unset($data['_config_name']);
      }
    }
    // If an error occurs, catch and rethrow with the file name in the message.
    catch (ConfigStorageException $e) {
      throw new ConfigStorageReadException(format_string("The configuration file \"@filename\" is not properly formatted JSON.\n\nContents:\n<pre>@contents</pre>\n", array('@filename' => $name, '@contents' => $data)));
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
}
