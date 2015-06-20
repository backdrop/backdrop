<?php

/**
 * @file
 * Support for migrate module.
 *
 * With Migrate 2.4 or later, you can use the subfield syntax to set the title
 * and attributes:
 *
 * @code
 * $this->addFieldMapping('field_my_link', 'source_url');
 * $this->addFieldMapping('field_my_link:title', 'source_title');
 * $this->addFieldMapping('field_my_link:attributes', 'source_attributes');
 * @endcode
 *
 * With earlier versions of Migrate, you must pass an arguments array:
 *
 * @code
 * $link_args = array(
 *   'title' => array('source_field' => 'source_title'),
 *   'attributes' => array('source_field' => 'source_attributes'),
 * );
 * $this->addFieldMapping('field_my_link', 'source_url')
 *      ->arguments($link_args);
 * @endcode
 */

/**
 * Implements hook_migrate_api().
 */
function link_migrate_api() {
  return array(
    'api' => 2,
    'field handlers' => array('MigrateLinkFieldHandler'),
  );
}

class MigrateLinkFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('link_field'));
  }

  static function arguments($title = NULL, $attributes = NULL, $language = NULL) {
    $arguments = array();
    if (!is_null($title)) {
      $arguments['title'] = $title;
    }
    if (!is_null($attributes)) {
      $arguments['attributes'] = $attributes;
    }
    if (!is_null($language)) {
      $arguments['language'] = $language;
    }
    return $arguments;
  }

  /**
   * Implementation of MigrateFieldHandler::fields().
   *
   * @param $type
   *  The field type.
   * @param $instance
   *  Instance info for the field.
   * @param Migration $migration
   *  The migration context for the parent field. We can look at the mappings
   *  and determine which subfields are relevant.
   * @return array
   */
  public function fields($type, $instance, $migration = NULL) {
    return array(
      'title' => t('Subfield: The link title attribute'),
      'attributes' => t('Subfield: The attributes for this link'),
      'language' => t('Subfield: The language for the field'),
    );
  }

  public function prepare($entity, array $field_info, array $instance, array $values) {
    if (isset($values['arguments'])) {
      $arguments = $values['arguments'];
      unset($values['arguments']);
    }
    else {
      $arguments = array();
    }

    $language = $this->getFieldLanguage($entity, $field_info, $arguments);
    $values = array_filter($values);

    foreach ($values as $delta => $value) {
      $item = array();
      if (isset($arguments['title'])) {
        if (!is_array($arguments['title'])) {
          $item['title'] = $arguments['title'];
        }
        elseif (isset($arguments['title'][$delta])) {
          $item['title'] = $arguments['title'][$delta];
        }
      }
      if (isset($arguments['attributes'])) {
        if (is_array($arguments['attributes']) && isset($arguments['attributes'][$delta])) {
          $item['attributes'] = $arguments['attributes'][$delta];
        }
        else {
          $item['attributes'] = $arguments['attributes'];
        }
      }
      $item['url'] = $value;
      $return[$language][$delta] = $item;
    }

    return isset($return) ? $return : NULL;
  }
}
