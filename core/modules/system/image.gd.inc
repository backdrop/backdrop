<?php

/**
 * @file
 * GD2 toolkit for image manipulation within Backdrop.
 */

/**
 * @addtogroup image
 * @{
 */

/**
 * Retrieve settings for the GD2 toolkit.
 */
function image_gd_settings() {
  if (image_gd_check_settings()) {
    $form['status'] = array(
      '#markup' => t('The GD toolkit is installed and working properly.')
    );

    $form['image_jpeg_quality'] = array(
      '#type' => 'number',
      '#title' => t('JPEG quality'),
      '#description' => t('Define the image quality for JPEG manipulations. Ranges from 0 to 100. Higher values mean better image quality but bigger files.'),
      '#min' => 0,
      '#max' => 100,
      '#default_value' => config_get('system.core', 'image_jpeg_quality', 75),
      '#field_suffix' => t('%'),
    );

    return $form;
  }
  else {
    form_set_error('image_toolkit', t('The GD image toolkit requires that the GD module for PHP be installed and configured properly. For more information see <a href="@url">PHP\'s image documentation</a>.', array('@url' => 'http://php.net/image')));
    return FALSE;
  }
}

/**
 * Verify GD2 settings (that the right version is actually installed).
 *
 * @return
 *   A boolean indicating if the GD toolkit is available on this machine.
 */
function image_gd_check_settings() {
  // GD2 support is available.
  return function_exists('imagegd2');
}

/**
 * Scale an image to the specified size using GD.
 *
 * @param $image
 *   An image object. The $image->resource, $image->info['width'], and
 *   $image->info['height'] values will be modified by this call.
 * @param $width
 *   The new width of the resized image, in pixels.
 * @param $height
 *   The new height of the resized image, in pixels.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_resize()
 */
function image_gd_resize(stdClass $image, $width, $height) {
  $res = image_gd_create_tmp($image, $width, $height);

  if (!imagecopyresampled($res, $image->resource, 0, 0, 0, 0, $width, $height, $image->info['width'], $image->info['height'])) {
    return FALSE;
  }

  imagedestroy($image->resource);
  // Update image object.
  $image->resource = $res;
  $image->info['width'] = $width;
  $image->info['height'] = $height;
  return TRUE;
}

/**
 * Rotate an image the given number of degrees.
 *
 * @param $image
 *   An image object. The $image->resource, $image->info['width'], and
 *   $image->info['height'] values will be modified by this call.
 * @param $degrees
 *   The number of (clockwise) degrees to rotate the image.
 * @param $background
 *   An decimal integer specifying the background color to use for the
 *   uncovered area of the image after the rotation. E.g. 0 for black,
 *   16711935 for fuchsia, and 16777215 for white. For images that support
 *   transparency, this will default to transparent. Otherwise it will
 *   be white.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_rotate()
 */
function image_gd_rotate(stdClass $image, $degrees, $background = NULL) {
  // PHP installations using non-bundled GD do not have imagerotate.
  if (!function_exists('imagerotate')) {
    watchdog('image', 'The image %file could not be rotated because the imagerotate() function is not available in this PHP installation.', array('%file' => $image->source));
    return FALSE;
  }

  // PHP 5.5 GD bug: https://bugs.php.net/bug.php?id=65148: To prevent buggy
  // behavior on negative multiples of 90 degrees we convert any negative
  // angle to a positive one between 0 and 360 degrees.
  $degrees -= floor($degrees / 360) * 360;

  if (isset($background)) {
    $background = _image_dec_to_rgba($background);
  }
  // Set the background color as transparent white if $background is NULL.
  else {
    // Get the current transparent color.
    $background = imagecolortransparent($image->resource);

    // If no transparent colors, use transparent white.
    if ($background === -1) {
      $background = array('red' => 255, 'green' => 255, 'blue' => 255, 'alpha' => 127);
    }
    // Otherwise apply the transparent color
    // TODO: correctly apply this instead of just copying the use transparent white from above
    else{
      $background = array('red' => 255, 'green' => 255, 'blue' => 255, 'alpha' => 127);
    }
  }

  // Store the color index for the background as that is what GD uses.
  $background_idx = imagecolorallocatealpha($image->resource, $background['red'], $background['green'], $background['blue'], $background['alpha']);

  // Images are assigned a new color palette when rotating, removing any
  // transparency flags. For GIF images, keep a record of the transparent color.
  if ($image->info['extension'] == 'gif') {
    // GIF does not work with a transparency channel, but can define 1 color
    // in its palette to act as transparent.

    // Get the current transparent color, if any.
    $gif_transparent_id = imagecolortransparent($image->resource);
    if ($gif_transparent_id !== -1) {
      // The gif already has a transparent color set: remember it to set it on
      // the rotated image as well.
      $arguments['gif_transparent_color'] = imagecolorsforindex($image->resource, $gif_transparent_id);

      if ($background['alpha'] >= 127) {
        // We want a transparent background: use the color already set to act
        // as transparent, as background.
        $background_idx = $gif_transparent_id;
      }
    }
    else {
      // The gif does not currently have a transparent color set.
      if ($background['alpha'] >= 127) {
        // But as the background is transparent, it should get one.
        $transparent_gif_color = $background;
      }
    }
  }

  $image->resource = imagerotate($image->resource, 360 - $degrees, $background_idx);

  // GIFs need to reassign the transparent color after performing the rotate.
  if (isset($transparent_gif_color)) {
    $background = imagecolorexactalpha($image->resource, $transparent_gif_color['red'], $transparent_gif_color['green'], $transparent_gif_color['blue'], $transparent_gif_color['alpha']);
    imagecolortransparent($image->resource, $background);
  }

  $image->info['width'] = imagesx($image->resource);
  $image->info['height'] = imagesy($image->resource);
  return TRUE;
}

/**
 * Crop an image using the GD toolkit.
 *
 * @param $image
 *   An image object. The $image->resource, $image->info['width'], and
 *   $image->info['height'] values will be modified by this call.
 * @param $x
 *   The starting x offset at which to start the crop, in pixels.
 * @param $y
 *   The starting y offset at which to start the crop, in pixels.
 * @param $width
 *   The width of the cropped area, in pixels.
 * @param $height
 *   The height of the cropped area, in pixels.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_crop()
 */
function image_gd_crop(stdClass $image, $x, $y, $width, $height) {
  $res = image_gd_create_tmp($image, $width, $height);

  if (!imagecopyresampled($res, $image->resource, 0, 0, $x, $y, $width, $height, $width, $height)) {
    return FALSE;
  }

  // Destroy the original image and return the modified image.
  imagedestroy($image->resource);
  $image->resource = $res;
  $image->info['width'] = $width;
  $image->info['height'] = $height;
  return TRUE;
}

/**
 * Convert an image resource to grayscale.
 *
 * Note that transparent GIFs loose transparency when desaturated.
 *
 * @param $image
 *   An image object. The $image->resource value will be modified by this call.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_desaturate()
 */
function image_gd_desaturate(stdClass $image) {
  // PHP installations using non-bundled GD do not have imagefilter.
  if (!function_exists('imagefilter')) {
    watchdog('image', 'The image %file could not be desaturated because the imagefilter() function is not available in this PHP installation.', array('%file' => $image->source));
    return FALSE;
  }

  return imagefilter($image->resource, IMG_FILTER_GRAYSCALE);
}

/**
 * GD helper function to create an image resource from a file.
 *
 * @param $image
 *   An image object. The $image->resource value will populated by this call.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_load()
 */
function image_gd_load(stdClass $image) {
  $extension = str_replace('jpg', 'jpeg', $image->info['extension']);
  $function = 'imagecreatefrom' . $extension;
  return (function_exists($function) && $image->resource = $function($image->source));
}

/**
 * GD helper to write an image resource to a destination file.
 *
 * @param $image
 *   An image object.
 * @param $destination
 *   A string file URI or path where the image should be saved.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_save()
 */
function image_gd_save(stdClass $image, $destination) {
  $scheme = file_uri_scheme($destination);
  // Work around lack of stream wrapper support in imagejpeg() and imagepng().
  if ($scheme && file_stream_wrapper_valid_scheme($scheme)) {
    // If destination is not local, save image to temporary local file.
    $local_wrappers = file_get_stream_wrappers(STREAM_WRAPPERS_LOCAL);
    if (!isset($local_wrappers[$scheme])) {
      $permanent_destination = $destination;
      $destination = backdrop_tempnam('temporary://', 'gd_');
    }
    // Convert stream wrapper URI to normal path.
    $destination = backdrop_realpath($destination);
  }

  $extension = str_replace('jpg', 'jpeg', $image->info['extension']);
  $function = 'image' . $extension;
  if (!function_exists($function)) {
    return FALSE;
  }
  if ($extension == 'jpeg') {
    $success = $function($image->resource, $destination, config_get('system.core', 'image_jpeg_quality', 75));
  }
  else {
    // Always save PNG images with full transparency.
    if ($extension == 'png') {
      imagealphablending($image->resource, FALSE);
      imagesavealpha($image->resource, TRUE);
    }
    $success = $function($image->resource, $destination);
  }
  // Move temporary local file to remote destination.
  if (isset($permanent_destination) && $success) {
    return (bool) file_unmanaged_move($destination, $permanent_destination, FILE_EXISTS_REPLACE);
  }
  return $success;
}

/**
 * Create a truecolor image preserving transparency from a provided image.
 *
 * @param $image
 *   An image object.
 * @param $width
 *   The new width of the new image, in pixels.
 * @param $height
 *   The new height of the new image, in pixels.
 * @return
 *   A GD image handle.
 */
function image_gd_create_tmp(stdClass $image, $width, $height) {
  $res = imagecreatetruecolor($width, $height);

  if ($image->info['extension'] == 'gif') {
    // Grab transparent color index from image resource.
    $transparent = imagecolortransparent($image->resource);

    if ($transparent >= 0) {
      // The original must have a transparent color, allocate to the new image.
      $transparent_color = imagecolorsforindex($image->resource, $transparent);
      $transparent = imagecolorallocate($res, $transparent_color['red'], $transparent_color['green'], $transparent_color['blue']);

      // Flood with our new transparent color.
      imagefill($res, 0, 0, $transparent);
      imagecolortransparent($res, $transparent);
    }
  }
  elseif ($image->info['extension'] == 'png') {
    imagealphablending($res, FALSE);
    $transparency = imagecolorallocatealpha($res, 0, 0, 0, 127);
    imagefill($res, 0, 0, $transparency);
    imagealphablending($res, TRUE);
    imagesavealpha($res, TRUE);
  }
  else {
    imagefill($res, 0, 0, imagecolorallocate($res, 255, 255, 255));
  }

  return $res;
}

/**
 * Get details about an image.
 *
 * @param $image
 *   An image object.
 * @return
 *   FALSE, if the file could not be found or is not an image. Otherwise, a
 *   keyed array containing information about the image:
 *   - "width": Width, in pixels.
 *   - "height": Height, in pixels.
 *   - "extension": Commonly used file extension for the image.
 *   - "mime_type": MIME type ('image/jpeg', 'image/gif', 'image/png').
 *
 * @see image_get_info()
 */
function image_gd_get_info(stdClass $image) {
  $details = FALSE;
  $data = @getimagesize($image->source);

  if (isset($data) && is_array($data)) {
    $extensions = array('1' => 'gif', '2' => 'jpg', '3' => 'png');
    $extension = isset($extensions[$data[2]]) ?  $extensions[$data[2]] : '';
    $details = array(
      'width'     => $data[0],
      'height'    => $data[1],
      'extension' => $extension,
      'mime_type' => $data['mime'],
    );
  }

  return $details;
}

/**
 * @} End of "addtogroup image".
 */
