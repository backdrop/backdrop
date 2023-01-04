<?php
/**
 * @file
 * Devel generate support for File module.
 */

function file_devel_generate($object, $field, $instance, $bundle) {
  if (field_behaviors_widget('multiple values', $instance) == FIELD_BEHAVIOR_CUSTOM) {
    return devel_generate_multiple('_file_devel_generate', $object, $field, $instance, $bundle);
  }
  else {
    return _file_devel_generate($object, $field, $instance, $bundle);
  }
}

function _file_devel_generate($object, $field, $instance, $bundle) {
  static $file;

  if (empty($file)) {
    if ($path = devel_generate_textfile()) {
      $destination_dir = $field['settings']['uri_scheme'] . '://' . $instance['settings']['file_directory'];
      file_prepare_directory($destination_dir, FILE_CREATE_DIRECTORY);
      if ($uri = file_unmanaged_move($path, $destination_dir)) {
        $file = new File();
        $file->fid = NULL;
        $file->uri = $uri;
        $file->filename = backdrop_basename($uri);
        $file->filemime = file_get_mimetype($file->uri);
        // @todo Randomize file owner.
        $file->uid = 1;
        $file = file_save($file);
      }
      else {
        return FALSE;
      }
    }
    else {
      return FALSE;
    }
  }
  if (!$file) {
    // In case a previous file operation failed or no file is set, return FALSE
    return FALSE;
  }
  else {
    $object_field['fid'] = $file->fid;
    $object_field['display'] = $field['settings']['display_default'];
    $object_field['description'] = devel_create_greeking(10);

    return $object_field;
  }
}

/**
 * Private function for generating a random text file.
 */
function devel_generate_textfile($filesize = 1024) {
  if ($tmp_file = backdrop_tempnam('temporary://', 'filefield_')) {
    $destination = $tmp_file . '.txt';
    file_unmanaged_move($tmp_file, $destination);

    $fp = fopen($destination, 'w');
    fwrite($fp, str_repeat('01', $filesize/2));
    fclose($fp);

    return $destination;
  }
}
