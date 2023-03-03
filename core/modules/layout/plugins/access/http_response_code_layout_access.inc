<?php
/**
 * @file
 * Class for loading, modifying, and saving a layout access rule (condition).
 */
class HttpResponseCodeLayoutAccess extends LayoutAccessNegatable {
  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $form['http_response_codes'] = array(
      '#type' => 'checkboxes',
      '#title' => t('HTTP response status is'),
      '#options' => array(
        200 => t('Success (200)'),
        403 => t('Access denied (403)'),
        404 => t('Page not found (404)'),
      ),
      '#default_value' => isset($this->settings['http_response_codes']) ? $this->settings['http_response_codes'] : array(),
      '#description' => t('Shows the block on pages with any matching response code. If nothing is selected, the block will be shown on all pages where this layout applies.'),
    );
    $form['negate']['#description'] = t('Hides the block on pages with any matching response code selected from the options above.');
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    $allowed_status_codes = array_filter($this->settings['http_response_codes']);

    $router = menu_get_item();
    $router_path = $router['path'];
    // "Access denied" and "Page not found" get "system/403" and "system/404" as
    // their path respectively. See backdrop_deliver_html_page().
    switch ($router_path) {
      case 'system/403':
        $current_status_code = 403;
        break;
      case 'system/404':
        $current_status_code = 404;
        break;
      default:
        $current_status_code = 200;
        break;
    }

    $access = in_array($current_status_code, $allowed_status_codes);

    return $this->settings['negate'] ? !$access : $access;
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
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
