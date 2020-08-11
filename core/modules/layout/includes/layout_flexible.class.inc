<?php
/**
 * @file
 * Class that holds configuration for a flexible layout template configuration.
 */
class LayoutFlexibleTemplate {
  /**
   * The name of the layout template.
   *
   * @var string
   */
  var $name = '';

  /**
   * The human readable name of the layout template.
   *
   * @var string
   */
  var $title = '';

  /**
   * The description of the layout template.
   *
   * @var string
   */
  var $description;

  /**
   * Configuration of the rows for this layout template.
   *
   * Each row contains the following values:
   * - contains: The layout structure of the row.
   * - element: The type of element, such as "header", "div", etc.
   * - classes: Additional classes to be added to the row.
   * - container: The type of container such as "container" or "container-flex".
   * - region_names: An array of region names within this row.
   *
   * @var array
   */
  var $rows = array();

  /**
   * Constructor for a LayoutFlexibleTemplate class.
   *
   * @param array $config
   *   An array of configuration data.
   */
  function __construct(array $config = array()) {
    foreach ($config as $property => $property_value) {
      $this->{$property} = $property_value;
    }

    if (empty($this->rows)) {
      $uuid = new Uuid();
      $this->rows[$uuid->generate()] = array(
        'contains' => 'region_12',
        'element' => 'header',
        'classes' => '',
        'container' => 'container',
        'region_names' => array('region_name_0' => t('Header')),
      );
      $this->rows[$uuid->generate()] = array(
        'contains' => 'region_12',
        'element' => '',
        'classes' => '',
        'container' => 'container',
        'region_names' => array('region_name_0' => t('Content')),
      );
      $this->rows[$uuid->generate()] = array(
        'contains' => 'region_12',
        'element' => 'footer',
        'classes' => '',
        'container' => 'container',
        'region_names' => array('region_name_0' => t('Footer')),
      );
    }
  }

  /**
   * Save a flexible layout template to config.
   */
  function save() {
    $data = array(
      'name' => $this->name,
      'title' => $this->title,
      'description' => $this->description,
      'rows' => $this->rows,
    );

    config('layout.flexible.' . $this->name)
      ->setData($data)
      ->save();

    if (isset($this->is_new)) {
      unset($this->is_new);
    }

    layout_reset_caches();
    layout_flexible_reset_caches();
    backdrop_theme_rebuild();
  }

  /**
   * Delete this layout template.
   */
  function delete() {
    config('layout.flexible.' . $this->name)->delete();

    layout_reset_caches();
    layout_flexible_reset_caches();
    backdrop_theme_rebuild();
  }
}

