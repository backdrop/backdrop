<?php

/**
 * @file
 * Class that holds information relating to the user from node relationship.
 */
class LayoutRelationshipEntityFromField extends LayoutRelationship {

  /**
   * Get the context from this relationship.
   */
  function getContext($context) {
    // Perform access check on current logged in user.
    global $user;
    // Clone user object so account can be passed by value to access callback.
    $account = clone $user;

    $delta = !empty($this->settings['delta']) ? intval($this->settings['delta']) : 0;
    $plugin = $this->settings['relationship'];
    list($plugin, $plugin_name) = explode(':', $plugin);
    list($field_name, $from_entity, $to_entity) = explode('-', $plugin_name);
    // If unset it wants a generic, unfilled context, which is just NULL.
    $entity_info = entity_get_info($from_entity);
    if (empty($context->data) || !isset($context->data->{$entity_info['entity keys']['id']})) {
      return layout_create_context($to_entity);
    }

    if (isset($context->data->{$entity_info['entity keys']['id']})) {
      // Load the entity.
      $id = $context->data->{$entity_info['entity keys']['id']};
      $entity = entity_load($from_entity, array($id));
      $entity = $entity[$id];
      if ($items = field_get_items($from_entity, $entity, $field_name)) {
        if (isset($items[$delta])) {
          $to_entity_info = entity_get_info($to_entity);

          $plugin_info = $this->getChild($this->childDelta);;
          $to_entity_id = $items[$delta][$plugin_info['source key']];
          $loaded_to_entity = entity_load($to_entity, array($to_entity_id));
          $loaded_to_entity = array_shift($loaded_to_entity);

          // Pass current user account and entity type to access callback.
          // @todo: we dont have 'access callback' in core entity API. Leave for contrib?
          if (isset($to_entity_info['access callback']) && function_exists($to_entity_info['access callback']) && !call_user_func($to_entity_info['access callback'], 'view', $loaded_to_entity, $account, $to_entity)) {
            return layout_create_context($to_entity);
          }
          else {
            $to_entity_context = layout_create_context($to_entity);
            $to_entity_context->setData($loaded_to_entity);
            return $to_entity_context;
          }
        }
        else {
          // In case that delta was empty.
          return layout_create_context($to_entity);
        }
      }
    }
  }

  /**
   * Return a summary for this relationship.
   */
  function getAdminSummary() {
    $child = $this->getChild();
    $title = $child['title'];

    if (!empty($this->settings['context_parent'])) {
      list($label, $value) = explode(':', $this->settings['context_parent']);
      if ($label == 'position') {
        return t('@title in position @position', array('@title' => $title, '@position' => $value+1));
      }
      else {
        return t('@title ID: @id', array('@title' => $title, '@id' => $value));
      }
    }
    return t('Entity from field');
  }

  function getChild() {
    $children = $this->getChildren();
    return $children['entity_from_field:' . $this->childDelta];
  }

  /**
   * Return child relationship plugins.
   */
  function getChildren() {
    $parent = 'entity_from_field';
    $info = layout_get_relationship_info($parent);
    $cache = &backdrop_static(__FUNCTION__);
    if (!empty($cache[$parent])) {
      return $cache[$parent];
    }

    $entities = entity_get_info();
    $children = array();
    $context_types = array();

    // Get the schema information for every field.
    $fields_info = field_info_fields();
    foreach ($fields_info as $field_name => $field) {
      foreach ($field['bundles'] as $from_entity => $bundles) {
        foreach ($bundles as $bundle) {
          // There might be fields attached to bundles that are disabled (e.g. a
          // module that declared a node's content type, is now disabled), but the
          // field instance is still shown.
          if (!empty($entities[$from_entity]['bundles'][$bundle])) {
            $foreign_keys = $this->entity_from_field_foreign_keys($field_name);

            foreach ($foreign_keys as $key => $info) {
              if (isset($info['table'])) {
                foreach ($entities as $to_entity => $to_entity_info) {
                  $from_entity_info = $entities[$from_entity];
                  // If somehow the bundle doesn't exist on the to-entity,
                  // skip.
                  if (!isset($from_entity_info['bundles'][$bundle])) {
                    continue;
                  }

                  if (isset($to_entity_info['base table']) && $to_entity_info['base table'] == $info['table'] && array_keys($info['columns'], $to_entity_info['entity keys']['id'])) {
                    $name = $field_name . '-' . $from_entity . '-' . $to_entity;
                    $plugin_id = $parent . ':' . $name;

                    // Record the bundle for later.
                    $context_types[$plugin_id]['types'][$bundle] = $from_entity_info['bundles'][$bundle]['label'];

                    // We check for every bundle; this plugin may already have
                    // been created, so don't recreate it.
                    if (!isset($children[$plugin_id])) {
                      $replacements = array(
                        '@to_entity' => $to_entity_info['label'],
                        '@from_entity' => $from_entity_info['label'],
                        '@field_name' => $field_name,
                        '@field_label' => $this->entity_from_field_label($field_name),
                      );
                      $info['title'] = t('@to_entity from @from_entity (on @from_entity: @field_label [@field_name])', $replacements);
                      $info['name'] = $plugin_id;
                      $info['description'] = t('Creates a @to_entity context from @from_entity using the @field_name field on @from_entity.', $replacements);
                      $info['context'] = $from_entity;
                      $info['context_label'] = $from_entity_info['label'];
                      $info['from entity'] = $from_entity;
                      $info['to entity'] = $to_entity;
                      $info['field name'] = $field_name;
                      $info['source key'] = current(array_keys($info['columns']));

                      $children[$plugin_id] = $info;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    $cache[$parent] = $children;

    return $children;
    
  }

  function entity_from_field_foreign_keys($field_name) {
    $foreign_keys = &backdrop_static(__FUNCTION__, array());
    if (!isset($foreign_keys[$field_name])) {
      $foreign_keys[$field_name] = array();
      $field = field_info_field($field_name);

      if (!empty($field['foreign keys'])) {
        $foreign_keys[$field_name] = $field['foreign keys'];
      }
      else {
        // try to fetch foreign keys from schema, as not everything
        // stores foreign keys properly in the field info.
        $module = $field['module'];

        module_load_install($module);
        $schema = module_invoke($module, 'field_schema', $field);
        if (!empty($schema['foreign keys'])) {
          $foreign_keys[$field_name] = $schema['foreign keys'];
        }
      }
    }

    return $foreign_keys[$field_name];
  }

  function entity_from_field_label($field_name) {
    $label_counter = array();
    // Count the amount of instances per label per field.
    $instances = field_info_instances();
    foreach ($instances as $entity_type) {
      foreach ($entity_type as $bundle) {
        if (isset($bundle[$field_name])) {
          $label_counter[$bundle[$field_name]['label']] = isset($label_counter[$bundle[$field_name]['label']]) ? ++$label_counter[$bundle[$field_name]['label']] : 1;
        }
      }
    }
    if (empty($label_counter)) {
      return $field_name;
    }
    // Sort the field lables by it most used label and return the most used one.
    arsort($label_counter);
    $label_counter = array_keys($label_counter);
    return $label_counter[0];
  }

  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $child_relationship = $this->getChild();
    $field = field_info_field($child_relationship['field name']);
    $settings = $this->settings;

    if ($field && $field['cardinality'] != 1) {
      if ($field['cardinality'] == -1) {
        $form['delta'] = array(
          '#type' => 'textfield',
          '#title' => t('Delta'),
          '#description' => t('The relationship can only create one context, but multiple items can be related. Please select which one. Since this can have unlimited items, type in the number you want. The first one will be 0.'),
          '#default_value' => !empty($settings['delta']) ? $settings['delta'] : 0,
        );
      }
      else {
        $form['delta'] = array(
          '#type' => 'select',
          '#title' => t('Delta'),
          '#description' => t('The relationship can only create one context, but multiple items can be related. Please select which one.'),
          '#options' => range(1, $field['cardinality']),
          '#default_value' => !empty($settings['delta']) ? $settings['delta'] : 0,
        );
      }
    }

    return $form;
  }
}
