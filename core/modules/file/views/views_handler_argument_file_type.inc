<?php
/**
 * @file
 * Definition of views_handler_argument_file_type.
 */

/**
 * Argument handler to accept a file type.
 */
class views_handler_argument_file_type extends views_handler_argument_string {

  /**
   * Override the behavior of summary_name().
   */
  public function summary_name($data) {
    return $this->fileType($data->{$this->name_alias});
  }

  /**
   * Override the behavior of title().
   */
  public function title() {
    return $this->fileType($this->argument);
  }

  /**
   * Get the File label (human readable).
   *
   * @param string $type
   *   The views string argument, supposed to be a File type (machine name).
   *
   * @return string
   *   The File type label, if the type exists.
   */
  protected function fileType($type) {
    $file_type_name = file_type_get_name($type);
    if (empty($file_type_name)) {
      return t('Unknown file type');
    }
    return check_plain($file_type_name);
  }
}
