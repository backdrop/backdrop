<?php
/**
 * @file
 * File hooks implemented by the File module.
 */

/**
 * Implements hook_file_presave().
 */
function file_file_presave($file) {
  // Always ensure the filemime property is current.
  if (!empty($file->original) || empty($file->filemime)) {
    $file->filemime = file_get_mimetype($file->uri);
  }

  field_attach_presave('file', $file);

  // Fetch image dimensions.
  file_metadata_fetch_image_dimensions($file);
}

/**
 * Implements hook_file_type().
 */
function file_file_type($file) {
  $types = array();
  foreach (file_type_get_types() as $type) {
    if (file_match_mimetypes($type->mimetypes, $file->filemime)) {
      $types[] = $type->type;
    }
  }

  return $types;
}

/**
 * Implements hook_file_insert().
 */
function file_file_insert($file) {
  // Save file metadata.
  if (!empty($file->metadata)) {
    $query = db_insert('file_metadata')->fields(array('fid', 'name', 'value'));
    foreach ($file->metadata as $name => $value) {
      $query->values(array(
        'fid' => $file->fid,
        'name' => $name,
        'value' => serialize($value),
      ));
    }
    $query->execute();
  }

  // Clear any related field caches.
  file_invalidate_field_caches($file);
}

/**
 * Implements hook_file_update().
 */
function file_file_update($file) {
  // Ensure field data is saved since file_save() does not in Drupal 7.
  field_attach_update('file', $file);

  // Save file metadata.
  db_delete('file_metadata')->condition('fid', $file->fid)->execute();
  if (!empty($file->metadata)) {
    $query = db_insert('file_metadata')->fields(array('fid', 'name', 'value'));
    foreach ($file->metadata as $name => $value) {
      $query->values(array(
        'fid' => $file->fid,
        'name' => $name,
        'value' => serialize($value),
      ));
    }
    $query->execute();
  }

  if (module_exists('image') && file_get_mimetype_type($file) == 'image' && $file->filesize && isset($file->original)) {
    if (!isset($file->metadata)) {
      $file->metadata = array();
    }

    if (!isset($file->original->metadata)) {
      if (!is_object($file->original)) {
        $file->original = new stdClass();
      }
      $file->original->metadata = array();
    }

    // If the file has changed dimensions or a new file has been uploaded,
    // update any image field reference to this file and flush image style
    // derivatives.
    $file->metadata += array('width' => NULL, 'height' => NULL);
    $file->original->metadata += array('width' => NULL, 'height' => NULL);
    if ($file->filesize != $file->original->filesize || $file->metadata['width'] != $file->original->metadata['width'] || $file->metadata['height'] != $file->original->metadata['height']) {
      _file_update_image_field_dimensions($file);
    }

    // Flush image style derivatives whenever an image is updated.
    image_path_flush($file->uri);
  }

  // Clear any related field caches.
  file_invalidate_field_caches($file);
}

/**
 * Implements hook_file_delete().
 */
function file_file_delete($file) {
  field_attach_delete('file', $file);

  // This is safe to call since the file's records from the usage table have
  // not yet been deleted.
  file_invalidate_field_caches($file);

  // Remove file metadata.
  db_delete('file_metadata')->condition('fid', $file->fid)->execute();

  // Remove this file from the search index if needed.
  // This code is implemented in file entity module rather than in search
  // module because file entity is implementing search module's API, not the
  // other way around.
  if (module_exists('search')) {
    search_reindex($file->fid, 'file');
  }
}

/**
 * Implements hook_file_mimetype_mapping_alter().
 */
function file_file_mimetype_mapping_alter(&$mapping) {
  // Add support for mka and mkv.
  // @todo Remove when http://drupal.org/node/1443070 is fixed in core.
  $new_mappings['mka'] = 'audio/x-matroska';
  $new_mappings['mkv'] = 'video/x-matroska';

  // Add support for weba, webm, and webp.
  // @todo Remove when http://drupal.org/node/1443070 is fixed in core.
  $new_mappings['weba'] = 'audio/webm';
  $new_mappings['webm'] = 'video/webm';
  $new_mappings['webp'] = 'image/webp';

  foreach ($new_mappings as $extension => $mime_type) {
    if (!in_array($mime_type, $mapping['mimetypes'])) {
      // If the mime type does not already exist, add it.
      $mapping['mimetypes'][] = $mime_type;
    }

    // Get the index of the mime type and assign the extension to that key.
    $index = array_search($mime_type, $mapping['mimetypes']);
    $mapping['extensions'][$extension] = $index;
  }
}

/**
 * Implements hook_file_load().
 */
function file_file_load($files) {
  // Add alt and title text to images.
  $config = config('file.settings');

  $replace_options = array(
    'clear' => TRUE,
    'sanitize' => FALSE,
  );

  // Load and unserialize metadata.
  $results = db_query("SELECT * FROM {file_metadata} WHERE fid IN (:fids)", array(':fids' => array_keys($files)));

  foreach ($results as $result) {
    $name = $result->name;

    // image.module required height and width to be properties of the file.
    if ($name == 'height' || $name == 'width') {
      $files[$result->fid]->$name = unserialize($result->value);
    }

    $files[$result->fid]->metadata[$name] = unserialize($result->value);
  }
}

/**
 * Fetch the dimensions of an image and store them in the file metadata array.
 */
function file_metadata_fetch_image_dimensions($file) {
  // Prevent PHP notices when trying to read empty files.
  // @see http://drupal.org/node/681042
  if (!$file->filesize) {
    return;
  }

  // Do not bother proceeding if this file does not have an image mime type.
  if (file_get_mimetype_type($file) != 'image') {
    return;
  }

  // We have a non-empty image file.
  $image_info = image_get_info($file->uri);
  if ($image_info) {
    $file->metadata['width'] = $image_info['width'];
    $file->metadata['height'] = $image_info['height'];
  }
  else {
    // Fallback to NULL values.
    $file->metadata['width'] = NULL;
    $file->metadata['height'] = NULL;
  }
}

/**
 * Update the image dimensions stored in any image fields for a file.
 *
 * @param object $file
 *   A file object that is an image.
 *
 * @see http://drupal.org/node/1448124
 */
function _file_update_image_field_dimensions($file) {
  // Prevent PHP notices when trying to read empty files.
  // @see http://drupal.org/node/681042
  if (!$file->filesize) {
    return;
  }

  // Do not bother proceeding if this file does not have an image mime type.
  if (file_get_mimetype_type($file) != 'image') {
    return;
  }

  // Find all image field enabled on the site.
  $image_fields = _file_get_fields_by_type('image');

  foreach ($image_fields as $image_field) {
    $query = new EntityFieldQuery();
    $query->fieldCondition($image_field, 'fid', $file->fid);
    $results = $query->execute();

    foreach ($results as $entity_type => $entities) {
      $entities = entity_load($entity_type, array_keys($entities));
      foreach ($entities as $entity) {
        foreach ($entity->{$image_field} as $langcode => $items) {
          foreach ($items as $delta => $item) {
            if ($item['fid'] == $file->fid) {
              $entity->{$image_field}[$langcode][$delta]['width'] = $file->metadata['width'];
              $entity->{$image_field}[$langcode][$delta]['height'] = $file->metadata['height'];
            }
          }
        }

        // Save the updated field column values.
        _file_fields_update($entity_type, $entity);
      }
    }
  }
}

/**
 * Update an entity's field values without changing anything on the entity.
 */
function _file_fields_update($entity_type, $entity) {
  $id = $entity->id();
  if (empty($id)) {
    throw new Exception(t('Cannot call _file_fields_update() on a new entity.'));
  }

  // Some modules use the original property.
  if (!isset($entity->original)) {
    $entity->original = $entity;
  }

  // Ensure that file_field_update() will not trigger additional usage.
  unset($entity->revision);

  // Invoke the field presave and update hooks.
  field_attach_presave($entity_type, $entity);
  field_attach_update($entity_type, $entity);

  // Clear the cache for this entity now.
  entity_get_controller($entity_type)->resetCache(array($id));
}

/**
 * Implements hook_file_metadata_info().
 */
function file_file_metadata_info() {
  $info['width'] = array('label' => t('Width'), 'type' => 'integer');
  $info['height'] = array('label' => t('Height'), 'type' => 'integer');
  return $info;
}
