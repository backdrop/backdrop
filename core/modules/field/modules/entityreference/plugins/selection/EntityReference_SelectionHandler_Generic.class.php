<?php

/**
 * A generic Entity handler.
 *
 * The generic base implementation has a variety of overrides to workaround
 * core's largely deficient entity handling.
 */
class EntityReference_SelectionHandler_Generic implements EntityReference_SelectionHandler {

  /**
   * Implements EntityReferenceHandler::getInstance().
   */
  public static function getInstance($field, $instance = NULL, $entity_type = NULL, $entity = NULL) {
    $target_entity_type = $field['settings']['target_type'];

    // Check if the entity type does exist and has a base table.
    $entity_info = entity_get_info($target_entity_type);
    if (empty($entity_info['base table'])) {
      return EntityReference_SelectionHandler_Broken::getInstance($field, $instance);
    }

    if (class_exists($class_name = 'EntityReference_SelectionHandler_Generic_' . $target_entity_type)) {
      return new $class_name($field, $instance, $entity_type, $entity);
    }
    else {
      return new EntityReference_SelectionHandler_Generic($field, $instance, $entity_type, $entity);
    }
  }

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
        '#options' => drupal_map_assoc($entity_info['schema_fields_sql']['base table']),
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
  public function getReferencableEntities($match = NULL, $match_operator = 'CONTAINS', $limit = 0) {
    $options = array();
    $entity_type = $this->field['settings']['target_type'];

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
      $entities = $this->getReferencableEntities($input, '=', 6);
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
   * Build an EntityFieldQuery to get referencable entities.
   */
  protected function buildEntityFieldQuery($match = NULL, $match_operator = 'CONTAINS') {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', $this->field['settings']['target_type']);
    if (!empty($this->field['settings']['handler_settings']['target_bundles'])) {
      $query->entityCondition('bundle', $this->field['settings']['handler_settings']['target_bundles'], 'IN');
    }
    if (isset($match)) {
      $entity_info = entity_get_info($this->field['settings']['target_type']);
      if (isset($entity_info['entity keys']['label'])) {
        $query->propertyCondition($entity_info['entity keys']['label'], $match, $match_operator);
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
    // Save the old tags and metadata.
    // For some reason, those are public.
    $old_tags = $query->alterTags;
    $old_metadata = $query->alterMetaData;

    $query->alterTags = array($tag => TRUE);
    $query->alterMetaData['base_table'] = $base_table;
    drupal_alter(array('query', 'query_' . $tag), $query);

    // Restore the tags and metadata.
    $query->alterTags = $old_tags;
    $query->alterMetaData = $old_metadata;
  }

  /**
   * Implements EntityReferenceHandler::getLabel().
   */
  public function getLabel($entity) {
    return entity_label($this->field['settings']['target_type'], $entity);
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
    $id = $entity_info['entity keys']['id'];
    // Return the alias of the table.
    return $query->innerJoin($target_type, NULL, "$target_type.$id = $alias.entity_id");
  }
}

/**
 * Override for the Node type.
 *
 * This only exists to workaround core bugs.
 */
class EntityReference_SelectionHandler_Generic_node extends EntityReference_SelectionHandler_Generic {
  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Adding the 'node_access' tag is sadly insufficient for nodes: core
    // requires us to also know about the concept of 'published' and
    // 'unpublished'. We need to do that as long as there are no access control
    // modules in use on the site. As long as one access control module is there,
    // it is supposed to handle this check.
    if (!user_access('bypass node access') && !count(module_implements('node_grants'))) {
      $base_table = $this->ensureBaseTable($query);
      $query->condition("$base_table.status", NODE_PUBLISHED);
    }
  }
}

/**
 * Override for the User type.
 *
 * This only exists to workaround core bugs.
 */
class EntityReference_SelectionHandler_Generic_user extends EntityReference_SelectionHandler_Generic {
  public function buildEntityFieldQuery($match = NULL, $match_operator = 'CONTAINS') {
    $query = parent::buildEntityFieldQuery($match, $match_operator);

    // The user entity doesn't have a label column.
    if (isset($match)) {
      $query->propertyCondition('name', $match, $match_operator);
    }

    // Adding the 'user_access' tag is sadly insufficient for users: core
    // requires us to also know about the concept of 'blocked' and
    // 'active'.
    if (!user_access('administer users')) {
      $query->propertyCondition('status', 1);
    }
    return $query;
  }

  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    if (user_access('administer users')) {
      // In addition, if the user is administrator, we need to make sure to
      // match the anonymous user, that doesn't actually have a name in the
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
          // Sadly, the Database layer doesn't allow us to build a condition
          // in the form ':placeholder = :placeholder2', because the 'field'
          // part of a condition is always escaped.
          // As a (cheap) workaround, we separately build a condition with no
          // field, and concatenate the field and the condition separately.
          $value_part = db_and();
          $value_part->condition('anonymous_name', $condition['value'], $condition['operator']);
          $value_part->compile(Database::getConnection(), $query);
          $or->condition(db_and()
            ->where(str_replace('anonymous_name', ':anonymous_name', (string) $value_part), $value_part->arguments() + array(':anonymous_name' => format_username(user_load(0))))
            ->condition('users.uid', 0)
          );
          $query->condition($or);
        }
      }
    }
  }
}

/**
 * Override for the Comment type.
 *
 * This only exists to workaround core bugs.
 */
class EntityReference_SelectionHandler_Generic_comment extends EntityReference_SelectionHandler_Generic {
  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Adding the 'comment_access' tag is sadly insufficient for comments: core
    // requires us to also know about the concept of 'published' and
    // 'unpublished'.
    if (!user_access('administer comments')) {
      $base_table = $this->ensureBaseTable($query);
      $query->condition("$base_table.status", COMMENT_PUBLISHED);
    }

    // The Comment module doesn't implement any proper comment access,
    // and as a consequence doesn't make sure that comments cannot be viewed
    // when the user doesn't have access to the node.
    $tables = $query->getTables();
    $base_table = key($tables);
    $node_alias = $query->innerJoin('node', 'n', '%alias.nid = ' . $base_table . '.nid');
    // Pass the query to the node access control.
    $this->reAlterQuery($query, 'node_access', $node_alias);

    // Alas, the comment entity exposes a bundle, but doesn't have a bundle column
    // in the database. We have to alter the query ourself to go fetch the
    // bundle.
    $conditions = &$query->conditions();
    foreach ($conditions as $key => &$condition) {
      if ($key !== '#conjunction' && is_string($condition['field']) && $condition['field'] === 'node_type') {
        $condition['field'] = $node_alias . '.type';
        foreach ($condition['value'] as &$value) {
          if (substr($value, 0, 13) == 'comment_node_') {
            $value = substr($value, 13);
          }
        }
        break;
      }
    }

    // Passing the query to node_query_node_access_alter() is sadly
    // insufficient for nodes.
    // @see EntityReferenceHandler_node::entityFieldQueryAlter()
    if (!user_access('bypass node access') && !count(module_implements('node_grants'))) {
      $query->condition($node_alias . '.status', 1);
    }
  }
}

/**
 * Override for the File type.
 *
 * This only exists to workaround core bugs.
 */
class EntityReference_SelectionHandler_Generic_file extends EntityReference_SelectionHandler_Generic {
  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // Core forces us to know about 'permanent' vs. 'temporary' files.
    $tables = $query->getTables();
    $base_table = key($tables);
    $query->condition('status', FILE_STATUS_PERMANENT);

    // Access control to files is a very difficult business. For now, we are not
    // going to give it a shot.
    // @todo: fix this when core access control is less insane.
    return $query;
  }

  public function getLabel($entity) {
    // The file entity doesn't have a label. More over, the filename is
    // sometimes empty, so use the basename in that case.
    return $entity->filename !== '' ? $entity->filename : basename($entity->uri);
  }
}

/**
 * Override for the Taxonomy term type.
 *
 * This only exists to workaround core bugs.
 */
class EntityReference_SelectionHandler_Generic_taxonomy_term extends EntityReference_SelectionHandler_Generic {
  public function entityFieldQueryAlter(SelectQueryInterface $query) {
    // The Taxonomy module doesn't implement any proper taxonomy term access,
    // and as a consequence doesn't make sure that taxonomy terms cannot be viewed
    // when the user doesn't have access to the vocabulary.
    $base_table = $this->ensureBaseTable($query);
    $vocabulary_alias = $query->innerJoin('taxonomy_vocabulary', 'n', '%alias.vid = ' . $base_table . '.vid');
    $query->addMetadata('base_table', $vocabulary_alias);
    // Pass the query to the taxonomy access control.
    $this->reAlterQuery($query, 'taxonomy_vocabulary_access', $vocabulary_alias);

    // Also, the taxonomy term entity exposes a bundle, but doesn't have a bundle
    // column in the database. We have to alter the query ourself to go fetch
    // the bundle.
    $conditions = &$query->conditions();
    foreach ($conditions as $key => &$condition) {
      if ($key !== '#conjunction' && is_string($condition['field']) && $condition['field'] === 'vocabulary_machine_name') {
        $condition['field'] = $vocabulary_alias . '.machine_name';
        break;
      }
    }
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

    // We imitate core by calling taxonomy_get_tree().
    $entity_info = entity_get_info('taxonomy_term');
    $bundles = !empty($this->field['settings']['handler_settings']['target_bundles']) ? $this->field['settings']['handler_settings']['target_bundles'] : array_keys($entity_info['bundles']);

    foreach ($bundles as $bundle) {
      if ($vocabulary = taxonomy_vocabulary_machine_name_load($bundle)) {
        if ($terms = taxonomy_get_tree($vocabulary->vid, 0)) {
          foreach ($terms as $term) {
            $options[$vocabulary->machine_name][$term->tid] = str_repeat('-', $term->depth) . check_plain($term->name);
          }
        }
      }
    }

    return $options;
  }
}
