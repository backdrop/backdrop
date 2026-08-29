<?php
/**
 * @file
 * Functions to aid in presenting mysql database results as a set of pages.
 */

/**
 * Query extender for mysql pager queries.
 *
 * This is the mysql specific pager mechanism. It creates a paged query with a
 * fixed number of entries per page and simultaneously counts the total number
 * of rows.
 */
class PagerDefault_mysql extends PagerDefault {

  /**
   * Override the execute method.
   *
   * Before we run the query, we need to add pager-based range() instructions
   * to it.
   */
  public function execute() {
    global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;

    // Add convenience tag to mark that this is an extended query. We have to
    // do this in the constructor to ensure that it is set before preExecute()
    // gets called.
    if (!$this->preExecute($this)) {
      return NULL;
    }

    // A NULL limit is the "kill switch" for pager queries.
    if (empty($this->limit)) {
      return;
    }
    $this->ensureElement();

    $page = isset($_GET['page']) ? $_GET['page'] : '';

    // Convert comma-separated $page to an array, used by other functions.
    $pager_page_array = array_map('intval', explode(',', $page));

    if (!isset($pager_page_array[$this->element])) {
      $pager_page_array[$this->element] = 0;
    }

    // Rather than rely on the countQuery, add a query hint to prepare for a
    // FOUND_ROWS() call a bit later on. That means we don't need to load the
    // entire table index twice, which should see a nice speed increase when
    // paging through large tables, especially on InnoDB.
    $this->query->setHints('SQL_CALC_FOUND_ROWS');

    // We usually calculate the total number of pages as ceil(items / limit) but
    // now that we don't run a countQuery, we don't know the total number of
    // rows before running the pager_query. For now we assume that the total
    // items is the current page number multiplied by the limit. We recalculate
    // when we know the correct total number.
    $this->range((int) $page * $this->limit, $this->limit);

    // Now that we've added our pager-based range instructions, run the query
    // normally.
    $result = $this->query->execute();

    // And now we can fetch the total number of results, had there not been
    // a range.
    $pager_total_items[$this->element] = $this->connection->query('SELECT FOUND_ROWS()')->fetchField();

    // This does unfortunately mean that a user can override the page variable
    // in the query string and end up with an empty pager. Previously this would
    // make this default to the last page. We can emulate this behaviour by
    // checking the total number of rows returned and re-run an adjusted query
    // in that case.
    //
    // What we do now is check if our query range was out of bounds, which can
    // happen if the user manually overrode the page variable in the query
    // string. If no rows were returned and if we're not looking at the first
    // page of this pager, adjust the range to return the last page of results
    // only, just as we would normally do.
    if (((int) $page * $this->limit) > $pager_total_items[$this->element] && (int) $page) {
      // Calculate the new range and re-run the pager query to return the last
      // page of results.
      $page_max = ceil($pager_total_items[$this->element] / $this->limit) - 1;
      $this->range($page_max * $this->limit, $this->limit);
      // No need to re-count all rows, so remove the hint.
      $this->query->setHints('');
      $result = $this->query->execute();
    }

    // Redo the page number calculation, because we now know the total number of
    // results.
    $pager_total[$this->element] = ceil($pager_total_items[$this->element] / $this->limit);
    $pager_page_array[$this->element] = max(0, min((int) $pager_page_array[$this->element], ((int) $pager_total[$this->element]) - 1));
    $pager_limits[$this->element] = $this->limit;

    return $result;
  }
}
