<?php
/**
 * @file
 * Dashboard block displaying informtion about taxonomy, including:
 * -
 */
class DashboardTaxonomyBlock extends Block {
  /**
   * {@inheritdoc}
   */
  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $this->settings += array(
      'vocabularies' => array(),
    );
  }

  /**
   * {@inheritdoc}
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Taxonomy');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminTitle() {
    if (!empty($this->settings['admin_label'])) {
      return check_plain($this->settings['admin_label']);
    }

    return t('Manage taxonomy');
  }

  /**
   * {@inheritdoc}
   */
  function getAdminPreview() {
    if (!empty($this->settings['admin_description'])) {
      return filter_xss($this->settings['admin_description']);
    }

    return t('Links to administer taxonomy vocabularies.');
  }

  /**
   * {@inheritdoc}
   */
  function getContent() {
    if (!module_exists('taxonomy')) {
      return;
    }

    $allowed_vocabularies = !empty($this->settings['vocabularies']) ? $this->settings['vocabularies'] : FALSE;
    // Passing FALSE to taxonomy_vocabulary_load_multiple() loads all
    // vocabulaies.
    $vocabularies = taxonomy_vocabulary_load_multiple($allowed_vocabularies);

    $rows = array();
    // Flag that determines whether the block should be rendered or not.
    $no_access = TRUE;
    foreach ($vocabularies as $vocabulary) {
      $term_count = db_query("SELECT count(*) FROM {taxonomy_term_data} WHERE vocabulary = :name", array(':name' => $vocabulary->machine_name))->fetchField();
      $terms = format_plural($term_count, '1 term', '@count terms');

      module_load_include('inc', 'taxonomy', 'taxonomy.admin');
      $operations = _vocabulary_get_operations($vocabulary);
      foreach ($operations as $operation => $link) {
        // We only need a specific set of operations in the Taxonomy block.
        if (!in_array($operation, array('list', 'add', 'configure'))) {
          // Remove the rest.
          unset($operations[$operation]);
        }
        // Add the current page as destination for the remaining operations.
        else {
          $operations[$operation]['query']['destination'] = current_path();
        }
      }

      // If even one operation is allowed to be performed on any vocabulary,
      // render the block.
      if (!empty($operations)) {
        $no_access = FALSE;
      }

      $operations = array(
        '#type' => 'operations',
        '#links' => $operations,
      );

      $rows[] = array(
        'data' => array(
          check_plain(t($vocabulary->name)),
          $terms,
          backdrop_render($operations),
        ),
      );
    }

    // If there are existing vocabularies, but the user has no access to perform
    // any operation to any of them, hide the block completely.
    if ($no_access) {
      return array();
    }

    $header = array(
      array('data' => t('Vocabulary')),
      array('data' => t('Terms')),
      array('data' => t('Operations')),
    );

    $panel = array(
      '#theme' => 'dashboard_panel__taxonomy',
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('There are no vocabularies to display.'),
    );
    if (user_access('administer taxonomy')) {
      $panel['link'] = array(
        '#theme' => 'link',
        '#path' => 'admin/structure/taxonomy',
        '#text' => t('Manage taxonomy'),
      );
    }

    return $panel;
  }

  /**
   * {@inheritdoc}
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    if (!module_exists('taxonomy')) {
      return $form;
    }

    $vocabularies = taxonomy_get_vocabularies();
    $options = array();
    foreach ($vocabularies as $machine => $vocabulary) {
      $options[$machine] = check_plain(t($vocabulary->name));
    }

    $form['vocabularies'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display the following Vocabularies'),
      '#multiple' => TRUE,
      '#options' => $options,
      // An empty array indicates enabled on all.
      '#default_value' => empty($this->settings['vocabularies']) ? array_keys($options) : $this->settings['vocabularies'],
    );
  }

  /**
   * {@inheritdoc}
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    // If all types are checked, save nothing, indicating all types are allowed.
    $checked_vocabularies = array_filter($form_state['values']['vocabularies']);
    if (count($form['vocabularies']['#options']) === count($checked_vocabularies)) {
      $this->settings['vocabularies'] = array();
    }
    else {
      $this->settings['vocabularies'] = array_values($checked_vocabularies);
    }
  }
}
