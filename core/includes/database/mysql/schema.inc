<?php
/**
 * @file
 * Database schema code for MySQL database servers.
 */

/**
 * @addtogroup schemaapi
 * @{
 */

/**
 * Class to create and manipulate MySQL tables.
 */
class DatabaseSchema_mysql extends DatabaseSchema {

  /**
   * Maximum length of a table comment in MySQL.
   */
  const COMMENT_MAX_TABLE = 60;

  /**
   * Maximum length of a column comment in MySQL.
   */
  const COMMENT_MAX_COLUMN = 255;

  /**
   * {@inheritdoc}
   */
  protected function getPrefixInfo($table = 'default', $add_prefix = TRUE) {
    $info = array('prefix' => $this->connection->tablePrefix($table));
    if ($add_prefix) {
      $table = $info['prefix'] . $table;
    }
    if (($pos = strpos($table, '.')) !== FALSE) {
      $info['database'] = substr($table, 0, $pos);
      $info['table'] = substr($table, ++$pos);
    }
    else {
      $db_info = $this->connection->getConnectionOptions();
      $info['database'] = $db_info['database'];
      $info['table'] = $table;
    }
    return $info;
  }

  /**
   * {@inheritdoc}
   *
   * MySQL uses databases like schemas rather than catalogs so when we build
   * a condition to query the information_schema.tables, we set the default
   * database as the schema unless specified otherwise, and exclude
   * table_catalog from the condition criteria.
   */
  protected function buildTableNameCondition($table_name, $operator = '=', $add_prefix = TRUE) {
    $table_info = $this->getPrefixInfo($table_name, $add_prefix);

    $condition = new DatabaseCondition('AND');
    $condition->condition('table_schema', $table_info['database']);
    $condition->condition('table_name', $table_info['table'], $operator);
    return $condition;
  }

  /**
   * {@inheritdoc}
   */
  protected function createTableSql($name, $table) {
    $info = $this->connection->getConnectionOptions();

    // Provide defaults if needed.
    $table += array(
      'mysql_engine' => 'InnoDB',
      // Allow the default charset to be overridden in settings.php.
      'mysql_character_set' => $this->connection->utf8mb4IsActive() ? 'utf8mb4' : 'utf8',
    );

    $sql = "CREATE TABLE {" . $name . "} (\n";

    // Add the SQL statement for each field.
    foreach ($table['fields'] as $field_name => $field) {
      $sql .= $this->createFieldSql($field_name, $this->processField($field)) . ", \n";
    }

    // Process keys & indexes.
    $keys = $this->createKeysSql($table);
    if (count($keys)) {
      $sql .= implode(", \n", $keys) . ", \n";
    }

    // Remove the last comma and space.
    $sql = substr($sql, 0, -3) . "\n) ";

    $sql .= 'ENGINE = ' . $table['mysql_engine'] . ' DEFAULT CHARACTER SET ' . $table['mysql_character_set'];
    // By default, MySQL uses the default collation for new tables, which is
    // 'utf8_general_ci' for utf8. If an alternate collation has been set, it
    // needs to be explicitly specified.
    // @see DatabaseConnection_mysql
    if (!empty($info['collation'])) {
      $sql .= ' COLLATE ' . $info['collation'];
    }

    // The row format needs to be either DYNAMIC or COMPRESSED in order to allow
    // for the innodb_large_prefix setting to take effect, see
    // https://dev.mysql.com/doc/refman/8.4/en/create-table.html
    if ($this->connection->utf8mb4IsActive()) {
      $sql .= ' ROW_FORMAT=DYNAMIC';
    }

    // Add table comment.
    if (!empty($table['description'])) {
      $sql .= ' COMMENT ' . $this->prepareComment($table['description'], self::COMMENT_MAX_TABLE);
    }

    return array($sql);
  }

  /**
   * Create a SQL string for a field to be used in table creation or alteration.
   *
   * Before passing a field out of a schema definition into this function it has
   * to be processed by DatabaseSchema_mysql::processField().
   *
   * @param string $name
   *   Name of the field.
   * @param array $spec
   *   The field specification, as per the schema data structure format.
   *
   * @return string
   *   A string that can be used to create or alter a table.
   */
  protected function createFieldSql($name, array $spec) {
    $sql = "`" . $name . "` " . $spec['mysql_type'];

    $string_types = array(
      'VARCHAR',
      'CHAR',
      'TINYTEXT',
      'MEDIUMTEXT',
      'LONGTEXT',
      'TEXT',
    );
    if (in_array($spec['mysql_type'], $string_types)) {
      if (isset($spec['length'])) {
        $sql .= '(' . $spec['length'] . ')';
      }
      if (!empty($spec['binary'])) {
        $sql .= ' BINARY';
      }
    }
    elseif (isset($spec['precision']) && isset($spec['scale'])) {
      $sql .= '(' . $spec['precision'] . ', ' . $spec['scale'] . ')';
    }

    if (!empty($spec['unsigned'])) {
      $sql .= ' unsigned';
    }

    if (isset($spec['not null'])) {
      if ($spec['not null']) {
        $sql .= ' NOT NULL';
      }
      else {
        $sql .= ' NULL';
      }
    }

    if (!empty($spec['auto_increment'])) {
      $sql .= ' auto_increment';
    }

    // $spec['default'] can be NULL, so we explicitly check for the key here.
    if (array_key_exists('default', $spec)) {
      if (is_string($spec['default'])) {
        $spec['default'] = "'" . $spec['default'] . "'";
      }
      elseif (!isset($spec['default'])) {
        $spec['default'] = 'NULL';
      }
      $sql .= ' DEFAULT ' . $spec['default'];
    }

    if (empty($spec['not null']) && !isset($spec['default'])) {
      $sql .= ' DEFAULT NULL';
    }

    // Add column comment.
    if (!empty($spec['description'])) {
      $sql .= ' COMMENT ' . $this->prepareComment($spec['description'], self::COMMENT_MAX_COLUMN);
    }

    return $sql;
  }

  /**
   * Set database-engine specific properties for a field.
   *
   * @param array $field
   *   A field description array, as specified in the schema documentation.
   */
  protected function processField(array $field) {

    if (!isset($field['size'])) {
      $field['size'] = 'normal';
    }

    $map = $this->getFieldTypeMap();
    $field['mysql_type'] = $map[$field['type'] . ':' . $field['size']];

    if (isset($field['type']) && $field['type'] == 'serial') {
      $field['auto_increment'] = TRUE;
    }

    return $field;
  }

  /**
   * {@inheritdoc}
   */
  public function getFieldTypeMap() {
    // Put :normal last so it gets preserved by array_flip. This makes
    // it much easier for modules (such as schema.module) to map
    // database types back into schema types.
    // $map does not use backdrop_static as its value never changes.
    static $map = array(
      'varchar:normal'  => 'VARCHAR',
      'char:normal'     => 'CHAR',

      'text:tiny'       => 'TINYTEXT',
      'text:small'      => 'TINYTEXT',
      'text:medium'     => 'MEDIUMTEXT',
      'text:big'        => 'LONGTEXT',
      'text:normal'     => 'TEXT',

      'serial:tiny'     => 'TINYINT',
      'serial:small'    => 'SMALLINT',
      'serial:medium'   => 'MEDIUMINT',
      'serial:big'      => 'BIGINT',
      'serial:normal'   => 'INT',

      'int:tiny'        => 'TINYINT',
      'int:small'       => 'SMALLINT',
      'int:medium'      => 'MEDIUMINT',
      'int:big'         => 'BIGINT',
      'int:normal'      => 'INT',

      'float:tiny'      => 'FLOAT',
      'float:small'     => 'FLOAT',
      'float:medium'    => 'FLOAT',
      'float:big'       => 'DOUBLE',
      'float:normal'    => 'FLOAT',

      'numeric:normal'  => 'DECIMAL',

      'blob:big'        => 'LONGBLOB',
      'blob:normal'     => 'BLOB',

      'date:normal'     => 'DATE',
      'datetime:normal' => 'DATETIME',
      'time:normal'     => 'TIME',
    );
    return $map;
  }

  /**
   * Generates a string suitable for creating MySQL table keys.
   *
   * @param array $spec
   *   The table specification.
   *
   * @return string[]
   *   An array of partial SQL strings for the keys specified within the table.
   */
  protected function createKeysSql(array $spec) {
    $keys = array();

    if (!empty($spec['primary key'])) {
      $keys[] = 'PRIMARY KEY (' . $this->createKeySql($spec['primary key']) . ')';
    }
    if (!empty($spec['unique keys'])) {
      foreach ($spec['unique keys'] as $key => $fields) {
        $keys[] = 'UNIQUE KEY `' . $key . '` (' . $this->createKeySql($fields) . ')';
      }
    }
    if (!empty($spec['indexes'])) {
      foreach ($spec['indexes'] as $index => $fields) {
        $keys[] = 'INDEX `' . $index . '` (' . $this->createKeySql($fields) . ')';
      }
    }

    return $keys;
  }

  /**
   * Generates a string suitable for creating key across multiple fields.
   *
   * @param array|string $fields
   *   The list of table fields to be included in the key.
   *
   * @return string
   *   The list of fields concatenated together, separated by commas.
   */
  protected function createKeySql($fields) {
    $return = array();
    foreach ($fields as $field) {
      if (is_array($field)) {
        $return[] = '`' . $field[0] . '`(' . $field[1] . ')';
      }
      else {
        $return[] = '`' . $field . '`';
      }
    }
    return implode(', ', $return);
  }

  /**
   * {@inheritdoc}
   */
  public function renameTable($table, $new_name) {
    if (!$this->tableExists($table)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot rename @table to @table_new: table @table doesn't exist.", array(
        '@table' => $table,
        '@table_new' => $new_name,
      )));
    }
    if ($this->tableExists($new_name)) {
      throw new DatabaseSchemaObjectExistsException(t("Cannot rename @table to @table_new: table @table_new already exists.", array(
        '@table' => $table,
        '@table_new' => $new_name,
      )));
    }

    $info = $this->getPrefixInfo($new_name);
    return $this->connection->query('ALTER TABLE {' . $table . '} RENAME TO `' . $info['table'] . '`');
  }

  /**
   * {@inheritdoc}
   */
  public function dropTable($table) {
    if (!$this->tableExists($table)) {
      return FALSE;
    }

    $this->connection->query('DROP TABLE {' . $table . '}');
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function addField($table, $field, $spec, $keys_new = array()) {
    if (!$this->tableExists($table)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot add field @table.@field: table doesn't exist.", array(
        '@field' => $field,
        '@table' => $table,
      )));
    }
    if ($this->fieldExists($table, $field)) {
      throw new DatabaseSchemaObjectExistsException(t("Cannot add field @table.@field: field already exists.", array(
        '@field' => $field,
        '@table' => $table,
      )));
    }

    $fix_null = FALSE;
    if (!empty($spec['not null']) && !isset($spec['default'])) {
      $fix_null = TRUE;
      $spec['not null'] = FALSE;
    }
    $query = 'ALTER TABLE {' . $table . '} ADD ';
    $query .= $this->createFieldSql($field, $this->processField($spec));
    if ($keys_sql = $this->createKeysSql($keys_new)) {
      $query .= ', ADD ' . implode(', ADD ', $keys_sql);
    }
    $this->connection->query($query);
    if (isset($spec['initial'])) {
      $this->connection->update($table)
        ->fields(array($field => $spec['initial']))
        ->execute();
    }
    if ($fix_null) {
      $spec['not null'] = TRUE;
      $this->changeField($table, $field, $field, $spec);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function dropField($table, $field) {
    if (!$this->fieldExists($table, $field)) {
      return FALSE;
    }

    $this->connection->query('ALTER TABLE {' . $table . '} DROP `' . $field . '`');
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function fieldSetDefault($table, $field, $default) {
    if (!$this->fieldExists($table, $field)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot set default value of field @table.@field: field doesn't exist.", array(
        '@table' => $table,
        '@field' => $field,
      )));
    }

    if (!isset($default)) {
      $default = 'NULL';
    }
    else {
      $default = is_string($default) ? "'$default'" : $default;
    }

    $this->connection->query('ALTER TABLE {' . $table . '} ALTER COLUMN `' . $field . '` SET DEFAULT ' . $default);
  }

  /**
   * {@inheritdoc}
   */
  public function fieldSetNoDefault($table, $field) {
    if (!$this->fieldExists($table, $field)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot remove default value of field @table.@field: field doesn't exist.", array(
        '@table' => $table,
        '@field' => $field,
      )));
    }

    $this->connection->query('ALTER TABLE {' . $table . '} ALTER COLUMN `' . $field . '` DROP DEFAULT');
  }

  /**
   * {@inheritdoc}
   */
  public function indexExists($table, $name) {
    // Returns one row for each column in the index. Result is string or FALSE.
    // Details at https://dev.mysql.com/doc/refman/8.4/en/show-index.html
    $row = $this->connection->query('SHOW INDEX FROM {' . $table . "} WHERE key_name = '$name'")->fetchAssoc();
    return isset($row['Key_name']);
  }

  /**
   * {@inheritdoc}
   */
  public function addPrimaryKey($table, $fields) {
    if (!$this->tableExists($table)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot add primary key to table @table: table doesn't exist.", array(
        '@table' => $table,
      )));
    }
    if ($this->indexExists($table, 'PRIMARY')) {
      throw new DatabaseSchemaObjectExistsException(t("Cannot add primary key to table @table: primary key already exists.", array(
        '@table' => $table,
      )));
    }

    $this->connection->query('ALTER TABLE {' . $table . '} ADD PRIMARY KEY (' . $this->createKeySql($fields) . ')');
  }

  /**
   * {@inheritdoc}
   */
  public function dropPrimaryKey($table) {
    if (!$this->indexExists($table, 'PRIMARY')) {
      return FALSE;
    }

    $this->connection->query('ALTER TABLE {' . $table . '} DROP PRIMARY KEY');
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function addUniqueKey($table, $name, $fields) {
    if (!$this->tableExists($table)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot add unique key @name to table @table: table doesn't exist.", array(
        '@table' => $table,
        '@name' => $name,
      )));
    }
    if ($this->indexExists($table, $name)) {
      throw new DatabaseSchemaObjectExistsException(t("Cannot add unique key @name to table @table: unique key already exists.", array(
        '@table' => $table,
        '@name' => $name,
      )));
    }

    $this->connection->query('ALTER TABLE {' . $table . '} ADD UNIQUE KEY `' . $name . '` (' . $this->createKeySql($fields) . ')');
  }

  /**
   * {@inheritdoc}
   */
  public function dropUniqueKey($table, $name) {
    if (!$this->indexExists($table, $name)) {
      return FALSE;
    }

    $this->connection->query('ALTER TABLE {' . $table . '} DROP KEY `' . $name . '`');
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function addIndex($table, $name, $fields) {
    if (!$this->tableExists($table)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot add index @name to table @table: table doesn't exist.", array(
        '@table' => $table,
        '@name' => $name,
      )));
    }
    if ($this->indexExists($table, $name)) {
      throw new DatabaseSchemaObjectExistsException(t("Cannot add index @name to table @table: index already exists.", array(
        '@table' => $table,
        '@name' => $name,
      )));
    }

    $this->connection->query('ALTER TABLE {' . $table . '} ADD INDEX `' . $name . '` (' . $this->createKeySql($fields) . ')');
  }

  /**
   * {@inheritdoc}
   */
  public function dropIndex($table, $name) {
    if (!$this->indexExists($table, $name)) {
      return FALSE;
    }

    $this->connection->query('ALTER TABLE {' . $table . '} DROP INDEX `' . $name . '`');
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  public function changeField($table, $field, $field_new, $spec, $keys_new = array()) {
    if (!$this->fieldExists($table, $field)) {
      throw new DatabaseSchemaObjectDoesNotExistException(t("Cannot change the definition of field @table.@name: field doesn't exist.", array(
        '@table' => $table,
        '@name' => $field,
      )));
    }
    if (($field != $field_new) && $this->fieldExists($table, $field_new)) {
      throw new DatabaseSchemaObjectExistsException(t("Cannot rename field @table.@name to @name_new: target field already exists.", array(
        '@table' => $table,
        '@name' => $field,
        '@name_new' => $field_new,
      )));
    }

    $sql = 'ALTER TABLE {' . $table . '} CHANGE `' . $field . '` ' . $this->createFieldSql($field_new, $this->processField($spec));
    if ($keys_sql = $this->createKeysSql($keys_new)) {
      $sql .= ', ADD ' . implode(', ADD ', $keys_sql);
    }
    $this->connection->query($sql);
  }

  /**
   * {@inheritdoc}
   */
  public function prepareComment($comment, $length = NULL) {
    // Work around a bug in some versions of PDO.
    // See http://bugs.php.net/bug.php?id=41125
    $comment = str_replace("'", '’', $comment);

    // Prefix tables, but remove any back-ticks.
    $comment = str_replace('`', '', $this->connection->prefixTables($comment));

    // Truncate comment to maximum comment length.
    if (isset($length)) {
      $comment = truncate_utf8($comment, $length, TRUE, TRUE);
    }

    return $this->connection->quote($comment);
  }

  /**
   * Retrieve a table or column comment.
   */
  public function getComment($table, $column = NULL) {
    $condition = $this->buildTableNameCondition($table);
    if (isset($column)) {
      $condition->condition('column_name', $column);
      $condition->compile($this->connection, $this);
      // Don't use {} around information_schema.columns table.
      return $this->connection->query("SELECT column_comment FROM information_schema.columns WHERE " . (string) $condition, $condition->arguments())->fetchField();
    }
    $condition->compile($this->connection, $this);
    // Don't use {} around information_schema.tables table.
    return $this->connection->query("SELECT table_comment FROM information_schema.tables WHERE " . (string) $condition, $condition->arguments())->fetchField();
  }

  /**
   * {@inheritdoc}
   */
  public function findTables($table_expression) {
    // Back-ticks need to be removed when selecting from the information_schema
    // table. These may be added if using DatabaseConnection::prefixTables() on
    // a table name.
    $table_expression = str_replace('`', '', $table_expression);
    return parent::findTables($table_expression);
  }

}

/**
 * @} End of "addtogroup schemaapi".
 */
