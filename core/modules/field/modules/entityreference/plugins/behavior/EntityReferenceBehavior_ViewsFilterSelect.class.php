<?php

class EntityReferenceBehavior_ViewsFilterSelect extends EntityReference_BehaviorHandler_Abstract {

  public function views_data_alter(&$data, $field) {
    $entity_info = entity_get_info($field['settings']['target_type']);
    $field_name = $field['field_name'] . '_target_id';
    foreach ($data as $table_name => &$table_data) {
      if (isset($table_data[$field_name])) {
        // Set the entity id filter to use the in_operator handler with our
        // own callback to return the values.
        $table_data[$field_name]['filter']['handler'] = 'views_handler_filter_in_operator';
        $table_data[$field_name]['filter']['options callback'] = 'entityreference_views_handler_options_list';
        $table_data[$field_name]['filter']['options arguments'] = array($field['field_name']);
      }
    }
  }
}
