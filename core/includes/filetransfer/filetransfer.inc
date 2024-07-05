<?php
/**
 * @file
 * Base FileTransfer class.
 *
 * Classes extending this class perform file operations on directories not
 * writable by the webserver. To achieve this, the class should connect back
 * to the server using some backend (for example FTP or SSH). To keep security,
 * the password should always be asked from the user and never stored. For
 * safety, all methods operate only inside a "jail", by default the Backdrop
 * root.
 */
abstract class FileTransfer {

  /**
   * The username for this file transfer.
   *
   * @var string
   */
  protected $username;

  /**
   * The password for this file transfer.
   *
   * @var string
   */
  protected $password;

  /**
   * The hostname for this file transfer.
   *
   * @var string
   */
  protected $hostname = 'localhost';

  /**
   * The port for this file transfer.
   *
   * @var int
   */
  protected $port;

  /**
   * Full path to directory where file-transfer is restricted to.
   *
   * @var string
   */
  protected $jail;

  /**
   * Path to connection chroot.
   *
   * @var string|false|null
   */
  private $chrootPath;

  /**
   * The instantiated connection object.
   *
   * @var object|false|null
   */
  private $connectionHandle;

  /**
   * The constructor for the FileTransfer class.
   *
   * This method is also called from the classes that extend this class and
   * override this method.
   *
   * @param string $jail
   *   The full path where all file operations performed by this object will
   *   be restricted to. This prevents the FileTransfer classes from being
   *   able to touch other parts of the filesystem.
   */
  function __construct($jail) {
    $this->jail = $jail;
  }

  /**
   * Defines a factory method for this class.
   *
   * Classes that extend this class must override the factory() static method.
   * They should return a new instance of the appropriate FileTransfer subclass.
   *
   * @param string $jail
   *   The full path where all file operations performed by this object will
   *   be restricted to. This prevents the FileTransfer classes from being
   *   able to touch other parts of the filesystem.
   * @param array $settings
   *   An array of connection settings for the FileTransfer subclass. If the
   *   getSettingsForm() method uses any nested settings, the same structure
   *   will be assumed here.
   *
   * @return object
   *   New instance of the appropriate FileTransfer subclass.
   *
   * @throws FileTransferException
   */
  static function factory($jail, $settings) {
    throw new FileTransferException('FileTransfer::factory() static method not overridden by FileTransfer subclass.');
  }

  /**
   * Implementation of the magic __get() method.
   *
   * If the connection isn't set to anything, this will call the connect() method
   * and set it to and return the result; afterwards, the connection will be
   * returned directly without using this method.
   *
   * @param string $name
   *   The name of the variable to return.
   *
   * @return string|bool
   *   The variable specified in $name.
   */
  function __get($name) {
    if ($name == 'connection') {
      $this->connect();
      return $this->connectionHandle;
    }

    if ($name == 'chroot') {
      $this->setChroot();
      return $this->chrootPath;
    }
  }

  /**
   * Implementation of the magic __set() method.
   *
   * @param string $name
   *   The name of the variable to set.
   *
   * @param string|object|false|null $value
   *   The value to be assigned to the variable.
   */
  public function __set($name, $value) {
    if ($name == 'connection') {
      $this->connectionHandle = $value;
    }
    elseif ($name == 'chroot') {
      $this->chrootPath = $value;
    }
  }

  /**
   * Implementation of the magic __isset() method.
   *
   * @param string $name
   *   The name of the variable to check.
   */
  public function __isset($name) {
    if ($name == 'connection') {
      return isset($this->connectionHandle);
    }
    if ($name == 'chroot') {
      return isset($this->chrootPath);
    }
    return FALSE;
  }

  /**
   * Implementation of the magic __unset() method.
   *
   * @param string $name
   *   The name of the variable to unset.
   */
  public function __unset($name) {
    if ($name == 'connection') {
      unset($this->connectionHandle);
    }
    elseif ($name == 'chroot') {
      unset($this->chrootPath);
    }
  }

  /**
   * Connects to the server.
   */
  abstract public function connect();

  /**
   * Copies a directory.
   *
   * @param $source
   *   The source path.
   * @param $destination
   *   The destination path.
   */
  final public function copyDirectory($source, $destination) {
    $source = $this->sanitizePath($source);
    $destination = $this->fixRemotePath($destination);
    $this->checkPath($destination);
    $this->copyDirectoryJailed($source, $destination);
  }

  /**
   * Changes the permissions of the specified $path (file or directory).
   *
   * @param string $path
   *   The file / directory to change the permissions of.
   * @param int $mode
   *   The new file permission mode to be passed to chmod().
   * @param bool $recursive
   *   Pass TRUE to recursively chmod the entire directory specified in $path.
   *
   * @throws FileTransferException
   *
   * @see http://php.net/chmod
   */
  final public function chmod($path, $mode, $recursive = FALSE) {
    if (!in_array('FileTransferChmodInterface', class_implements(get_class($this)))) {
      throw new FileTransferException('Unable to change file permissions');
    }
    $path = $this->sanitizePath($path);
    $path = $this->fixRemotePath($path);
    $this->checkPath($path);
    $this->chmodJailed($path, $mode, $recursive);
  }

  /**
   * Creates a directory.
   *
   * @param $directory
   *   The directory to be created.
   */
  public final function createDirectory($directory) {
    $directory = $this->fixRemotePath($directory);
    $this->checkPath($directory);
    $this->createDirectoryJailed($directory);
  }

  /**
   * Removes a directory.
   *
   * @param $directory
   *   The directory to be removed.
   */
  public final function removeDirectory($directory) {
    $directory = $this->fixRemotePath($directory);
    $this->checkPath($directory);
    $this->removeDirectoryJailed($directory);
  }

  /**
   * Copies a file.
   *
   * @param $source
   *   The source file.
   * @param $destination
   *   The destination file.
   */
  public final function copyFile($source, $destination) {
    $source = $this->sanitizePath($source);
    $destination = $this->fixRemotePath($destination);
    $this->checkPath($destination);
    $this->copyFileJailed($source, $destination);
  }

  /**
   * Removes a file.
   *
   * @param $destination
   *   The destination file to be removed.
   */
  public final function removeFile($destination) {
    $destination = $this->fixRemotePath($destination);
    $this->checkPath($destination);
    $this->removeFileJailed($destination);
  }

  /**
   * Checks that the path is inside the jail and throws an exception if not.
   *
   * @param $path
   *   A path to check against the jail.
   *
   * @throws FileTransferException
   */
  protected final function checkPath($path) {
    $full_jail = $this->chroot . $this->jail;
    $full_path = backdrop_realpath(substr($this->chroot . $path, 0, strlen($full_jail)));
    $full_path = $this->fixRemotePath($full_path, FALSE);
    if ($full_jail !== $full_path) {
      throw new FileTransferException('@directory is outside of the @jail', NULL, array('@directory' => $path, '@jail' => $this->jail));
    }
  }

  /**
   * Returns a modified path suitable for passing to the server.
   *
   * If a path is a windows path, makes it POSIX compliant by removing the drive letter.
   * If $this->chroot has a value, it is stripped from the path to allow for
   * chroot'd filetransfer systems.
   *
   * @param $path
   *   The path to modify.
   * @param $strip_chroot
   *   Whether to remove the path in $this->chroot.
   *
   * @return string
   *   The modified path.
   */
  protected final function fixRemotePath($path, $strip_chroot = TRUE) {
    $path = $this->sanitizePath($path);
    // Strip out Windows drive letter if it's there.
    $path = preg_replace('|^([a-z]{1}):|i', '', $path);
    if ($strip_chroot) {
      if ($this->chrootPath && strpos($path, $this->chrootPath) === 0) {
        $path = ($path == $this->chrootPath) ? '' : substr($path, strlen($this->chrootPath));
      }
    }
    return $path;
  }

  /**
  * Changes backslashes to slashes, also removes a trailing slash.
  *
  * @param string $path
  * @return string
  */
  function sanitizePath($path) {
    $path = str_replace('\\', '/', $path); // Windows path sanitization.
    if (substr($path, -1) == '/') {
      $path = substr($path, 0, -1);
    }
    return $path;
  }

  /**
   * Copies a directory.
   *
   * We need a separate method to make the $destination is in the jail.
   *
   * @param $source
   *   The source path.
   * @param $destination
   *   The destination path.
   */
  protected function copyDirectoryJailed($source, $destination) {
    if ($this->isDirectory($destination)) {
      $destination = $destination . '/' . backdrop_basename($source);
    }
    $this->createDirectory($destination);
    $directory_iterator = new RecursiveDirectoryIterator($source, FilesystemIterator::SKIP_DOTS);
    $recursive_iterator = new RecursiveIteratorIterator($directory_iterator, RecursiveIteratorIterator::SELF_FIRST);
    foreach ($recursive_iterator as $filename => $file) {
      $relative_path = substr($filename, strlen($source));
      if ($file->isDir()) {
        $this->createDirectory($destination . $relative_path);
      }
      else {
        $this->copyFile($file->getPathName(), $destination . $relative_path);
      }
    }
  }

  /**
   * Creates a directory.
   *
   * @param $directory
   *   The directory to be created.
   */
  abstract protected function createDirectoryJailed($directory);

  /**
   * Removes a directory.
   *
   * @param $directory
   *   The directory to be removed.
   */
  abstract protected function removeDirectoryJailed($directory);

  /**
   * Copies a file.
   *
   * @param $source
   *   The source file.
   * @param $destination
   *   The destination file.
   */
  abstract protected function copyFileJailed($source, $destination);

  /**
   * Removes a file.
   *
   * @param $destination
   *   The destination file to be removed.
   */
  abstract protected function removeFileJailed($destination);

  /**
   * Checks if a particular path is a directory
   *
   * @param $path
   *   The path to check
   *
   * @return boolean
   */
  abstract public function isDirectory($path);

  /**
   * Checks if a particular path is a file (not a directory).
   *
   * @param $path
   *   The path to check
   *
   * @return boolean
   */
  abstract public function isFile($path);

  /**
   * Returns the chroot property for this connection.
   *
   * It does this by moving up the tree until it finds itself. If successful,
   * it will return the chroot, otherwise FALSE.
   *
   * @return
   *   The chroot path for this connection or FALSE.
   */
  function findChroot() {
    // If the file exists as is, there is no chroot.
    $path = __FILE__;
    $path = $this->fixRemotePath($path, FALSE);
    if ($this->isFile($path)) {
      return FALSE;
    }

    $path = __DIR__;
    $path = $this->fixRemotePath($path, FALSE);
    $parts = explode('/', $path);
    $chroot = '';
    while (count($parts)) {
      $check = implode('/', $parts);
      if ($this->isFile($check . '/' . backdrop_basename(__FILE__))) {
        // Remove the trailing slash.
        return substr($chroot, 0, -1);
      }
      $chroot .= array_shift($parts) . '/';
    }
    return FALSE;
  }

  /**
   * Sets the chroot and changes the jail to match the correct path scheme
   *
   */
  function setChroot() {
    $this->chroot = $this->findChroot();
    $this->jail = $this->fixRemotePath($this->jail);
  }

  /**
   * Returns a form to collect connection settings credentials.
   *
   * Implementing classes can either extend this form with fields collecting the
   * specific information they need, or override it entirely.
   *
   * @return array
   *   An array that contains a Form API definition.
   */
  public function getSettingsForm() {
    $form['username'] = array(
      '#type' => 'textfield',
      '#title' => t('Username'),
    );
    $form['password'] = array(
      '#type' => 'password',
      '#title' => t('Password'),
      '#description' => t('Your password is not saved in the database and is only used to establish a connection.'),
      '#password_toggle' => TRUE,
    );
    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['advanced']['hostname'] = array(
      '#type' => 'textfield',
      '#title' => t('Host'),
      '#default_value' => 'localhost',
      '#description' => t('The connection will be created between your web server and the machine hosting the web server files. In the vast majority of cases, this will be the same machine, and "localhost" is correct.'),
    );
    $form['advanced']['port'] = array(
      '#type' => 'textfield',
      '#title' => t('Port'),
      '#default_value' => NULL,
    );
    return $form;
  }
}

/**
 * FileTransferException class.
 */
class FileTransferException extends Exception {
  public $arguments;

  function __construct($message, $code = 0, $arguments = array()) {
    parent::__construct($message, (int) $code);
    $this->arguments = $arguments;
  }
}


/**
 * A FileTransfer Class implementing this interface can be used to chmod files.
 */
interface FileTransferChmodInterface {

  /**
   * Changes the permissions of the file / directory specified in $path
   *
   * @param string $path
   *   Path to change permissions of.
   * @param int $mode
   *   The new file permission mode to be passed to chmod().
   * @param boolean $recursive
   *   Pass TRUE to recursively chmod the entire directory specified in $path.
   */
  function chmodJailed($path, $mode, $recursive);
}

/**
 * Provides an interface for iterating recursively over filesystem directories.
 *
 * Manually skips '.' and '..' directories, since no existing method existed in
 * PHP versions prior to 5.3.
 *
 * Instead of using this class, use RecursiveDirectoryIterator directly and
 * pass the FilesystemIterator::SKIP_DOTS option.
 *
 * @deprecated since 1.22.0
 */
class SkipDotsRecursiveDirectoryIterator extends RecursiveDirectoryIterator {
  /**
   * Constructs a SkipDotsRecursiveDirectoryIterator
   *
   * @param $path
   *   The path of the directory to be iterated over.
   */
  function __construct($path) {
    watchdog_deprecated_function('filetransfer', __METHOD__);
    parent::__construct($path);
    $this->skipdots();
  }

  #[\ReturnTypeWillChange]
  function rewind() {
    parent::rewind();
    $this->skipdots();
  }

  #[\ReturnTypeWillChange]
  function next() {
    parent::next();
    $this->skipdots();
  }

  protected function skipdots() {
    while ($this->isDot()) {
      parent::next();
    }
  }
}
