<?php
/**
 * @file
 * Plugin to provide access control/visibility based on date and time.
 */
class DateTimeLayoutAccess extends LayoutAccessNegatable {

  /**
   * {@inheritdoc}
   */
  function summary() {
    $from = format_date(strtotime($this->settings['start_time']),'medium');
    $to = format_date(strtotime($this->settings['end_time']),'medium');
    $range = t('from ') . $from . t(' to ') . $to; 
    if ($this->settings['negate']) {
      return t('Date is NOT: @range', array('@range'  => $range));
    }
    else {
      return t('Date is: @range', array('@range'  => $range));
    }
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    $timezone = '';
    if ($this->settings['timezone'] == 'user') {
      $timezone = ' ' . backdrop_get_user_timezone();
    }
    $from = strtotime($this->settings['start_time'] . $timezone);
    $to = strtotime($this->settings['end_time'] . $timezone);
    $now = time();
    $in_range = (($from < $now) && ($to > $now));

    return $this->settings['negate'] ? !$in_range : $in_range;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    $now = format_date(time(),'medium');
    $format = 'Y-m-d H:i';
     
    $form['start_time'] = array(
      '#type' => 'date_popup',
      '#date_format' => $format,
      '#title' => t('Start date'),
      '#date_label_position' => 'invisible',
      '#date_year_range' => '-0:+3',
      '#default_value' => isset($this->settings['start_time']) ? $this->settings['start_time'] : '',
    );
    $form['end_time'] = array(
      '#type' => 'date_popup',
      '#date_format' => $format,
      '#title' => t('End date'),
      '#date_label_position' => 'invisible',
      '#date_year_range' => '-0:+3',
      '#default_value' => isset($this->settings['end_time']) ? $this->settings['end_time'] : '',
    );
    $form['timezone'] = array(
      '#type' => 'radios',
      '#title' => t('Timezone'),
      '#description' => t("If set to 'User', access will be determined based on the time in the user's timezone, not the site time. Current site time is !now", array('!now' => $now)),
      '#options' => array(
        'site' => t('Site'),
        'user' => t('User'),
      ),
      '#default_value' => isset($this->settings['timezone']) ? $this->settings['timezone'] : 'site',
    );
  }
}
