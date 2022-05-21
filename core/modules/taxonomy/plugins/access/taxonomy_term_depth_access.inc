<?php
  /**
   * @file
   * Plugin to provide access control based upon taxonomy term depth.
   */
  class TaxonomyTermDepthLayoutAccess extends LayoutAccess {
    /**
     * Constructor for a Layout access rule.
     */
    function __construct($plugin_name, array $data = array()) {
      parent::__construct($plugin_name, $data);
      $this->settings += array(
        'depth' => array()
      );
    }

    /**
     * {@inheritdoc}
     */
    function form(&$form, &$form_state) {
      parent::form($form, $form_state);

      $form['depth'] = array(
        '#type' => 'number',
        '#title' => t('Depth'),
        '#description' => t('Depth is the level of nesting for a taxonomy term. See example below.'),
        '#default_value' => $this->settings['depth'],
      );

      $code  = '<code>';
      $code .= 'State (1st level, <strong>depth 1</strong>)<br />';
      $code .= '- City (2nd level, <strong>depth 2</strong>)<br />';
      $code .= '&nbsp; - Neighborhood (3rd level, <strong>depth 3</strong>)';
      $code .= '</code>';

      $form['depth_example'] = array(
        '#type' => 'markup',
        '#markup' => $code,
      );
    }

    /**
     * {@inheritdoc}
     */
    function formSubmit($form, &$form_state) {
      parent::formSubmit($form, $form_state);
      $this->settings['depth'] = $form_state['values']['depth'];
    }

    /**
     * {@inheritdoc}
     */
    function summary() {
      if (isset($this->settings['depth'])) {
        return t('Taxonomy term depth: @depth', array('@depth' => $this->settings['depth']));
      }

      return '';
    }

    /**
     * {@inheritdoc}
     */
    function checkAccess() {
      if (!isset($this->contexts['taxonomy_term']->data)) {
        return FALSE;
      };

      $tid = $this->contexts['taxonomy_term']->data->tid;
      $depth = $this->getTermDepth($tid);

      if ($depth == $this->settings['depth']) {
        return TRUE;
      }

      return FALSE;
    }

    /**
     * Gets the depth of a term.
     *
     * @param $tid
     *   The taxonomy term ID
     *
     * @return int
     *   An integer representing how many levels deep your term is nested.
     */
    function getTermDepth($tid) {
      $parent = db_query("SELECT parent FROM {taxonomy_term_hierarchy} WHERE tid = :tid", array(':tid' => $tid))->fetchField();
      if ($parent == 0) {
        return 1;
      }
      else {
        return 1 + $this->getTermDepth($parent);
      }
    }
  }

