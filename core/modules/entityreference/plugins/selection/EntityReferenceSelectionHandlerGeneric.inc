<?php

/**
 * @file
 * Generic Entity handler.
 *
 * The generic base implementation with a variety of overrides for core
 * entities.
 */
class EntityReferenceSelectionHandlerGeneric implements EntityReferenceSelectionHandlerInterface {

  /**
   * The entity's label key as required for matching the autocomplete string.
   * @see EntityReferenceSelectionHandlerGeneric::buildEntityFieldQuery
   *
   * @var string
   */
  protected $label_key;

  public $field;
  public $instance;
  public $entity_type;
  public $entity;

  /**
   * Implements EntityReferenceHandler::getInstance().
   */
  public static function getInstance($field, $instance = NULL, $entity_type = NULL, $entity = NULL) {
    $target_entity_type = $field['settings']['target_type'];

    // Check if the entity type does exist and has a base table.
    $entity_info = entity_get_info($target_entity_type);
    if (empty($entity_info['base table'])) {
      // Make sure the EntityReferenceSelectionHandlerBroken class is available.
      include_once(dirname(__FILE__) . '/EntityReferenceSelectionHandler.inc');
      return EntityReferenceSelectionHandlerBroken::getInstance($field, $instance);
    }

    if (class_exists($class_name = 'EntityReferenceSelectionHandlerGeneric_' . $target_entity_type)) {
      return new $class_name($field, $instance, $entity_type, $entity);
    }
    else {
      return new EntityReferenceSelectionHandlerGeneric($field, $instance, $entity_type, $entity);
    }
  }

  /**
   * Constructor for EntityReferenceSelectionHandlerGeneric.
   */
  protected function __construct($field, $instance = NULL, $entity_type = NULL, $entity = NULL) {
    $this->field = $field;
    $this->instance = $instance;
    $this->entity_type = $entity_type;
    $this->entity = $entity;
  }

  /**
   * Implements EntityReferenceHandler::settingsForm().
   */
  public static function settingsForm($field, $instance) {
    $entity_info = entity_get_info($field['settings']['target_type']);

    // Merge-in default values.
    $field['settings']['handler_settings'] += array(
      'target_bundles' => array(),
      'sort' => array(
        'type' => 'none',
      )
    );

    if (!empty($entity_info['entity keys']['bundle'])) {
      $bundles = array();
      foreach ($entity_info['bundles'] as $bundle_name => $bundle_info) {
        $bundles[$bundle_name] = $bundle_info['label'];
      }

      $form['target_bundles'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Target bundles'),
        '#options' => $bundles,
        '#default_value' => $field['settings']['handler_settings']['target_bundles'],
        '#size' => 6,
        '#multiple' => TRUE,
        '#description' => t('The bundles of the entity type that can be referenced. Optional, leave empty for all bundles.'),
        '#element_validate' => array('_entityreference_element_validate_filter'),
      );
    }
    else {
      $form['target_bundles'] = array(
        '#type' => 'value',
        '#value' => array(),
      );
    }

    $form['sort']['type'] = array(
      '#type' => 'select',
      '#title' => t('Sort by'),
      '#options' => array(
        'none' => t("Don't sort"),
        'property' => t('A property of the base table of the entity'),
        'field' => t('A field attached to this entity'),
      ),
      '#ajax' => TRUE,
      '#limit_validation_errors' => array(),
      '#default_value' => $field['settings']['handler_settings']['sort']['type'],
    );

    $form['sort']['settings'] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('entityreference-settings')),
      '#process' => array('_entityreference_form_process_merge_parent'),
    );

    if ($field['settings']['handler_settings']['sort']['type'] == 'property') {
      // Merge-in default values.
      $field['settings']['handler_settings']['sort'] += array(
        'property' => NULL,
      );

      $form['sort']['settings']['property'] = array(
        '#type' => 'select',
        '#title' => t('Sort property'),
        '#required' => TRUE,
        '#options' => backdrop_map_assoc($entity_info['schema_fields_sql']['base table']),
        '#default_value' => $field['settings']['handler_settings']['sort']['property'],
      );
    }
    elseif ($field['settings']['handler_settings']['sort']['type'] == 'field') {
      // Merge-in default values.
      $field['settings']['handler_settings']['sort'] += array(
        'field' => NULL,
      );

      $fields = array();
      foreach (field_info_instances($field['settings']['target_type']) as $bundle_name => $bundle_instances) {
        foreach ($bundle_instances as $instance_name => $instance_info) {
          $field_info = field_info_field($instance_name);
          foreach ($field_info['columns'] as $column_name => $column_info) {
            $fields[$instance_name . ':' . $column_name] = t('@label (column @column)', array('@label' => $instance_info['label'], '@column' => $column_name));
          }
        }
      }

      $form['sort']['settings']['field'] = array(
        '#type' => 'select',
        '#title' => t('Sort field'),
        '#required' => TRUE,
        '#options' => $fields,
        '#default_value' => $field['settings']['handler_settings']['sort']['field'],
      );
    }

    if ($field['settings']['handler_settings']['sort']['type'] != 'none') {
      // Merge-in default values.
      $field['settings']['handler_settings']['sort'] += array(
        'direction' => 'ASC',
      );

      $form['sort']['settings']['direction'] = array(
        '#type' => 'select',
        '#title' => t('Sort direction'),
        '#required' => TRUE,
        '#options' => array(
          'ASC' => t('Ascending'),
          'DESC' => t('Descending'),
        ),
        '#default_value' => $field['settings']['handler_settings']['sort']['direction'],
      );
    }

    return $form;
  }

  /**
   * Implements EntityReferenceHandler::getReferencableEntities().
   */
  public function getReferencableEntities($match = '', $match_operator = 'CONTAINS', $limit = 0) {
    $options = array();
    $entity_type = $this->field['settings']['target_type'];

    if (is_null($match)) {
      $match = '';
    }
    // Remove escape formatting.
    $match = trim(str_replace('""', '"', $match));

    $query = $this->buildEntityFieldQuery($match, $match_operator);
    if ($limit > 0) {
      $query->range(0, $limit);
    }

    $results = $query->execute();

    if (!empty($results[$entity_type])) {
      $entities = entity_load($entity_type, array_keys($results[$entity_type]));
      foreach ($entities as $entity_id => $entity) {
        list(,, $bundle) = entity_extract_ids($entity_type, $entity);
        $options[$bundle][$entity_id] = check_plain($this->getLabel($entity));
      }
    }

    return $options;
  }

  /**
   * Implements EntityReferenceHandler::countReferencableEntities().
   */
  public function countReferencableEntities($match = NULL, $match_operator = 'CONTAINS') {
    $query = $this->buildEntityFieldQuery($match, $match_operator);
    return $query
      ->count()
      ->execute();
  }

  /**
   * Implements EntityReferenceHandler::validateReferencableEntities().
   */
  public function validateReferencableEntities(array $ids) {
    if ($ids) {
      $entity_type = $this->field['settings']['target_type'];
      $query = $this->buildEntityFieldQuery();
      $query->entityCondition('entity_id', $ids, 'IN');
      $result = $query->execute();
      if (!empty($result[$entity_type])) {
        return array_keys($result[$entity_type]);
      }
    }

    return array();
  }

  /**
   * Implements EntityReferenceHandler::validateAutocompleteInput().
   */
  public function validateAutocompleteInput($input, &$element, &$form_state, $form) {
      $bundled_entities = $this->getReferencableEntities($input, '=', 6);
      $entities = array();
      foreach($bundled_entities as $entities_list) {
        $entities += $entities_list;
      }
      if (empty($entities)) {
        // Error if there are no entities available for a required field.
        form_error($element, t('There are no entities matching "%value"', array('%value' => $input)));
      }
      elseif (count($entities) > 5) {
        // Error if there are more than 5 matching entities.
        form_error($element, t('Many entities are called %value. Specify the one you want by appending the id in parentheses, like "@value (@id)"', array(
          '%value' => $input,
          '@value' => $input,
          '@id' => key($entities),
        )));
      }
      elseif (count($entities) > 1) {
        // More helpful error if there are only a few matching entities.
        $multiples = array();
        foreach ($entities as $id => $name) {
          $multiples[] = $name . ' (' . $id . ')';
        }
        form_error($element, t('Multiple entities match this reference; "%multiple"', array('%multiple' => implode('", "', $multiples))));
      }
      else {
        // Take the one and only matching entity.
        return key($entities);
      }
  }

  /**
   * Build an EntityFieldQuery to get referenceable entities.
   */
  protected function buildEntityFieldQuery($match = NULL, $match_operator = 'CONTAINS') {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', $this->field['settings']['target_type']);
    if (!empty($this->field['settings']['handler_settings']['target_bundles'])) {
      $query->entityCondition('bundle', $this->field['settings']['handler_settings']['target_bundles'], 'IN');
    }
    if (isset($match)) {
      if (!empty($this->label_key)) {
        $query->propertyCondition($this->label_key, $match, $match_operator);
      }
    }

    // Add a generic entity access tag to the query.
    $query->addTag($this->field['settings']['target_type'] . '_access');
    $query->addTag('entityreference');
    $query->addMetaData('field', $this->field);
    $query->addMetaData('entityreference_selection_handler', $this);

    // Add the sort option.
    if (!empty($this->field['settings']['handler_settings']['sort'])) {
      $sort_settings = $this->field['settings']['handler_settings']['sort'];
      if ($sort_settings['type'] == 'property') {
        $query->propertyOrderBy($sort_settings['property'], $sort_settings['direction']);
      }
      elseif ($sort_settings['type'] == 'field') {
        list($field, $column) = explode(':', $sort_settings['field'], 2);
        $query->fieldOrderBy($field, $column, $sort_settings['direction']);
      }
    }

    return $query;
  }

  /**
   * Implements EntityReferenceHandler::entityFieldQueryAlter().
   */
  public function entityFieldQueryAlter(SelectQueryInterface $query) {

  }

  /**
   * Helper method: pass a query to the alteration system again.
   *
   * This allow Entity Reference to add a tag to an existing query, to ask
   * access control mechanisms to alter it again.
   */
  protected function reAlterQuery(SelectQueryInterface $query, $tag, $base_table) {
    $query->addTag($tag);
    $query->addMetaData('base_table', $base_table);
    backdrop_alter(array('query', 'query_' . $tag), $query);
  }

  /**
   * Implements EntityReferenceHandler::getLabel().
   */
  public function getLabel($entity) {
    $target_type = $this->field['settings']['target_type'];
    return entity_access('view', $target_type, $entity) ? entity_label($target_type, $entity) : t(ENTITYREFERENCE_DENIED);
  }

  /**
   * Ensure a base table exists for the query.
   *
   * If we have a field-only query, we want to assure we have a base-table
   * so we can later alter the query in entityFieldQueryAlter().
   *
   * @param $query
   *   The Select query.
   *
   * @return
   *   The alias of the base-table.
   */
  public function ensureBaseTable(SelectQueryInterface $query) {
    $tables = $query->getTables();

    // Check the current base table.
    foreach ($tables as $table) {
      if (empty($table['join'])) {
        $alias = $table['alias'];
        break;
      }
    }

    if (strpos($alias, 'field_data_') !== 0) {
      // The existing base-table is the correct one.
      return $alias;
    }

    // Join the known base-table.
    $target_type = $this->field['settings']['target_type'];
    $entity_info = entity_get_info($target_type);
    $target_type_base_table = $entity_info['base table'];
    $id = $entity_info['entity keys']['id'];

    // Return the alias of the table.
    return $query->innerJoin($target_type_base_table, NULL, "%alias.$id = $alias.entity_id");
  }
}

/**
 * Override for the Node type.
 *
 * This only exists to workaround core bugs.
 */
class EntityReferenceSelectionHandlerGeneric_node extends EntityReferenceSelectionHandlerGeneric {

  protected $label_key = 'title';

  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Adding the 'node_access' tag is insufficient for nodes: need to also
    // know about the concept of 'published' and 'unpublished' if there are
    // no access control modules in use.
    if (!user_access('bypass node access') && !count(module_implements('node_grants'))) {
      $base_table = $this->ensureBaseTable($query);
      $query->condition("$base_table.status", NODE_PUBLISHED);
    }
  }
}

/**
 * Override for the User entity type.
 */
class EntityReferenceSelectionHandlerGeneric_user extends EntityReferenceSelectionHandlerGeneric {

  protected $label_key = 'name';

  public function buildEntityFieldQuery($match = NULL, $match_operator = 'CONTAINS') {
    $query = parent::buildEntityFieldQuery($match, $match_operator);

    // Adding the 'user_access' tag is insufficient for users: need to
    // also know about the concept of 'blocked' and 'active'.
    if (!user_access('administer users')) {
      $query->propertyCondition('status', 1);
    }
    return $query;
  }

  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    if (user_access('administer users')) {
      // In addition, if the user is administrator, need to make sure to
      // match the anonymous user, which doesn't actually have a name in the
      // database.
      $conditions = &$query->conditions();
      foreach ($conditions as $key => $condition) {
        if ($key !== '#conjunction' && is_string($condition['field']) && $condition['field'] === 'users.name') {
          // Remove the condition.
          unset($conditions[$key]);

          // Re-add the condition and a condition on uid = 0 so that we end up
          // with a query in the form:
          //    WHERE (name LIKE :name) OR (:anonymous_name LIKE :name AND uid = 0)
          $or = db_or();
          $or->condition($condition['field'], $condition['value'], $condition['operator']);
          // The Database layer doesn't allow us to build a condition
          // in the form ':placeholder = :placeholder2', because the 'field'
          // part of a condition is always escaped.
          // As a workaround, build a condition with no field, and concatenate
          // the field and the condition separately.
          $value_part = db_and();
          $value_part->condition('anonymous_name', $condition['value'], $condition['operator']);
          $value_part->compile(Database::getConnection(), $query);
          $or->condition(db_and()
            ->where(str_replace('`anonymous_name`', ':anonymous_name', (string) $value_part), $value_part->arguments() + array(':anonymous_name' => user_format_name(user_load(0))))
            ->condition('users.uid', 0)
          );
          $query->condition($or);
        }
      }
    }
  }
}

/**
 * Override for the Comment entity type.
 */
class EntityReferenceSelectionHandlerGeneric_comment extends EntityReferenceSelectionHandlerGeneric {

  protected $label_key = 'subject';

  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Adding the 'comment_access' tag is insufficient for comments: need to
    // also know about the concept of 'published' and 'unpublished'.
    if (!user_access('administer comments')) {
      $base_table = $this->ensureBaseTable($query);
      $query->condition("$base_table.status", COMMENT_PUBLISHED);
    }

    // Need to make sure that comments cannot be viewed when the user doesn't
    // have access to the node.
    $tables = $query->getTables();
    $base_table = key($tables);
    $node_alias = $query->innerJoin('node', 'n', '%alias.nid = ' . $base_table . '.nid');
    // Pass the query to the node access control.
    $this->reAlterQuery($query, 'node_access', $node_alias);

    // The comment entity exposes a bundle, but doesn't have a bundle column
    // in the database. Alter the query to fetch the bundle.
    $conditions = &$query->conditions();
    foreach ($conditions as $key => &$condition) {
      if ($key !== '#conjunction' && is_string($condition['field']) && $condition['field'] === $base_table . '.node_type') {
        $condition['field'] = '`' . $node_alias . '`.`type`';
        foreach ($condition['value'] as &$value) {
          if (substr($value, 0, 13) == 'comment_node_') {
            $value = substr($value, 13);
          }
        }
        break;
      }
    }

    // @see EntityReferenceHandler_node::entityFieldQueryAlter()
    if (!user_access('bypass node access') && !count(module_implements('node_grants'))) {
      $query->condition($node_alias . '.status', 1);
    }
  }
}

/**
 * Override for the File entity type.
 */
class EntityReferenceSelectionHandlerGeneric_file extends EntityReferenceSelectionHandlerGeneric {

  protected $label_key = 'filename';

  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Need to know about 'permanent' vs. 'temporary' files.
    $tables = $query->getTables();
    $base_table = key($tables);
    $query->condition('status', FILE_STATUS_PERMANENT);

    // Access control to files is a very difficult business.
    // @todo: Fix this when file access control is implemented.
    // @see File::access().
    return $query;
  }

  public function getLabel($entity) {
    // The file entity doesn't have a label. Moreover, the filename is
    // sometimes empty, so use the basename in that case.
    return $entity->filename !== '' ? $entity->filename : basename($entity->uri);
  }
}

/**
 * Override for the Taxonomy term entity type.
 */
class EntityReferenceSelectionHandlerGeneric_taxonomy_term extends EntityReferenceSelectionHandlerGeneric {

  protected $label_key = 'name';

  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Ensure that taxonomy terms cannot be viewed when the user doesn't have
    // access to the vocabulary.
    $base_table = $this->ensureBaseTable($query);
    $tag = 'taxonomy_vocabulary_access';
    $query->addTag($tag);
    backdrop_alter(array('query', 'query_' . $tag), $query);
  }

  /**
   * Implements EntityReferenceHandler::getReferencableEntities().
   */
  public function getReferencableEntities($match = NULL, $match_operator = 'CONTAINS', $limit = 0) {
    if ($match || $limit) {
      return parent::getReferencableEntities($match , $match_operator, $limit);
    }

    $options = array();
    $entity_type = $this->field['settings']['target_type'];

    $entity_info = entity_get_info('taxonomy_term');
    $bundles = !empty($this->field['settings']['handler_settings']['target_bundles']) ? $this->field['settings']['handler_settings']['target_bundles'] : array_keys($entity_info['bundles']);

    foreach ($bundles as $bundle) {
      if ($vocabulary = taxonomy_vocabulary_load($bundle)) {
        if ($terms = taxonomy_get_tree($vocabulary->machine_name, 0, NULL, TRUE)) {
          foreach ($terms as $term) {
            $options[$vocabulary->machine_name][$term->tid] = str_repeat('-', $term->depth) . check_plain(entity_label('taxonomy_term', $term));
          }
        }
      }
    }

    return $options;
  }
}
