<?php

/**
 * @file
 * Provides a relationship handler for an entity from a field.
 */
class LayoutRelationshipEntityFromSchema extends LayoutRelationship {
  
  function getChild() {
    $children = $this->getChildren();
    return $children['entity_from_schema:' . $this->childDelta];
  }

  /**
   * Return child relationship plugins.
   */
  function getChildren() {
    $entities = entity_get_info();
    $plugins = array();
    $parent = 'entity_from_schema';
    $parent_plugin = layout_get_relationship_info($parent);

    foreach (module_implements('entity_info') as $module) {
      module_load_install($module);
      $schemas = backdrop_get_schema();

      foreach ($entities as $from_entity => $from_entity_info) {
        if (empty($from_entity_info['base table'])) {
          continue;
        }

        $table = $from_entity_info['base table'];
        if (isset($schemas[$table]['foreign keys'])) {
          foreach ($schemas[$table]['foreign keys'] as $relationship => $info) {
            foreach ($entities as $to_entity => $to_entity_info) {
              if (empty($info['table'])) {
                continue;
              }

              if (isset($to_entity_info['base table']) && $info['table'] == $to_entity_info['base table'] && in_array($to_entity_info['entity keys']['id'], $info['columns'])) {
                $this_col = $this->entity_from_schema_columns_filter($info['columns'], $to_entity_info['entity keys']['id']);

                // Set up our t() replacements as we reuse these.
                $replacements = array(
                  '@relationship' => $relationship,
                  '@base_table' => $table,
                  '@to_entity' => $to_entity_info['label'],
                  '@from_entity' => $from_entity_info['label'],
                );

                $name = $this_col . '-' . $from_entity . '-' . $to_entity;
                $plugin_id = $parent . ':' . $name;
                $plugin = $parent_plugin;

                $plugin['title'] = t('@to_entity from @from_entity (on @base_table.@relationship)', $replacements);
                $plugin['keyword'] = $to_entity;
                $plugin['context name'] = $name;
                $plugin['context'] = $from_entity;
                $plugin['context_label'] = $from_entity_info['label'];
                $plugin['name'] = $plugin_id;
                $plugin['description'] = t('Builds a relationship from a @from_entity to a @to_entity using the @base_table.@relationship field.', $replacements);

                // $plugin['required context'] = new ctools_context_required($from_entity_info['label'], $from_entity);

                $plugin_entities = array('to' => $to_entity_info, 'from' => $from_entity_info);
                $plugin_entities = array('to' => array($to_entity => $to_entity_info), 'from' => array($from_entity => $from_entity_info));

                // backdrop_alter('ctools_entity_context', $plugin, $plugin_entities, $plugin_id);
                $plugins[$plugin_id] = $plugin;

                // Add the relation in the reverse direction.
                $name = $this_col . '-' . $to_entity . '-' . $from_entity;
                $plugin_id = $parent . ':' . $name;
                $plugin = $parent_plugin;

                $plugin['title'] = t('@from_entity from @to_entity (on @base_table.@relationship)', $replacements);
                $plugin['keyword'] = $to_entity;
                $plugin['context name'] = $name;
                $plugin['context'] = $from_entity;
                $plugin['context_label'] = $from_entity_info['label'];
                $plugin['name'] = $plugin_id;
                $plugin['description'] = t('Builds a relationship from a @to_entity to a @from_entity using the @base_table.@relationship field.', $replacements);

                // $plugin['required context'] = new ctools_context_required($to_entity_info['label'], $to_entity);

                $plugin_entities = array('to' => $from_entity_info, 'from' => $to_entity_info);
                $plugin_entities = array('to' => array($from_entity => $from_entity_info), 'from' => array($to_entity => $to_entity_info));
                // backdrop_alter('ctools_entity_context', $plugin, $plugin_entities, $plugin_id);
                 $plugins[$plugin_id] = $plugin;

              }
            }
          }
        }
      }
    }

    return $plugins;
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
    return t('Entity from schema');
  }

  function entity_from_schema_columns_filter($columns, $value) {
    foreach ($columns as $this_col => $that_col) {
      if ($value == $that_col) {
        return $this_col;
      }
    }
  }

  /**
   * Get the context from this relationship.
   */
  function getContext($context) {
    $plugin = $this->settings['relationship'];
    list($plugin, $plugin_name) = explode(':', $plugin);
    list($this_col, $from_entity, $to_entity) = explode('-', $plugin_name);
    // If unset it wants a generic, unfilled context, which is just NULL.
    $entity_info = entity_get_info($from_entity);
    if (empty($context->data) || !isset($context->data->{$entity_info['entity keys']['id']})) {
      return layout_create_context($to_entity, array('label' => $this->label()));
    }

    if (isset($context->data->{$entity_info['entity keys']['id']})) {
      // Load the entity.
      $id = $context->data->{$entity_info['entity keys']['id']};
      $entity = entity_load($from_entity, array($id));
      $entity = $entity[$id];
      if (isset($entity->$this_col)) {
        $to_entity_id = $entity->$this_col;

        $loaded_to_entity = entity_load($to_entity, array($to_entity_id));
        $loaded_to_entity = array_shift($loaded_to_entity);
        $to_entity_context =  layout_create_context($to_entity, array('label' => $this->label()));
        $to_entity_context->setData($loaded_to_entity);
        return $to_entity_context;
      }
    }
  }


  /**
   * Assemble a human-readable label of this object.
   */
  function label() {
    $child = $this->getChild();
    return $child['title'];
  }
}