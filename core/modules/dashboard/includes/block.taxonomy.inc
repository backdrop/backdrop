<?php
/**
 * @file
 * Dashboard block displaying informtion about taxonomy, including:
 * -
 */
class DashboardTaxonomyBlock extends Block {

  function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    // Set defaults.
    $this->settings += array(
      'vocabularies' => array_keys(taxonomy_get_vocabularies()),
    );
  }

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Categories');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Manage taxonomy');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Links to administer taxonomy.');
  }

  /**
   * Run-time rendering of the body of the block.
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
      array('data' => t('Operations')),
    );
    $rows = array();
    $no_access = FALSE;

    foreach ($vocabularies as $vocabulary) {
      if (in_array($vocabulary->machine_name, $this->settings['vocabularies'])) {
        $term_count = db_query("SELECT count(*) FROM {taxonomy_term_data} WHERE vocabulary = :name", array(':name' => $vocabulary->machine_name))->fetchField();
        if (user_access('administer taxonomy') || user_access('edit terms in ' . $vocabulary->machine_name)) {
          $terms = format_plural($term_count, '1 term', '@count terms');
          $links['configure'] = array(
            'title' => t('Configure'),
            'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/edit',
            'query' => $options,
          );
          $links['manage'] = array(
            'title' => t('Manage categories'),
            'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name,
            'query' => $options,
          );
          $links['add'] = array(
            'title' => t('Add new category'),
            'href' => 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/add',
            'query' => $options,
          );
          $operations = array(
            '#type' => 'dropbutton',
            '#links' => $links,
          );
          $rows[] = array(
            'data' => array(
              t($vocabulary->name) . ': ' . $terms,
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

    $link = '';
    if (user_access('administer taxonomy')) {
      $link = l(t('Manage taxonomy'), 'admin/structure/taxonomy');
    }

    return theme('dashboard_admin_table', array('header' => $header, 'rows' => $rows, 'link' => $link));
  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    if (!module_exists('taxonomy')) {
      return $form;
    }

    $vocabularies = taxonomy_get_vocabularies();
    $options = array();
    foreach ($vocabularies as $machine_name => $vocabulary) {
      $options[$machine_name] = $vocabulary->name;
    }

    $form['vocabularies'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display the following Vocabularies'),
      '#multiple' => TRUE,
      '#options' => $options,
      '#default_value' => $this->settings['vocabularies'],
    );
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    foreach (array_keys($form_state['values']['vocabularies']) as $key) {
      $this->settings['vocabularies'][$key] = $form_state['values']['vocabularies'][$key];
    }
  }
}
