<?php
// $Id$

/**
 * @file
 * Tests for Views query features.
 */

/**
 * Abstract class for views testing
 */
abstract class ViewsTestCase extends DrupalWebTestCase {
  /**
   * Helper function: verify a result set returned by view.
   *
   * The comparison is done on the string representation of the columns of the
   * column map, taking the order of the rows into account, but not the order
   * of the columns.
   *
   * @param $view
   *  An executed View.
   * @param $expected_result
   *  An expected result set.
   * @param $column_map
   *  An associative array mapping the columns of the result set from the view
   *  (as keys) and the expected result set (as values).
   */
  protected function assertIdenticalResultset($view, $expected_result, $column_map, $message = 'Identical result set') {
    // Convert $view->result to an array of arrays.
    $result = array();
    foreach ($view->result as $key => $value) {
      $row = array();
      foreach ($column_map as $view_column => $expected_column) {
        // The comparison will be done on the string representation of the value.
        $row[$expected_column] = (string) $value->$view_column;
      }
      $result[$key] = $row;
    }

    // Remove the columns we don't need from the expected result.
    foreach ($expected_result as $key => $value) {
      $row = array();
      foreach ($column_map as $expected_column) {
        // The comparison will be done on the string representation of the value.
        $row[$expected_column] = (string) $value[$expected_column];
      }
      $expected_result[$key] = $row;
    }

    // Reset the numbering of the arrays.
    $result = array_values($result);
    $expected_result = array_values($expected_result);

    // Do the actual comparison.
    return $this->assertIdentical($result, $expected_result, $message);
  }

  /**
   * Helper function: order an array of array based on a column.
   */
  protected function orderResultSet($result_set, $column, $reverse = FALSE) {
    $this->sort_column = $column;
    $this->sort_order = $reverse ? -1 : 1;
    usort($result_set, array($this, 'helperCompareFunction'));
    return $result_set;
  }

  protected $sort_column = NULL;
  protected $sort_order = 1;

  /**
   * Helper comparison function for orderResultSet().
   */
  protected function helperCompareFunction($a, $b) {
    $value1 = $a[$this->sort_column];
    $value2 = $b[$this->sort_column];
    if ($value1 == $value2) {
        return 0;
    }
    return $this->sort_order * (($value1 < $value2) ? -1 : 1);
  }

  /**
   * Helper function to check whether a button with a certain id exists and has a certain label.
   */
  protected function helperButtonHasLabel($id, $expected_label, $message = 'Label has the expected value: %label.') {
    return $this->assertFieldById($id, $expected_label, t($message, array('%label' => $expected_label)));
  }
}

abstract class ViewsSqlTest extends ViewsTestCase {

  protected function setUp() {
    parent::setUp('views', 'views_test');

    // Load the test dataset.
    $query = db_insert('views_test')
      ->fields(array('name', 'age', 'job'));
    foreach ($this->testDataSet() as $record) {
      $query->values($record);
    }
    $query->execute();
  }

  /**
   * A very simple test dataset.
   */
  protected function testDataSet() {
    return array(
      array(
        'name' => 'John',
        'age' => 25,
        'job' => 'Singer',
      ),
      array(
        'name' => 'George',
        'age' => 27,
        'job' => 'Singer',
      ),
      array(
        'name' => 'Ringo',
        'age' => 28,
        'job' => 'Drummer',
      ),
      array(
        'name' => 'Paul',
        'age' => 26,
        'job' => 'Songwriter',
      ),
      array(
        'name' => 'Meredith',
        'age' => 30,
        'job' => 'Speaker',
      ),
    );
  }
}
