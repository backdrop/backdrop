<?php
/**
 * @file
 * Plugin to provide access control based upon the current site language.
 */

class LanguageLayoutAccess extends LayoutAccess {
  /**
   * Constructor for a Layout access rule.
   */
  function __construct($plugin_name, array $data = []) {
    parent::__construct($plugin_name, $data);
    $this->settings += [
      'language' => [],
    ];
  }

  /**
   * {@inheritdoc}
   */
  function summary() {
    $languages = [
      'default' => t('Default site language'),
    ];
    $languages += language_list(FALSE, TRUE);
    $names = [];
    foreach ($this->settings['language'] as $langcode) {
      $names[] = $languages[$langcode];
    }
    if (empty($names)) {
      return t('Site language is any language');
    }
    return format_plural(count($names), 'Site language is "@languages"', 'Site language is one of "@languages"', ['@languages' => implode(', ', $names)]);
  }

  /**
   * {@inheritdoc}
   */
  function checkAccess() {
    global $language;

    // Special case: If 'default' is checked, return TRUE if the default site
    // language matches the current interface language.
    if (in_array('default', $this->settings['language'])) {
      $default = language_default();
      if ($language->langcode == $default->langcode) {
        return TRUE;
      }
    }

    return in_array($language->langcode, $this->settings['language']);
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    $options = [
      'default' => t('Default site language'),
    ];
    $options += language_list(FALSE, TRUE);
    $form['language'] = [
      '#title' => t('Language'),
      '#type' => 'checkboxes',
      '#options' => $options,
      '#description' => t('Pass only if the current site language is one of the selected languages.'),
      '#default_value' => $this->settings['language'],
    ];
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['language'] = array_keys(array_filter($form_state['values']['language']));
  }
}

