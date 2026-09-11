<?php
/**
 * @file
 * Query code for MySQL embedded database engine.
 */

/**
 * @addtogroup database
 * @{
 */

/**
 * MySQL-specific class for executing INSERT queries.
 */
class InsertQuery_mysql extends InsertQuery {
  /**
   * {@inheritdoc}
   */
  public function execute() {
    if (!$this->preExecute()) {
      return NULL;
    }

    // If we're selecting from a SelectQuery, finish building the query and
    // pass it back, as any remaining options are irrelevant.
    if (empty($this->fromQuery)) {
      $max_placeholder = 0;
      $values = array();
      foreach ($this->insertValues as $insert_values) {
        foreach ($insert_values as $value) {
          $values[':db_insert_placeholder_' . ($max_placeholder++)] = $value;
        }
      }
    }
    else {
      $values = $this->fromQuery->getArguments();
    }

    $last_insert_id = $this->connection->query((string) $this, $values, $this->queryOptions);

    // Re-initialize the values array so that we can re-use this query.
    $this->insertValues = array();

    return $last_insert_id;
  }

  /**
   * {@inheritdoc}
   */
  public function __toString() {
    // Create a sanitized comment string to prepend to the query.
    $comments = $this->connection->makeComment($this->comments);

    // Default fields are always placed first for consistency.
    $insert_fields = array_merge($this->defaultFields, $this->insertFields);

    // Escape each field.
    foreach ($insert_fields as $field_key => $insert_field) {
      $insert_fields[$field_key] = $this->connection->escapeField($insert_field);
    }

    // If we're selecting from a SelectQuery, finish building the query and
    // pass it back, as any remaining options are irrelevant.
    if (!empty($this->fromQuery)) {
      $insert_fields_string = $insert_fields ? ' (' . implode(', ', $insert_fields) . ') ' : ' ';
      return $comments . 'INSERT INTO {' . $this->table . '}' . $insert_fields_string . $this->fromQuery;
    }

    $query = $comments . 'INSERT INTO {' . $this->table . '} (' . implode(', ', $insert_fields) . ') VALUES ';

    $max_placeholder = 0;
    $values = array();
    if (!is_array($this->insertValues)) {
      // Older PHP versions cannot throw an exception within __toString().
      if (version_compare(PHP_VERSION, '7.4', '>=')) {
        throw new UnexpectedValueException();
      }
      else {
        trigger_error('Unexpected Value');
      }
    }
    if (count($this->insertValues)) {
      foreach ($this->insertValues as $insert_values) {
        $placeholders = array();

        // Default fields aren't really placeholders, but this is the most
        // convenient way to handle them.
        $placeholders = array_pad($placeholders, count($this->defaultFields), 'default');

        $new_placeholder = $max_placeholder + count($insert_values);
        for ($i = $max_placeholder; $i < $new_placeholder; ++$i) {
          $placeholders[] = ':db_insert_placeholder_' . $i;
        }
        $max_placeholder = $new_placeholder;
        $values[] = '(' . implode(', ', $placeholders) . ')';
      }
    }
    else {
      // If there are no values, then this is a default-only query. We still
      // need to handle that.
      $placeholders = array_fill(0, count($this->defaultFields), 'default');
      $values[] = '(' . implode(', ', $placeholders) . ')';
    }

    $query .= implode(', ', $values);

    return $query;
  }
}

/**
 * MySQL-specific class for executing TRUNCATE queries.
 *
 * Note: This has been used previously but is now empty.
 *
 * @todo Remove in Backdrop 2.x.
 */
class TruncateQuery_mysql extends TruncateQuery { }

/**
 * @} End of "addtogroup database".
 */
