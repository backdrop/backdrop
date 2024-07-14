<?php
/**
 * @file
 * Shared classes and interfaces for the archiver system.
 */

/**
 * Defines the common interface for all Archiver classes.
 */
interface ArchiverInterface {

  /**
   * Constructs a new archiver instance.
   *
   * @param $file_path
   *   The full system path of the archive to manipulate. Only local files
   *   are supported. If the file does not yet exist, it will be created if
   *   appropriate.
   */
  public function __construct($file_path);

  /**
   * Adds the specified file or directory to the archive.
   *
   * @param $file_path
   *   The full system path of the file or directory to add. Only local files
   *   and directories are supported.
   *
   * @return ArchiverInterface
   *   The called object.
   */
  public function add($file_path);

  /**
   * Removes the specified file from the archive.
   *
   * @param $path
   *   The file name relative to the root of the archive to remove.
   *
   * @return ArchiverInterface
   *   The called object.
   */
  public function remove($path);

  /**
   * Extracts multiple files in the archive to the specified path.
   *
   * @param $path
   *   A full system path of the directory to which to extract files.
   * @param $files
   *   Optionally specify a list of files to be extracted. Files are
   *   relative to the root of the archive. If not specified, all files
   *   in the archive will be extracted.
   *
   * @return ArchiverInterface
   *   The called object.
   */
  public function extract($path, array $files = array());

  /**
   * Lists all files in the archive.
   *
   * @return
   *   An array of file names relative to the root of the archive.
   */
  public function listContents();

  /**
   * Rename all paths/files in the archive.
   *
   * Iterates over all files/folders within the archive, and runs preg_replace()
   * against them, passing on the $pattern and $replacement parameters as
   * provided.
   *
   * Note: In order to save the changes of the renaming of the files/paths, the
   * archive needs to be closed. Implementations of this method need to do that,
   * and it is done for the core-provided ArchiverTar and ArchiverZip classes.
   * So if you need to perform any additional action with the archive (add(),
   * remove(), extract(), listContents()), then you need to instantiate the
   * archiver again.
   *
   * @param string|array $pattern
   *   The pattern to search for. It can be either a string or an array with
   *   strings. The same PCRE modifiers for PHP's preg_replace() can also be
   *   used.
   * @param string|array $replacement
   *   The string or an array with strings to replace. Same rules as the
   *   respective parameter in PHP's preg_replace() apply.
   *
   * @return ArchiverInterface
   *   The called object.
   */
  public function renameContents($pattern, $replacement);
}
