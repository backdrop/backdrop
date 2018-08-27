<?php

/**
 * @file
 * Installation code for MySQL embedded database engine.
 */

/**
 * Specifies installation tasks for MySQL and equivalent databases.
 */
class DatabaseTasks_mysql extends DatabaseTasks {
  /**
   * The PDO driver name for MySQL and equivalent databases.
   *
   * @var string
   */
  protected $pdoDriver = 'mysql';

  /**
   * Error code for "Unknown database" error.
   *
   * @var int
   */
  protected $databaseNotFoundErrorCode = 1049;

  /**
   * Error code for "Unknown database" error.
   *
   * @var int
   */
  protected $connectionRefusedErrorCode = 2002;

  /**
   * Returns a human-readable name string for MySQL and equivalent databases.
   */
  public function name() {
    return st('MySQL, MariaDB, or equivalent');
  }

  /**
   * Returns the minimum version for MySQL.
   */
  public function minimumVersion() {
    return '5.0.15';
  }
}

