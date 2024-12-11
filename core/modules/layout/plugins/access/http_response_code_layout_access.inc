<?php
/**
 * @file
 * Plugin to provide access control by the server HTTP response code.
 */
class HttpResponseCodeLayoutAccess extends LayoutAccessNegatable {
  /**
   * {@inheritdoc}
   */
  public function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $response_codes = isset($this->settings['http_response_codes']) ? $this->settings['http_response_codes'] : array();

    $checkboxes_description = '';
    $negate_description = '';
    if (isset($form_state['block'])) {
      $checkboxes_description = t('Shows the block on pages with any matching response code.');
      $negate_description = t('Hides the block on pages with any matching response code selected from the options above.');
    }
    elseif (isset($form_state['layout'])) {
      $checkboxes_description = t('This layout will apply on pages with any matching response code.');
      $negate_description = t('This layout will apply on all pages, except those with any matching response code selected from the options above.');
    }
    $form['http_response_codes'] = array(
      '#type' => 'checkboxes',
      '#title' => t('HTTP response status code'),
      '#options' => array(
        200 => t('Success (200)'),
        403 => t('Access denied (403)'),
        404 => t('Page not found (404)'),
      ),
      '#required' => TRUE,
      '#default_value' => $response_codes,
      '#description' => $checkboxes_description,
    );
    $form['negate']['#description'] = $negate_description;
    // Do not tick the negation checkbox if no status code has been selected.
    $form['negate']['#default_value'] = empty($response_codes) ? 0 : $this->settings['negate'];
    // Negation option should only be available if at least one status code has
    // been selected.
    $form['negate']['#states'] = array(
      'enabled' => array(
        array(':input[name="http_response_codes[200]"]' => array('checked' => TRUE)),
        array(':input[name="http_response_codes[403]"]' => array('checked' => TRUE)),
        array(':input[name="http_response_codes[404]"]' => array('checked' => TRUE)),
      ),
    );
  }

  /**
   * {@inheritdoc}
   */
  public function checkAccess() {
    $allowed_status_codes = array_filter($this->settings['http_response_codes']);

    $http_response_header = backdrop_get_http_header('Status');
    if ($http_response_header) {
      $current_status_code = substr($http_response_header, 0, 3);
    }
    // Response header might not be set, which indicates a normal page load.
    else {
      $current_status_code = '200';
    }

    $access = in_array($current_status_code, $allowed_status_codes);

    return $this->settings['negate'] ? !$access : $access;
  }

  /**
   * {@inheritdoc}
   */
  public function summary() {
    $allowed_status_codes = array_filter($this->settings['http_response_codes']);

    if (count($allowed_status_codes)) {
      if ($this->settings['negate']) {
        $summary = format_plural(count($allowed_status_codes), 'Server response code is not @response', 'Server response code is not one of: @response', array('@response' => implode(', ', $allowed_status_codes)));
      }
      else {
        $summary = format_plural(count($allowed_status_codes), 'Server response code is @response', 'Server response code is one of: @response', array('@response' => implode(', ', $allowed_status_codes)));
      }
    }
    else {
      $summary = t('Shown on all pages where this layout applies.');
    }

    return $summary;
  }
}
