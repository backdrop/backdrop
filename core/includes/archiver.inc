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
}
