<?php
/**
 * @file
 * Contains the 'node' argument validator plugin.
 */

/**
 * Validate whether an argument is an acceptable node.
 */
class views_plugin_argument_validate_node extends views_plugin_argument_validate {
  function option_definition() {
    $options = parent::option_definition();
    $options['types'] = ['default' => []];
    $options['access'] = ['default' => FALSE, 'bool' => TRUE];
    $options['access_op'] = ['default' => 'view'];
    $options['nid_type'] = ['default' => 'nid'];

    return $options;
  }

  function options_form(&$form, &$form_state) {
    $types = node_type_get_types();
    $options = [];
    foreach ($types as $type => $info) {
      $options[$type] = check_plain(t($info->name));
    }

    $form['types'] = [
      '#type' => 'checkboxes',
      '#title' => t('Content types'),
      '#options' => $options,
      '#default_value' => $this->options['types'],
      '#description' => t('Choose one or more content types to validate with.'),
    ];

    $form['access'] = [
      '#type' => 'checkbox',
      '#title' => t('Validate user has access to the content'),
      '#default_value' => $this->options['access'],
    ];
    $form['access_op'] = [
      '#type' => 'radios',
      '#title' => t('Access operation to check'),
      '#options' => ['view' => t('View'), 'update' => t('Edit'), 'delete' => t('Delete')],
      '#default_value' => $this->options['access_op'],
      '#states' => [
        'visible' => [
          ':input[name="options[validate][options][node][access]"]' => ['checked' => TRUE],
        ],
      ],
    ];

    $form['nid_type'] = [
      '#type' => 'select',
      '#title' => t('Filter value format'),
      '#options' => [
        'nid' => t('Node ID'),
        'nids' => t('Node IDs separated by , or +'),
      ],
      '#default_value' => $this->options['nid_type'],
    ];
  }

  function options_submit(&$form, &$form_state, &$options = []) {
    // filter trash out of the options so we don't store giant unnecessary arrays
    $options['types'] = array_filter($options['types']);
  }

  function convert_options(&$options) {
    if (!isset($options['types']) && !empty($this->argument->options['validate_argument_node_type'])) {
      $options['types'] = isset($this->argument->options['validate_argument_node_type']) ? $this->argument->options['validate_argument_node_type'] : [];
      $options['access'] = !empty($this->argument->options['validate_argument_node_access']);
      $options['access_op'] = isset($this->argument->options['validate_argument_node_access_op']) ? $this->argument->options['validate_argument_node_access_op'] : 'view';
      $options['nid_type'] = isset($this->argument->options['validate_argument_nid_type']) ? $this->argument->options['validate_argument_nid_type'] : [];
    }
  }

  function validate_argument($argument) {
    $types = $this->options['types'];

    switch ($this->options['nid_type']) {
      case 'nid':
        if (!is_numeric($argument)) {
          return FALSE;
        }
        $node = node_load($argument);
        if (!$node) {
          return FALSE;
        }

        if (!empty($this->options['access'])) {
          if (!node_access($this->options['access_op'], $node)) {
            return FALSE;
          }
        }

        // Save the title() handlers some work.
        $this->argument->validated_title = check_plain($node->title);

        if (empty($types)) {
          return TRUE;
        }

        return isset($types[$node->type]);
      break;
      case 'nids':
        $nids = new stdClass();
        $nids->value = [$argument];
        $nids = views_break_phrase($argument, $nids);
        if ($nids->value == [-1]) {
          return FALSE;
        }

        $test = backdrop_map_assoc($nids->value);
        $titles = [];

        $result = db_query("SELECT * FROM {node} WHERE nid IN (:nids)", [':nids' =>  $nids->value]);
        foreach ($result as $node) {
          if ($types && empty($types[$node->type])) {
            return FALSE;
          }

          if (!empty($this->options['access'])) {
            if (!node_access($this->options['access_op'], $node)) {
              return FALSE;
            }
          }

          $titles[] = check_plain($node->title);
          unset($test[$node->nid]);
        }

        $this->argument->validated_title = implode($nids->operator == 'or' ? ' + ' : ', ', $titles);
        // If this is not empty, we did not find a nid.
        return empty($test);
    }
  }
}
