<?php
/**
 * @file
 * Date pager.
 * Works with a Date argument, the argument filters
 * the view and the pager provides back/next navigation.
 *
 * USER NOTES:
 *
 * To use this, add a pager to a view, and choose the option to 'Page by date'.
 * There are several settings:
 * - The pager id: Set an id to be used as the identifier
 *    in the url for pager values, defaults to 'date'.
 * - Pager position: Choose whether to display the date
 *    pager above, below, or both above and below the content.
 * - Link format: Choose whether the pager links will be in t
 *    he simple 'calendar/2011-12' format or the
 *    more complex 'calendar/?date=2011-12' pager format.
 *    The second one is more likely to work correctly
 *    if the pager is used in blocks and panels.
 *
 * The pager works in combination with a Date argument
 * and it will use the date fields and granularity
 * set in that argument to create its back/next links.
 * If the view has no Date argument, the pager can
 * do nothing. The argument can either be a 'Date' argument
 * that lets you select one or more date fields
 * in the argument, or the simple 'Content' argument for an
 * individual date field. It must be an
 * argument that uses the date argument handler.
 *
 * DEVELOPER NOTES
 *
 * The pager could technically create a query of its own rather
 * than depending on the date argument to
 * set the query, but it has only a limited set of tools to work
 * with because it is a plugin, not a handler:
 * it has no knowledge about relationships, it cannot use the
 * ensure_my_table() function, plugins are not even invoked in pre_query(),
 * so can't do anything there.
 *
 * My conclusion was that the date pager simply
 * is not powerful enough to create its own queries for
 * date fields, which require very complex queries.
 * Instead, we can combine this with a date argument and
 * let the argument create the query and let the pager
 * just provide the back/next links. If there is no
 * date argument, the pager will do nothing.
 *
 * There are still other problems. The pager is not even
 * initialized until after all the handlers
 * have created their queries, so it has no chance
 * to alter values ahead of that. And the argument
 * has no knowledge of the pager, so it can't check
 * for pager values before the query is created.
 *
 * The solution used here is to let the argument create
 * the original query. The pager query
 * runs after that, so the pager checks to see
 * if there is a pager value that needs to be used in the query.
 * The date argument has identified the placeholders
 * it used in the query. So if a change is needed,
 * we can swap the pager value into the query created
 * by the date argument and adjust the
 * $view->date_info values set by the argument accordingly
 * so the theme will pick up the new information.
 */

/**
 * Example plugin to handle paging by month.
 */
// @codingStandardsIgnoreStart
class date_views_plugin_pager extends views_plugin_pager {

  /**
   * This kind of pager does not need to count the number of records.
   */
  function use_count_query() {
    return FALSE;
  }

  /**
   * Because we don't know how many pages there are, we never believe there are more records.
   */
  function has_more_records() {
    return FALSE;
  }

  /*
   * Tell Views what this pager's setting is.
   */
  function summary_title() {
    return t("Position: @position, format: @format.", array('@position' => $this->options['pager_position'], '@format' => $this->options['link_format']));
  }

  /**
   * Tell Views what options this plugin can store.
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['date_id'] = array('default' => 'date');
    $options['pager_position'] = array('default' => 'top');
    $options['link_format'] = array('default' => 'pager');
    $options['date_argument'] = array('default' => 'Unknown');
    $options['granularity'] = array('default' => 'Unknown');
    $options['skip_empty_pages'] = array('default' => FALSE);
    return $options;
  }

  /*
   * Provide the form for setting options.
   */
  function options_form(&$form, &$form_state) {
    $form['markup']['#markup'] = t('This pager works together with a Date or Content date field contextual filter. If a Date filter has been added to the view, this pager will provide back/next paging to match the granularity of that filter (i.e. paging by year, month, week, or day). The filter must also be configured to use a DATE default value. If there is no Date contextual filter on this view, or if it has not been set to use a default date, the pager will not appear.');
    $form['date_id'] = array(
      '#title' => t('Date identifier'),
      '#type' => 'textfield',
      '#description' => t('The query identifier to use when fetching date data from in the URL. Note that if you have more than one display in the same view that uses the date pager (like a page and a block), the pager id must be different for each one or both will change when the pager value changes.'),
      '#default_value' => $this->options['date_id'],
      '#required' => TRUE,
    );
    $form['pager_position'] = array(
      '#title' => t('Pager position'),
      '#type' => 'select',
      '#options' => array('bottom' => t('Bottom'), 'top' => t('Top'), 'both' => t('Both')),
      '#description' => t('Where to place the date pager, on the top, bottom, or both top and bottom of the content.'),
      '#default_value' => $this->options['pager_position'],
      '#required' => TRUE,
    );
    $form['link_format'] = array(
      '#title' => t('Link format'),
      '#type' => 'select',
      '#options' => array('pager' => t('Pager'), 'clean' => t('Clean URL')),
      '#description' => t("The format for pager link urls. With the Pager format, the links look like 'calendar/?date=2020-05'. The Clean URL format links look like 'calendar/2020-05'. The Clean format links look nicer but the Pager format links are likely to work better if the calendar is used in blocks or panels."),
      '#default_value' => $this->options['link_format'],
      '#required' => TRUE,
    );
    $form['skip_empty_pages'] = array(
      '#title' => t('Skip empty pages'),
      '#type' => 'checkbox',
      '#description' => t('When selected, the pager will not display pages with no result for the given date. This causes a slight performance degradation because two additional queries need to be executed.'),
      '#default_value' => $this->options['skip_empty_pages'],
    );
    $form['date_argument']['#type'] = 'hidden';
    $form['date_argument']['#value'] = $this->options['date_argument'];
    $form['granularity']['#type'] = 'hidden';
    $form['granularity']['#value'] = $this->options['granularity'];
  }

  /**
   * Transfer date information from the argument to the view so the pager theme can use it
   * and update the date argument value to whatever is set by the pager.
   */
  function query() {

    // By fetching our data from the exposed input, it is possible to
    // feed pager data through some method other than $_GET.
    $input = $this->view->get_exposed_input();
    $value = NULL;
    if (!empty($input) && !empty($input[$this->options['date_id']])) {
      $value = $input[$this->options['date_id']];
    }

    // Bring the argument information into the view so our theme can access it.
    $i = 0;
    foreach ($this->view->argument as $id => &$argument) {
      if (date_views_handler_is_date($argument, 'argument')) {

        // If the argument is empty, nothing to do. This could be from
        // an argument that does not set a default value.
        if (empty($argument->argument) || empty($argument->date_handler)) {
          continue;
        }

        // Storing this information in the pager so it's available for summary info.
        // The view argument information is not otherwise accessible to the pager.
        // Not working right yet, tho.
        $date_handler = $argument->date_handler;
        $this->options['date_argument'] = $id;
        $this->options['granularity'] = $argument->date_handler->granularity;

        // Reset values set by argument if pager requires it.
        if (!empty($value)) {
          $this->set_argument_value($argument, $value);
        }

        // The pager value might move us into a forbidden range, so test it.
        if ($this->date_forbid($argument)) {
          $this->view->build_info['fail'] = TRUE;
          return;
        }
        // Write date_info to store information to be used
        // in the theming functions.
        if (empty($this->view->date_info)) {
          $this->view->date_info = new stdClass();
        }
        $this->view->date_info->granularity = $argument->date_handler->granularity;
        $format = $this->view->date_info->granularity == 'week' ? DATE_FORMAT_DATETIME : $argument->sql_format;
        $this->view->date_info->placeholders = isset($argument->placeholders) ? $argument->placeholders : $argument->date_handler->placeholders;
        $this->view->date_info->date_arg = $argument->argument;
        $this->view->date_info->date_arg_pos = $i;
        $this->view->date_info->limit = $argument->limit;
        $this->view->date_info->url = $this->view->get_url();
        $this->view->date_info->pager_id = $this->options['date_id'];
        $this->view->date_info->date_pager_position = $this->options['pager_position'];
        $this->view->date_info->date_pager_format = $this->options['link_format'];
        $this->view->date_info->skip_empty_pages = $this->options['skip_empty_pages'] == 1;
        // Execute two additional queries to find
        // the previous and next page with values.
        if ($this->view->date_info->skip_empty_pages) {
          $q = clone $argument->query;
          $field = $argument->table_alias . '.' . $argument->real_field;
          $fieldsql = $date_handler->sql_field($field);
          $fieldsql = $date_handler->sql_format($format, $fieldsql);
          $q->clear_fields();
          $q->orderby = array();
          $q->set_distinct(TRUE, TRUE);
          // Date limits of this argument.
          $datelimits = $argument->date_handler->arg_range($argument->limit[0] . '--' . $argument->limit[1]);
          // Find the first two dates between the minimum date
          // and the upper bound of the current value.
          $q->add_orderby(NULL, $fieldsql, 'DESC', 'date');
          $this->set_argument_placeholders($this->view->date_info->placeholders, $datelimits[0], $argument->max_date, $q, $format);

          $compiledquery = $q->query();
          $compiledquery->range(0, 2);
          $results = $compiledquery->execute()->fetchCol(0);

          $prevdate = array_shift($results);
          $prevdatealt = array_shift($results);
          // Find the first two dates between the lower bound
          // of the current value and the maximum date.
          $q->add_orderby(NULL, $fieldsql, 'ASC', 'date');
          $this->set_argument_placeholders($this->view->date_info->placeholders, $argument->min_date, $datelimits[1], $q, $format);

          $compiledquery = $q->query();
          $compiledquery->range(0, 2);
          $results = $compiledquery->execute()->fetchCol(0);

          $nextdate = array_shift($results);
          $nextdatealt = array_shift($results);

          // Set the default value of the query to $prevfirst or $nextfirst
          // when there is no value and $prevsecond or $nextsecond is set.
          if (empty($value)) {
            // @Todo find out which of $prevdate or $nextdate is closest to the
            // default argument date value and choose that one.
            if ($prevdate && $prevdatealt) {
              $this->set_argument_value($argument, $prevdate);
              $value = $prevdate;
              $prevdate = $prevdatealt;
              // If the first next date is the same as the first previous date,
              // move to the following next date.
              if ($value == $nextdate) {
                $nextdate = $nextdatealt;
                $nextdatealt = NULL;
              }
            }
            elseif ($nextdate && $nextdatealt) {
              $this->set_argument_value($argument, $nextdate);
              $value = $nextdate;
              $nextdate = $nextdatealt;
              // If the first previous date is the same as the first next date,
              // move to the following previous date.
              if ($value == $prevdate) {
                $prevdate = $prevdatealt;
                $prevdatealt = NULL;
              }
            }
          }
          else {
            // $prevdate and $nextdate are the same as $value, so move to
            // the next values.
            $prevdate = $prevdatealt;
            $nextdate = $nextdatealt;
          }

          $this->view->date_info->prev_date = $prevdate ? new DateObject($prevdate, NULL, $format) : NULL;
          $this->view->date_info->next_date = $nextdate ? new DateObject($nextdate, NULL, $format) : NULL;
        }
        else {
          $this->view->date_info->prev_date = clone($argument->min_date);
          date_modify($this->view->date_info->prev_date, '-1 ' . $argument->date_handler->granularity);
          $this->view->date_info->next_date = clone($argument->max_date);
          date_modify($this->view->date_info->next_date, '+1 ' . $argument->date_handler->granularity);
        }
        // Write the date_info properties that depend on the current value.
        $this->view->date_info->year = date_format($argument->min_date, 'Y');
        $this->view->date_info->month = date_format($argument->min_date, 'n');;
        $this->view->date_info->day = date_format($argument->min_date, 'j');
        $this->view->date_info->week = date_week(date_format($argument->min_date, DATE_FORMAT_DATE));
        $this->view->date_info->date_range = $argument->date_range;
        $this->view->date_info->min_date = $argument->min_date;
        $this->view->date_info->max_date = $argument->max_date;
      }
      $i++;
    }

    // Is this a view that needs to be altered based on a pager value?
    // If there is pager input and the argument has set the placeholders,
    // swap the pager value in for the placeholder set by the argument.
    if (!empty($value) && !empty($this->view->date_info->placeholders)) {
      $this->set_argument_placeholders($this->view->date_info->placeholders, $this->view->date_info->min_date, $this->view->date_info->max_date, $this->view->query, $format);
    }
  }

  function set_argument_value($argument, $value) {
    $argument->argument = $value;
    $argument->date_range = $argument->date_handler->arg_range($value);
    $argument->min_date = $argument->date_range[0];
    $argument->max_date = $argument->date_range[1];
    // $argument->is_default works correctly for normal arguments, but does not
    // work correctly if we are swapping in a new value from the pager.
    $argument->is_default = FALSE;
  }

  function set_argument_placeholders($placeholders, $mindate, $maxdate, $query, $format) {
    $count = count($placeholders);
    foreach ($query->where as $group => $data) {
      foreach ($data['conditions'] as $delta => $condition) {
        if (array_key_exists('value', $condition) && is_array($condition['value'])) {
          foreach ($condition['value'] as $placeholder => $placeholder_value) {
            if (array_key_exists($placeholder, $placeholders)) {
              // If we didn't get a match, this is a > $min < $max query that uses the view
              // min and max dates as placeholders.
              $date = ($count == 2) ? $mindate : $maxdate;
              $next_placeholder = array_shift($placeholders);
              $query->where[$group]['conditions'][$delta]['value'][$placeholder] = $date->format($format);
              $count--;
            }
          }
        }
      }
    }
  }

  /**
   * Add a callback to determine if we have moved outside the valid date range for this argument.
   */
  function date_forbid($argument) {
    // See if we're outside the allowed date range for our argument.
    $limit = date_range_years($argument->options['year_range']);
    if (date_format($argument->min_date, 'Y') < $limit[0] || date_format($argument->max_date, 'Y') > $limit[1]) {
      return TRUE;
    }
    return FALSE;
  }


  function render($input) {
    // This adds all of our template suggestions based upon the view name and display id.
    $pager_theme = views_theme_functions('date_views_pager', $this->view, $this->display);
    return theme($pager_theme, array('plugin' => $this, 'input' => $input));
  }
}
// @codingStandardsIgnoreEnd
