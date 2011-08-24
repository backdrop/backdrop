<?php
/**
 * A handler to display data from entity objects.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_entity extends views_handler_field {

  /**
   * Stores the entity type which is loaded by this field.
   */
  public $entity_type;

  /**
   * Stores all entites which are in the result.
   */
  public $entities;

  /**
   * The base field of the entity type assosiated with this field.
   */
  public $base_field;

  /**
   * Overriden to add the field for the entity id.
   */
  function query() {
    $this->table_alias = $base_table = $this->view->base_table;
    $this->base_field = $this->view->base_field;

    if (!empty($this->relationship)) {
      foreach ($this->view->relationship as $relationship) {
        if ($relationship->alias == $this->relationship) {
          $base_table = $relationship->definition['base'];
          $this->table_alias = $relationship->alias;

          $table_data = views_fetch_data($base_table);
          $this->base_field = $table_data['table']['base']['field'];
        }
      }
    }

    // Add the field if the query back-end implements an add_field() method,
    // just like the default back-end.
    if (method_exists($this->query, 'add_field')) {
      $this->field_alias = $this->query->add_field($this->table_alias, $this->base_field, '');
    }
  }

  /**
   * Load the entities for all rows that are about to be displayed.
   */
  function pre_render(&$values) {
    if (!empty($values)) {
      list($this->entity_type, $this->entities) = $this->query->get_result_entities($values, !empty($this->relationship) ? $this->relationship : NULL, $this->field_alias);
    }
  }

  /**
   * Overridden to return the entity object.
   */
  function get_value($values, $field = NULL) {
    return isset($this->entities[$this->view->row_index]) ? $this->entities[$this->view->row_index] : FALSE;
  }
}
