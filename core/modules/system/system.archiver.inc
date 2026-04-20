<?php
/**
 * @file
 * Archiver implementations provided by the system module.
 */

/**
 * Archiver for .tar files.
 */
class ArchiverTar implements ArchiverInterface {

  /**
   * The underlying Archive_Tar instance that does the heavy lifting.
   *
   * @var Archive_Tar
   */
  protected $tar;

  public function __construct($file_path) {
    $this->tar = new Archive_Tar($file_path);
  }

  public function add($file_path) {
    $this->tar->add($file_path);

    return $this;
  }

  public function remove($file_path) {
    // @todo Archive_Tar doesn't have a remove operation so we'll have to
    // simulate it somehow, probably by creating a new archive with everything
    // but the removed file.
    return $this;
  }

  public function extract($path, Array $files = array()) {
    if ($files) {
      $this->tar->extractList($files, $path, '', FALSE, FALSE);
    }
    else {
      $this->tar->extract($path, FALSE, FALSE);
    }

    return $this;
  }

  public function listContents() {
    $files = array();
    foreach ($this->tar->listContent() as $file_data) {
      $files[] = $file_data['filename'];
    }
    return $files;
  }

  public function renameContents($pattern, $replacement) {
    // @todo. On first look, Archive_Tar doesn't support renaming archive
    // contents. Perhaps simulate it by creating a new archive, similarly to
    // remove() above.
    return $this;
  }

  /**
   * Retrieve the tar engine itself.
   *
   * In some cases it may be necessary to directly access the underlying
   * Archive_Tar object for implementation-specific logic. This is for advanced
   * use only as it is not shared by other implementations of ArchiveInterface.
   *
   * @return
   *   The Archive_Tar object used by this object.
   */
  public function getArchive() {
    return $this->tar;
  }
}

/**
 * Archiver for .zip files.
 *
 * @link http://php.net/zip
 */
class ArchiverZip implements ArchiverInterface {

  /**
   * The underlying ZipArchive instance that does the heavy lifting.
   *
   * @var ZipArchive
   */
  protected $zip;

  /**
   * @var string The path of the Zip archive.
   */
  public $zip_archive = '';

  public function __construct($file_path) {
    $this->zip = new ZipArchive();

    // @todo: various comments in php.net and stackoverflow.com suggest that
    // ZipArchive::open() fails in certain PHP versions, unless called with the
    // 'ZipArchive::CREATE | ZipArchive::OVERWRITE' flags. Need to confirm and
    // adjust this accordingly.
    // Also see: https://www.drupal.org/project/drupal/issues/2850794.
    if ($this->zip->open($file_path) !== TRUE) {
      // @todo: This should be an interface-specific exception some day.
      throw new Exception(t('Cannot open %file_path', array('%file_path' => $file_path)));
    }

    // Store the path of the archive so that it can be used by other methods if
    // needed (like renameContents() for instance).
    $this->zip_archive = $file_path;
  }

  public function add($file_path) {
    $this->zip->addFile($file_path);

    return $this;
  }

  public function remove($file_path) {
    $this->zip->deleteName($file_path);

    return $this;
  }

  public function extract($path, Array $files = array()) {
    if ($files) {
      $this->zip->extractTo($path, $files);
    }
    else {
      $this->zip->extractTo($path);
    }

    return $this;
  }

  public function listContents() {
    $files = array();
    for ($i=0; $i < $this->zip->numFiles; $i++) {
      $files[] = $this->zip->getNameIndex($i);
    }
    return $files;
  }

  public function renameContents($pattern, $replacement) {
    for ($i = 0; $i < $this->zip->numFiles; $i++) {
      $new_name = preg_replace($pattern, $replacement, $this->zip->getNameIndex($i));
      $this->zip->renameName($this->zip->getNameIndex($i), $new_name);
    }
    // Need to close the archive in order to save the changes.
    if ($this->zip->close() !== TRUE) {
      // @todo: This should be an interface-specific exception some day.
      throw new Exception(t('Failure to close the archive. Changes might have not been saved.'));
    }

    // Reopen the archive after save.
    $this->zip = new ZipArchive();
    $this->zip->open($this->zip_archive);

    return $this;
  }

  /**
   * Retrieve the zip engine itself.
   *
   * In some cases it may be necessary to directly access the underlying
   * ZipArchive object for implementation-specific logic. This is for advanced
   * use only as it is not shared by other implementations of ArchiveInterface.
   *
   * @return
   *   The ZipArchive object used by this object.
   */
  public function getArchive() {
    return $this->zip;
  }
}
