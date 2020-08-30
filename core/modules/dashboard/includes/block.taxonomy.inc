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

    $vocabularies = taxonomy_get_vocabularies();
    $current_path = current_path();
    $options = array('destination' => $current_path);
    $header = array(
      array('data' => t('Vocabulary')),
      array('data' => t('Terms')),
      array('data' => t('Operations')),
    );
    $rows = array();
    $no_access = FALSE;

    foreach ($vocabularies as $machine => $vocabulary) {
      if (in_array($machine, $this->settings['vocabularies']) || empty($this->settings['vocabularies'])) {
        $term_count = db_query("SELECT count(*) FROM {taxonomy_term_data} WHERE vocabulary = :name", array(':name' => $vocabulary->machine_name))->fetchField();
        if (user_access('administer taxonomy') || user_access('edit terms in ' . $vocabulary->machine_name)) {
          $terms = format_plural($term_count, '1 term', '@count terms');
          $links['list'] = array(
            'title' => t('List terms'),
            'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name,
            'query' => $options,
          );
          $links['add'] = array(
            'title' => t('Add new term'),
            'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/add',
            'query' => $options,
          );
          if (user_access('administer taxonomy')) {
            $links['configure'] = array(
              'title' => t('Configure'),
              'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/edit',
              'query' => $options,
            );
          }
          $operations = array(
            '#type' => 'dropbutton',
            '#links' => $links,
          );
          $rows[] = array(
            'data' => array(
              check_plain(t($vocabulary->name)),
              $terms,
              backdrop_render($operations),
            ),
          );
        }
        else {
          $no_access = TRUE;
        }
      }
    }

    // If there are existing vocabularies, but user has no access to any
    // of them, hide the block completely.
    if (empty($rows)) {
      if ($no_access) {
        return array();
      }
      $rows[] = array(
        array(
          'data' => t('There are no vocabularies to display.'),
          'colspan' => 4,
        ),
      );
    }

    $panel = array(
      '#theme' => 'dashboard_panel__taxonomy',
    );
    $panel['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
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
