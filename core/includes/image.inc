<?php
/**
 * @file
 * API for manipulating images.
 */

/**
 * @defgroup image Image toolkits
 * @{
 * Functions for image file manipulations.
 *
 * Backdrop's image toolkits provide an abstraction layer for common image file
 * manipulations like scaling, cropping, and rotating. The abstraction frees
 * module authors from the need to support multiple image libraries, and it
 * allows site administrators to choose the library that's best for them.
 *
 * PHP includes the GD library by default so a GD toolkit is installed with
 * Backdrop. Other toolkits like ImageMagick are available from contrib modules.
 * GD works well for small images, but using it with larger files may cause PHP
 * to run out of memory. In contrast the ImageMagick library does not suffer
 * from this problem, but it requires the ISP to have installed additional
 * software.
 *
 * Image toolkits are discovered based on the associated module's
 * hook_image_toolkits. The toolkit must then be enabled using the
 * admin/config/media/image-toolkit form.
 *
 * Only one toolkit may be selected at a time. If a module author wishes to call
 * a specific toolkit they can check that it is installed by calling
 * image_get_available_toolkits(), and then calling its functions directly.
 */

/**
 * Default width for SVG images.
 */
define('IMAGE_SVG_DEFAULT_WIDTH', 200);

/**
 * Default height for SVG images.
 */
define('IMAGE_SVG_DEFAULT_HEIGHT', 200);

/**
 * Gets a list of available toolkits.
 *
 * @return
 *   An array with the toolkit names as keys and the descriptions as values.
 */
function image_get_available_toolkits() {
  // hook_image_toolkits returns an array of toolkit names.
  $toolkits = module_invoke_all('image_toolkits');

  $output = array();
  foreach ($toolkits as $name => $info) {
    // Only allow modules that aren't marked as unavailable.
    if ($info['available']) {
      $output[$name] = $info['title'];
    }
  }

  return $output;
}

/**
 * Gets the name of the currently used toolkit.
 *
 * @return
 *   String containing the name of the selected toolkit, or FALSE on error.
 */
function image_get_toolkit() {
  static $toolkit;

  if (!isset($toolkit)) {
    $toolkits = image_get_available_toolkits();
    $toolkit = config_get('system.core', 'image_toolkit');
    if (!isset($toolkits[$toolkit]) || !function_exists('image_' . $toolkit . '_load')) {
      // The selected toolkit isn't available so return the first one found. If
      // none are available this will return FALSE.
      reset($toolkits);
      $toolkit = key($toolkits);
    }
  }

  return $toolkit;
}

/**
 * Invokes the given method using the currently selected toolkit.
 *
 * @param $method
 *   A string containing the method to invoke.
 * @param $image
 *   An image object returned by image_load().
 * @param $params
 *   An optional array of parameters to pass to the toolkit method.
 *
 * @return
 *   Mixed values (typically Boolean indicating successful operation).
 */
function image_toolkit_invoke($method, stdClass $image, array $params = array()) {
  $function = 'image_' . $image->toolkit . '_' . $method;
  if (function_exists($function)) {
    array_unshift($params, $image);
    return call_user_func_array($function, $params);
  }
  watchdog('image', 'The selected image handling toolkit %toolkit can not correctly process %function.', array('%toolkit' => $image->toolkit, '%function' => $function), WATCHDOG_ERROR);
  return FALSE;
}

/**
 * Gets supported image extensions.
 *
 * Backdrop supports GIF, JPG, PNG, and WEBP file formats when supported by the
 * GD toolkit, and may support others, depending on the currently used toolkit.
 *
 * @return array
 *   Array of supported extensions.
 *
 * @since 1.25.0 Function added.
 */
function image_get_supported_extensions() {
  $toolkit = image_get_toolkit();
  $function = 'image_' . $toolkit . '_supported_extensions';
  if (function_exists($function)) {
    $extensions = $function();
    if (file_get_svg_support()) {
      $extensions[] = 'svg';
    }
    return $extensions;
  }
  watchdog('image', 'The selected image handling toolkit %toolkit should provide a %function callback.',
    array(
      '%toolkit' => $toolkit,
      '%function' => $function),
    WATCHDOG_ERROR
  );
  // Provide a fallback list of extensions.
  return array('jpg', 'jpeg', 'gif', 'png');
}

/**
 * Gets details about an image.
 *
 * Backdrop supports GIF, JPG and PNG file formats when used with the GD
 * toolkit, and may support others, depending on which toolkits are
 * installed.
 *
 * @param $filepath
 *   String specifying the path of the image file.
 * @param $toolkit
 *   An optional image toolkit name to override the default.
 *
 * @return
 *   FALSE, if the file could not be found or is not an image. Otherwise, a
 *   keyed array containing information about the image:
 *   - "width": Width, in pixels.
 *   - "height": Height, in pixels.
 *   - "extension": Commonly used file extension for the image.
 *   - "mime_type": MIME type ('image/jpeg', 'image/gif', 'image/png').
 *   - "file_size": File size in bytes.
 */
function image_get_info($filepath, $toolkit = FALSE) {
  $details = FALSE;
  if (!is_file($filepath) && !is_uploaded_file($filepath)) {
    return $details;
  }

  if (!$toolkit) {
    $toolkit = image_get_toolkit();
  }
  if ($toolkit) {
    $image = new stdClass();
    $image->source = $filepath;
    $image->toolkit = $toolkit;
    $details = image_toolkit_invoke('get_info', $image);
    if (isset($details) && is_array($details)) {
      $details['file_size'] = filesize($filepath);
    }
  }

  return $details;
}

/**
 * Scales an image to the exact width and height given.
 *
 * This function achieves the target aspect ratio by cropping the original image
 * according to the selected anchor type.
 *
 * The resulting image always has the exact target dimensions.
 *
 * @param $image
 *   An image object returned by image_load().
 * @param $width
 *   The target width, in pixels.
 * @param $height
 *   The target height, in pixels.
 * @param $anchor
 *   The anchor to use when cropping.
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @since 1.21.0 $anchor parameter added.
 *
 * @see image_load()
 * @see image_resize()
 * @see image_crop()
 * @see image_scale_and_crop_effect()
 */
function image_scale_and_crop(stdClass $image, $width, $height, $anchor = 'center-center') {
  $scale = max($width / $image->info['width'], $height / $image->info['height']);

  // Set the top left coordinates of the crop area, based on the anchor.
  list($x, $y) = explode('-', $anchor);
  $x = image_filter_keyword($x, $image->info['width'] * $scale, $width);
  $y = image_filter_keyword($y, $image->info['height'] * $scale, $height);

  if (image_resize($image, $image->info['width'] * $scale, $image->info['height'] * $scale)) {
    return image_crop($image, $x, $y, $width, $height);
  }
  return FALSE;
}

/**
 * Scales image dimensions while maintaining aspect ratio.
 *
 * The resulting dimensions can be smaller for one or both target dimensions.
 *
 * @param $dimensions
 *   Dimensions to be modified - an array with components width and height, in
 *   pixels.
 * @param $width
 *   The target width, in pixels. If this value is NULL then the scaling will be
 *   based only on the height value.
 * @param $height
 *   The target height, in pixels. If this value is NULL then the scaling will
 *   be based only on the width value.
 * @param $upscale
 *   Boolean indicating that images smaller than the target dimensions will be
 *   scaled up. This generally results in a low quality image.
 *
 * @return
 *   TRUE if $dimensions was modified, FALSE otherwise.
 *
 * @see image_scale()
 */
function image_dimensions_scale(array &$dimensions, $width = NULL, $height = NULL, $upscale = FALSE) {
  $aspect = $dimensions['height'] / $dimensions['width'];

  // Calculate one of the dimensions from the other target dimension,
  // ensuring the same aspect ratio as the source dimensions. If one of the
  // target dimensions is missing, that is the one that is calculated. If both
  // are specified then the dimension calculated is the one that would not be
  // calculated to be bigger than its target.
  if (($width && !$height) || ($width && $height && $aspect < $height / $width)) {
    $height = (int) round($width * $aspect);
  }
  else {
    $width = (int) round($height / $aspect);
  }

  // Don't upscale if the option isn't enabled.
  if (!$upscale && ($width >= $dimensions['width'] || $height >= $dimensions['height'])) {
    return FALSE;
  }

  $dimensions['width'] = $width;
  $dimensions['height'] = $height;
  return TRUE;
}

/**
 * Scales an image while maintaining aspect ratio.
 *
 * The resulting image can be smaller for one or both target dimensions.
 *
 * @param $image
 *   An image object returned by image_load().
 * @param $width
 *   The target width, in pixels. If this value is NULL then the scaling will
 *   be based only on the height value.
 * @param $height
 *   The target height, in pixels. If this value is NULL then the scaling will
 *   be based only on the width value.
 * @param $upscale
 *   Boolean indicating that files smaller than the dimensions will be scaled
 *   up. This generally results in a low quality image.
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @see image_dimensions_scale()
 * @see image_load()
 * @see image_scale_and_crop()
 */
function image_scale(stdClass $image, $width = NULL, $height = NULL, $upscale = FALSE) {
  $dimensions = $image->info;

  // Scale the dimensions - if they don't change then just return success.
  if (!image_dimensions_scale($dimensions, $width, $height, $upscale)) {
    return TRUE;
  }

  return image_resize($image, $dimensions['width'], $dimensions['height']);
}

/**
 * Resizes an image to the given dimensions (ignoring aspect ratio).
 *
 * @param $image
 *   An image object returned by image_load().
 * @param $width
 *   The target width, in pixels.
 * @param $height
 *   The target height, in pixels.
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @see image_load()
 * @see image_gd_resize()
 */
function image_resize(stdClass $image, $width, $height) {
  $width = (int) round($width);
  $height = (int) round($height);

  return image_toolkit_invoke('resize', $image, array($width, $height));
}

/**
 * Rotates an image by the given number of degrees.
 *
 * @param $image
 *   An image object returned by image_load().
 * @param $degrees
 *   The number of (clockwise) degrees to rotate the image.
 * @param $background
 *   An hexadecimal integer specifying the background color to use for the
 *   uncovered area of the image after the rotation. E.g. 0x000000 for black,
 *   0xff00ff for magenta, and 0xffffff for white. For images that support
 *   transparency, this will default to transparent. Otherwise it will
 *   be white.
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @see image_load()
 * @see image_gd_rotate()
 */
function image_rotate(stdClass $image, $degrees, $background = NULL) {
  // Standardize $degrees to be an integer if possible. This avoids strange
  // rounding errors when rotating multiples of 90 degrees. For example a
  // 100x100 image when rotated 90.0 (float) degrees may result in an image
  // dimension of 99x100 pixels. See https://bugs.php.net/bug.php?id=65148.
  if (strcmp((int) $degrees, $degrees) === 0) {
    $degrees = (int) $degrees;
  }
  else {
    $degrees = (float) $degrees;
  }
  // Likewise, negative rotations can sometimes have rounding errors, convert
  // to a positive number before passing to the toolkit. This bug is also
  // reported at https://bugs.php.net/bug.php?id=65148 (same as above).
  while ($degrees < 0) {
    $degrees = 360 + $degrees;
  }
  // Reduce greater than 360 degrees just for good measure.
  while ($degrees >= 360) {
    $degrees = 360 - $degrees;
  }

  return image_toolkit_invoke('rotate', $image, array($degrees, $background));
}

/**
 * Crops an image to a rectangle specified by the given dimensions.
 *
 * @param $image
 *   An image object returned by image_load().
 * @param $x
 *   The top left coordinate, in pixels, of the crop area (x axis value).
 * @param $y
 *   The top left coordinate, in pixels, of the crop area (y axis value).
 * @param $width
 *   The target width, in pixels.
 * @param $height
 *   The target height, in pixels.
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @see image_load()
 * @see image_scale_and_crop()
 * @see image_gd_crop()
 */
function image_crop(stdClass $image, $x, $y, $width, $height) {
  $aspect = $image->info['height'] / $image->info['width'];
  if (empty($height)) $height = $width / $aspect;
  if (empty($width)) $width = $height * $aspect;

  $width = (int) round($width);
  $height = (int) round($height);

  return image_toolkit_invoke('crop', $image, array($x, $y, $width, $height));
}

/**
 * Converts an image to grayscale.
 *
 * @param $image
 *   An image object returned by image_load().
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @see image_load()
 * @see image_gd_desaturate()
 */
function image_desaturate(stdClass $image) {
  return image_toolkit_invoke('desaturate', $image);
}

/**
 * Loads an image file and returns an image object.
 *
 * Any changes to the file are not saved until image_save() is called.
 *
 * @param $file
 *   Path to an image file.
 * @param $toolkit
 *   An optional, image toolkit name to override the default.
 *
 * @return
 *   An image object or FALSE if there was a problem loading the file. The
 *   image object has the following properties:
 *    - 'source' - The original file path.
 *    - 'info' - The array of information returned by image_get_info()
 *    - 'toolkit' - The name of the image toolkit requested when the image was
 *      loaded.
 *   Image toolkits may add additional properties. The caller is advised not to
 *   monkey about with them.
 *
 * @see image_save()
 * @see image_get_info()
 * @see image_get_available_toolkits()
 * @see image_gd_load()
 */
function image_load($file, $toolkit = FALSE) {
  if (!$toolkit) {
    $toolkit = image_get_toolkit();
  }
  if ($toolkit) {
    $image = new stdClass();
    $image->source = $file;
    $image->info = image_get_info($file, $toolkit);
    if (isset($image->info) && is_array($image->info)) {
      $image->toolkit = $toolkit;
      if (image_toolkit_invoke('load', $image)) {
        return $image;
      }
    }
  }
  return FALSE;
}

/**
 * Closes the image and saves the changes to a file.
 *
 * @param $image
 *   An image object returned by image_load(). The object's 'info' property
 *   will be updated if the file is saved successfully.
 * @param $destination
 *   Destination path where the image should be saved. If it is empty the
 *   original image file will be overwritten.
 *
 * @return
 *   TRUE on success, FALSE on failure.
 *
 * @see image_load()
 * @see image_gd_save()
 */
function image_save(stdClass $image, $destination = NULL) {
  if (empty($destination)) {
    $destination = $image->source;
  }
  if ($return = image_toolkit_invoke('save', $image, array($destination))) {
    // Clear the cached file size and refresh the image information.
    clearstatcache();
    $image->info = image_get_info($destination, $image->toolkit);

    if (backdrop_chmod($destination)) {
      return $return;
    }
  }
  return FALSE;
}

/**
 * Converts a hex string to RGBA (red, green, blue, alpha) integer values and
 * tidies up the input hex color string.
 *
 * @param $hex
 *   A string specifying an RGBA color in the formats: '#RGB', 'RGB', '#RGBA',
 *   'RGBA', '#RRGGBB', 'RRGGBB', '#RRGGBBAA', 'RRGGBBAA' or '0xRRGGBBAA'. The
 *   color is passed by reference to tidy up the string for consistency.
 *
 * @return
 *   An associative array with 'red', 'green', 'blue' and 'alpha' as keys and corresponding
 *   color decimal integer as values or FALSE on an invalid color hex string.
 */
function image_hex2rgba(&$hex) {
  // Remove the leading '0x', if any (legacy).
  if (strpos($hex, '0x') === 0) {
    $hex = substr($hex, 2);
  }

  // Save a uppercase version without leading "#" for later processing.
  $hex_color = backdrop_strtoupper(ltrim($hex, '#'));

  // Normalize shorthand versions.
  // '#FA3' will become '#FFAA33', '#FA37' will become '#FFAA3377'.
  if (strlen($hex_color) <= 4) {
    $hex_color = preg_replace('|([0-9A-F])|', '\1\1', $hex_color);
  }

  // Ensure consistency for the color string.
  $hex = '#' . $hex_color;

  // Return FALSE if is not a valid hex color string.
  if (!preg_match('/^([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})([0-7][0-9A-F])?$/', $hex_color, $colors)) {
    return FALSE;
  }

  return array(
    'red' => hexdec($colors[1]),
    'green' => hexdec($colors[2]),
    'blue' => hexdec($colors[3]),
    'alpha' => isset($colors[4]) ? hexdec($colors[4]) : 0,
  );
}

/**
 * Get an SVG image's defined dimensions.
 *
 * @param string $uri
 *   The URI or path to an SVG image.
 *
 * @return array|FALSE
 *   An array containing the keys "width" and "height" as integer values. If the
 *   image is an SVG but no set dimensions are available, these keys will have
 *   NULL values. If the image is not an SVG, FALSE will be return.
 */
function image_get_svg_dimensions($uri) {
  // Safety check.
  if (!image_is_svg($uri)) {
    return FALSE;
  }

  // Return cached dimensions if already retrieved once this request.
  $cached_images = &backdrop_static(__FUNCTION__, array());
  if (isset($cached_images[$uri])) {
    return $cached_images[$uri];
  }

  $svg_dimensions = array(
    'width' => IMAGE_SVG_DEFAULT_WIDTH,
    'height' => IMAGE_SVG_DEFAULT_HEIGHT,
  );

  $file_contents = file_get_contents($uri);
  if ($file_contents) {
    $svg = simplexml_load_string($file_contents);
    foreach ($svg->attributes() as $attribute => $value) {
      if ($attribute === 'width' || $attribute === 'height') {
        $svg_dimensions[$attribute] = (int) $value;
      }
    }
  }

  // Save values to static cache.
  $cached_images[$uri] = $svg_dimensions;
  return $svg_dimensions;
}

/**
 * Determine if a file URI is an SVG image based on its file extension.
 *
 * @param string $uri
 *   A path or URI to an image.
 * @return bool
 *   TRUE if the image is an SVG. FALSE if it is not.
 */
function image_is_svg($uri) {
  $mimetype = file_get_mimetype($uri);
  return $mimetype === 'image/svg+xml';
}

/**
 * @} End of "defgroup image".
 */
