<?php

  /**
   * @file
   *
   * "Taxonomy" content type. It shows users with permissions statistics
   * and links to manage terms in vocabularies on the site.
   *
   */

class DashboardTaxonomyBlock extends Block {


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
    return t('Dashboard manage taxonomy');
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

    $vids = (isset($this->settings['vids'])) ? $this->settings['vids'] : array();

    $vocabs = taxonomy_get_vocabularies();
      $options = array('destination' => 'admin/dashboard');
      $header = array(
        array('data' => t('Vocabulary')),
        array(
          'data' => t('Operations'),
        ),
      );
      $rows = array();

    if (!empty($vocabs)) {
        foreach ($vocabs as $vocab) {
          if ((in_array($vocab->machine_name, $vids)) || !array_key_exists('vids', $this->settings)) {
            $term_count = db_query("SELECT count(*) FROM {taxonomy_term_data} WHERE vocabulary = :vid", array(':vid' => $vocab->machine_name))->fetchField();
            if (user_access('administer taxonomy') || user_access('edit terms in ' . $vocab->machine_name)) {
              $terms = format_plural($term_count, '1 categories', '@count categories');
              $links['configure'] = array(
                'title' => t('Configure'),
                'href' => 'admin/structure/taxonomy/' . $vocab->machine_name . '/edit',
                'query' => $options,
              );
              $links['manage'] = array(
                'title' => t('Manage categories'),
                'href' => 'admin/structure/taxonomy/' . $vocab->machine_name,
                'query' => $options,
              );
              $links['add'] = array(
                'title' => t('Add new category'),
                'href' => 'admin/structure/taxonomy/' . $vocab->machine_name . '/add',
                'query' => $options,
              );
              $operations = array(
                '#type' => 'dropbutton',
                '#links' => $links,
              );
              $rows[] = array(
                'data' => array(
                  t($vocab->name . ': ' . $terms),
                  backdrop_render($operations),
                ),
              );
            }
          }
        }
      }

    if (empty($rows)) {
        $rows[] = array(
          array(
            'data' => t('There are no vocabularies to display.'),
            'colspan' => 4,
          ),
        );
      }

    $link = '';
      if (user_access('administer taxonomy')) {
        $link = l(t('Taxonomy administration'), 'admin/structure/taxonomy');
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

    $vocabs = taxonomy_get_vocabularies();
    if (!empty($vocabs)) {
      $vocab_options = array();
      $vocab_defaults = array();
      foreach ($vocabs as $vid => $vocab) {
        $vocab_options[$vid] = $vocab->name;
        $vocab_defaults[$vid] = $vid;
      }

      if (isset($this->settings['vids'])) {
          $vocab_defaults = $this->settings['vids'];
      }

      $form['vids'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Include Vocabularies'),
        '#multiple' => TRUE,
        '#options' => $vocab_options,
        '#default_value' => $vocab_defaults,
      );
    }
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    foreach (array_keys($form_state['values']['vids']) as $key) {
      $this->settings['vids'][$key] = $form_state['values']['vids'][$key];
    }
  }
}
