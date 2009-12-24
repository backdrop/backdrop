<?php
// $Id$
/**
 * Allow taxonomy term ID(s) as argument
 */
class views_handler_argument_term_node_tid extends views_handler_argument_many_to_one {
  function option_definition() {
    $options = parent::option_definition();
    $options['set_breadcrumb'] = array('default' => FALSE);
    $options['use_synonym_for_summary_links'] = array('default' => FALSE);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['set_breadcrumb'] = array(
      '#type' => 'checkbox',
      '#title' => t("Set the breadcrumb for the term parents"),
      '#description' => t('If selected, the breadcrumb trail will include all parent terms, each one linking to this view. Note that this only works if just one term was received.'),
      '#default_value' => !empty($this->options['set_breadcrumb']),
    );
    $form['use_synonym_for_summary_links'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use term synonyms for summary links'),
      '#description' => t('If selected, summary views will use a synonym defined for each term when creating the URL to link to that term. Note that this only works reliably if terms have at most one synonym.'),
      '#default_value' => !empty($this->options['use_synonym_for_summary_links']),
    );
  }

  function set_breadcrumb(&$breadcrumb) {
    if (empty($this->options['set_breadcrumb']) || !is_numeric($this->argument)) {
      return;
    }

    return views_taxonomy_set_breadcrumb($breadcrumb, $this);
  }

  function title_query() {
    $titles = array();

    $result = db_query("SELECT name FROM {taxonomy_term_data} WHERE tid IN (:tids)", array(':tids' => $this->value));
    foreach ($result as $term) {
      $titles[] = check_plain($term->name);
    }
    return $titles;
  }

  function summary_argument($data) {
    if (!empty($this->options['use_synonym_for_summary_links'])) {
      $synonym = db_result(db_query_range("SELECT name FROM {term_synonym} WHERE tid = %d", $data->{$this->base_alias}, 0, 1));
      if (!empty($synonym)) {
        return $synonym;
      }
    }
    return $data->{$this->base_alias};
  }

}

