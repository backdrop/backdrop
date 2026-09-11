<?php
/**
 * @file Contains the BackupMySQL class.
 */

/**
 * Creates and restores backups from a MySQL database source.
 *
 * @ingroup backup_restore
 */
class BackupMySql extends BackupDatabase {

  /**
   * The table's data keyed by table name.
   *
   * @var array
   */
  protected $tableData = array();

  /**
   * The tables keyed by name.
   *
   * @var array
   */
  protected $tableNames = array();

  /**
   * The views keyed by name.
   *
   * @var array
   */
  protected $viewNames = array();

  /**
   * Constructor for creating a new MySQL-based backup.
   */
  public function __construct($backup_name, $backup_target, $settings) {
    parent::__construct($backup_name, $backup_target, $settings);
  }

  /**
   * {@inheritdoc}
   */
  public function databaseInfo() {
    $database = $this->getDatabaseConnection();
    return array(
      'type' => 'mysql',
      'version' => $database ? $database->version() : t('Unknown'),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function defaultSettings() {
    $settings = parent::defaultSettings();
    $settings += array(
      'data_rows_per_query' => 50000,
      'data_rows_per_line' => 10,
      'data_bytes_per_line' => 2048,
    );
    return $settings;
  }

  /**
   * {@inheritdoc}
   */
  public static function applies($target) {
    list($target_type, $target_key) = explode(':', $target, 2);
    if ($target_type === 'db') {
      $connection = Database::getConnection($target_key);
      return $connection->driver() === 'mysql';
    }
    return FALSE;
  }

  /**
   * {@inheritdoc}
   */
  protected function backupDatabaseToFile(BackupFile $file) {
    $lines = 0;
    $include = $this->settings['include_tables'];
    $exclude = $this->settings['exclude_tables'];
    $nodata = $this->settings['nodata_tables'];
    if ($file->open(TRUE)) {
      $file->write($this->getSqlFileHeader());
      $all_tables = $this->getTables();
      $all_views = $this->getViews();

      foreach ($all_tables as $table) {
        if ($this->timeoutCheck()) {
          return FALSE;
        }
        // If there are any included tables specified, they take precedence.
        if ($include) {
          $included_table = in_array($table['name'], $include);
        }
        else {
          $included_table = !in_array($table['name'], $exclude);
        }
        if ($included_table) {
          $file->write($this->getTableStructureSql($table));
          $lines++;
          if (!in_array($table['name'], $nodata)) {
            $lines += $this->dumpTableDataSqlToFile($file, $table);
          }
        }
      }
      foreach ($all_views as $view) {
        if ($this->timeoutCheck()) {
          return FALSE;
        }
        if ($view['name'] && !isset($exclude[$view['name']])) {
          $file->write($this->getViewCreateSql($view));
        }

      }
      $file->write($this->getSqlFileFooter());
      $file->close();
      return $lines;
    }
    else {
      return FALSE;
    }
  }

  /**
   * {@inheritdoc}
   */
  protected function restoreDatabaseFromFile(BackupFile $file) {
    $lines_executed = 0;

    if ($file->open() && $connection = $this->getDatabaseConnection()) {
      // Read one line at a time and run the query.
      while ($line = $this->readSqlCommandFromFile($file)) {
        if ($this->timeoutCheck()) {
          return FALSE;
        }

        // Prepare and execute the statement instead of the api function to
        // avoid substitution of '{' etc.
        $statement = $connection->prepare($line);
        $statement->execute();
        $lines_executed++;
      }
      // Close the file with fclose/gzclose.
      $file->close();
    }
    else {
      backdrop_set_message(t('Unable to open file %file to restore database', array('%file' => $file->filepath())), 'error');
      return FALSE;
    }
    return $lines_executed;
  }

  /**
   * Read a multiline sql command from a file.
   *
   * Supports the formatting created by mysqldump, but won't handle multiline
   * comments.
   */
  private function readSqlCommandFromFile(BackupFile $file) {
    $out = '';
    while ($line = $file->read()) {
      $first2 = substr($line, 0, 2);
      $first3 = substr($line, 0, 3);

      // Ignore single line comments. This function doesn't support multiline
      // comments or inline comments.
      if ($first2 != '--' && ($first2 != '/*' || $first3 == '/*!')) {
        $out .= ' ' . trim($line);
        // If a line ends in ; or */ it is a sql command.
        if (substr($out, strlen($out) - 1, 1) == ';') {
          return trim($out);
        }
      }
    }
    return trim($out);
  }

  /**
   * {@inheritdoc}
   */
  protected function getTableNames() {
    if (empty($this->tableNames)) {
      $this->tableNames = $this->query("SHOW FULL TABLES WHERE Table_Type = 'BASE TABLE'")
        ->fetchCol();
    }

    return $this->tableNames;
  }

  /**
   * {@inheritdoc}
   */
  protected function getViewNames() {
    if (empty($this->viewNames)) {
      $this->viewNames = $this->query("SHOW FULL TABLES WHERE Table_Type = 'VIEW'")
        ->fetchCol();
    }

    return $this->viewNames;
  }

  /**
   * {@inheritdoc}
   */
  protected function lockTables() {
    $tables = $this->getLockedTables();
    if ($tables) {
      $tables_escaped = array();
      foreach ($tables as $table) {
        $tables_escaped[] = '`' . db_escape_table($table) . '` WRITE';
      }
      $this->query('LOCK TABLES ' . implode(', ', $tables_escaped));
    }
  }

  /**
   * {@inheritdoc}
   */
  protected function unlockTables() {
    if ($this->settings['lock_tables']) {
      $this->query('UNLOCK TABLES');
    }
  }

  /**
   * Get a list of table and view data from the database.
   */
  protected function getTableData() {
    if (empty($this->tableData)) {
      $tables = $this->query('SHOW TABLE STATUS')
        ->fetchAll(PDO::FETCH_ASSOC);

      foreach ($tables as $table) {
        // Lowercase the keys because between Drupal 7.12 and 7.13/14 the
        // default query behavior was changed.
        // See: http://drupal.org/node/1171866
        $table = array_change_key_case($table);
        $this->tableData[$table['name']] = $table;
      }
    }

    return $this->tableData;
  }

  /**
   * Get a list of tables in the database.
   */
  protected function getTables() {
    $out = array();
    foreach ($this->getTableData() as $table) {
      if (!empty($table['engine'])) {
        $out[$table['name']] = $table;
      }
    }

    return $out;
  }

  /**
   * Get a list of views in the database.
   */
  protected function getViews() {
    $out = array();
    foreach ($this->getTableData() as $table) {
      if (empty($table['engine'])) {
        $out[$table['name']] = $table;
      }
    }
    return $out;
  }

  /**
   * Get the SQL for the structure of the given table.
   */
  protected function getTableStructureSql($table) {
    $out = "";
    $result = $this->query("SHOW CREATE TABLE `" . $table['name'] . "`", array(), array('fetch' => PDO::FETCH_ASSOC));
    foreach ($result as $create) {
      // Lowercase the keys because between Drupal 7.12 and 7.13/14 the default
      // query behavior was changed. See: http://drupal.org/node/1171866
      $create = array_change_key_case($create);
      $out .= "DROP TABLE IF EXISTS `" . $table['name'] . "`;\n";
      // Remove newlines and convert " to ` because PDO seems to convert those
      // for some reason.
      $out .= strtr($create['create table'], array("\n" => ' ', '"' => '`'));
      if ($table['auto_increment']) {
        $out .= " AUTO_INCREMENT=" . $table['auto_increment'];
      }
      $out .= ";\n";
    }
    return $out;
  }

  /**
   * Get the SQL for the structure of the given view.
   */
  protected function getViewCreateSql($view) {
    $out = '';
    // Switch SQL mode to get rid of "CREATE ALGORITHM..." what requires more
    // permissions + troubles with the DEFINER user.
    $sql_mode = $this->query("SELECT @@SESSION.sql_mode")->fetchField();
    $this->query("SET sql_mode = 'ANSI'");
    $result = $this->query("SHOW CREATE VIEW `" . $view['name'] . "`", array(), array('fetch' => PDO::FETCH_ASSOC));
    $this->query("SET SQL_mode = :mode", array(':mode' => $sql_mode));
    foreach ($result as $create) {
      $out .= "DROP VIEW IF EXISTS `" . $view['name'] . "`;\n";
      $out .= "SET sql_mode = 'ANSI';\n";
      $out .= strtr($create['Create View'], "\n", " ") . ";\n";
      $out .= "SET sql_mode = '$sql_mode';\n";
    }
    return $out;
  }

  /**
   * Get the SQL to insert the data for a given table.
   */
  protected function dumpTableDataSqlToFile(BackupFile $file, $table) {
    $rows_per_query = $this->settings['data_rows_per_query'];
    $rows_per_line = $this->settings['data_rows_per_line'];
    $bytes_per_line = $this->settings['data_bytes_per_line'];

    if ($this->settings['verbose']) {
      $this->log('Table: %table', array('%table' => $table['name']), Backup::LOG_INFO);
    }

    // Escape backslashes, PHP code, special chars.
    $search = array('\\', "'", "\x00", "\x0a", "\x0d", "\x1a");
    $replace = array('\\\\', "''", '\0', '\n', '\r', '\Z');

    $lines = 0;
    $from = 0;
    $args = array('fetch' => PDO::FETCH_ASSOC);
    while ($data = $this->query("SELECT * FROM `" . $table['name'] . "`", array(), $args, $from, $rows_per_query)) {
      if ($data->rowCount() == 0) {
        break;
      }

      $rows = $bytes = 0;

      foreach ($data as $row) {
        $from++;

        // DB Escape the values.
        $items = array();
        foreach ($row as $value) {
          $items[] = is_null($value) ? "null" : "'" . str_replace($search, $replace, $value) . "'";
        }

        // If there is a row to be added.
        if ($items) {
          // Start a new line if we need to.
          if ($rows == 0) {
            $file->write("INSERT INTO `" . $table['name'] . "` VALUES ");
            $bytes = $rows = 0;
          }
          // Otherwise add a comma to end the previous entry.
          else {
            $file->write(",");
          }

          // Write the data itself.
          $sql = implode(',', $items);
          $file->write('(' . $sql . ')');
          $bytes += strlen($sql);
          $rows++;

          // Finish the last line if we've added enough items.
          if ($rows >= $rows_per_line || $bytes >= $bytes_per_line) {
            $file->write(";\n");
            $lines++;
            $bytes = $rows = 0;
          }
        }
      }

      // Finish any unfinished insert statements.
      if ($rows > 0) {
        $file->write(";\n");
        $lines++;
      }
    }

    if ($this->settings['verbose']) {
      $peak_memory_usage = format_size(memory_get_peak_usage(TRUE), LANGUAGE_SYSTEM);
      $this->log('Peak memory usage: %size', array('%size' => $peak_memory_usage), Backup::LOG_INFO);
    }

    return $lines;
  }

  /**
   * Get the db connection for the specified db.
   */
  protected function getDatabaseConnection() {
    if (!$this->connection) {
      $this->connection = parent::getDatabaseConnection();
    }
    return $this->connection;
  }

  /**
   * Run a query on this destination's database using Backdrop's MySQL engine.
   *
   * @param string $query
   *   The query string.
   * @param array $args
   *   Arguments for the query.
   * @param array $options
   *   Options to pass to the query.
   * @param int|null $from
   *   The starting point for the query; when passed will perform a queryRange()
   *   method instead of a regular query().
   * @param int|null $count
   *   The number of records to obtain from this query. Will be ignored if the
   *   $from argument is empty.
   *
   * @see DatabaseConnection_mysql::query()
   * @see DatabaseConnection_mysql::queryRange()
   */
  protected function query($query, array $args = array(), array $options = array(), $from = NULL, $count = NULL) {
    if ($connection = $this->getDatabaseConnection()) {
      // If no $from is passed in, just do a basic query.
      if (is_null($from)) {
        return $connection->query($query, $args, $options);
      }
      // The $from variable was passed in, so do a ranged query.
      else {
        return $connection->queryRange($query, $from, $count, $args, $options);
      }
    }

    return NULL;
  }

  /**
   * The header for the top of the SQL dump file.
   *
   * These commands set the connection character encoding to help prevent
   * encoding conversion issues.
   *
   * Note some versions of MySQL are sensitive to the spacing in the dump file.
   * Do not add indents or remove empty lines.
   */
  protected function getSqlFileHeader() {
    $info = $this->databaseInfo();

    return "-- Backdrop MySQL Dump
-- Backdrop Version: " . BACKDROP_VERSION . "
-- http://backdropcms.org/
--
-- Host: " . url('', array('absolute' => TRUE)) . "
-- Site Name: " . url('', array('absolute' => TRUE)) . "
-- Generation Time: " . format_date(time(), 'custom', 'r') . "
-- MySQL Version: " . $info['version'] . "

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE=NO_AUTO_VALUE_ON_ZERO */;

SET SQL_MODE=\"NO_AUTO_VALUE_ON_ZERO\";
SET NAMES utf8mb4;

";
  }

  /**
   * The footer of the SQL dump file.
   *
   * Note some versions of MySQL are sensitive to the spacing in the dump file.
   *
   * Do not add indents or remove empty lines.
   */
  protected function getSqlFileFooter() {
    return "

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
";
  }

  /**
   * {@inheritdoc}
   */
  protected function prepareEnvironment() {
    parent::prepareEnvironment();

    // Increase the MySQL wait timeout if needed to prevent disconnects.
    list(,$db_target) = explode(':', $this->getTarget(), 2);
    $options = array(
      'target' => $db_target,
    );
    $mysql_wait_timeout = db_query('SELECT @@session.wait_timeout', array(), $options)->fetchField();
    $backup_max_time = $this->settings['backup_max_time'];
    if ($mysql_wait_timeout < $backup_max_time) {
      // This does not work with placeholders, so sanitize by casting to int.
      db_query('SET SESSION wait_timeout = ' . (int) $backup_max_time, array(), $options);
    }
  }

}
