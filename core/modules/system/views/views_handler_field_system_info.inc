<?php
/**
 * @file
 * Definition of views_handler_field_system_info.
 */

/**
 * A handler to display module and theme properties stored in the database.
 *
 * Provides access to several properties stored as a serialized array in the
 * info column of the system table.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_system_info extends views_handler_field {
  /**
   * {@inheritdoc}
   */
  public function option_definition() {
    $options = parent::option_definition();
    $options['info_property'] = ['default' => ''];
    $options['custom_label'] = ['default' => 'default'];
    $options['other'] = ['default' => ''];
    $options['list_separator'] = ['default' => ', '];
    $options['multi_type'] = ['default' => 'separator'];
    $options['separator'] = ['default' => ', '];
    $options['dependencies_display'] = ['default' => 'project_names'];
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['custom_label']['#type'] = 'radios';
    $form['custom_label']['#options'] = [
      'default' => t('Use human-readable label shown in the "Property" select field below'),
      'custom' => t('Custom label'),
      'none' => t('No label'),
    ];
    unset($form['custom_label']['#description']);

    $form['custom_label']['#default_value'] = $this->options['custom_label'];
    $form['label']['#states'] = [
      'visible' => [
        ':input[name="options[custom_label]"]' => ['value' => 'custom'],
      ],
    ];
    $form['label']['#title'] = t('Custom label');

    $properties = [];
    foreach ($this->_getPropertyOptions() as $key => $label) {
      $properties[$key] = $label . ' (machine name: '. $key .')';
    }
    $properties['other'] = t('Other');

    $form['info_property'] = [
      '#type' => 'select',
      '#title' => t('Property'),
      '#options' => $properties,
      '#default_value' => $this->options['info_property'],
    ];

    $form['other'] = [
      '#type' => 'textfield',
      '#title' => t('Property name'),
      '#description' => t('The machine name of the property found in the .info file. If the property doesn\'t exist, the output will be empty.'),
      '#default_value' => $this->options['other'],
      '#states' => [
        'visible' => [
          ':input[name="options[info_property]"]' => ['value' => 'other'],
        ],
      ],
    ];

    $form['dependencies_display'] = [
      '#type' => 'radios',
      '#title' => t('Dependency display'),
      '#description' => t('Select how to display dependencies.'),
      '#options' => [
        'machine_names' => t('As machine names'),
        'project_names' => t('As project names'),
      ],
      '#default_value' => $this->options['dependencies_display'],
      '#states' => [
        'visible' => [
          ':input[name="options[info_property]"]' => ['value' => 'dependencies'],
        ],
      ],
    ];

    $form['multi_type'] = [
      '#type' => 'radios',
      '#title' => t('Display type'),
      '#options' => [
        'ul' => t('Unordered list'),
        'ol' => t('Ordered list'),
        'separator' => t('Simple separator'),
      ],
      '#states' => [
        'visible' => [
          ':input[name="options[info_property]"]' => [
            ['value' => 'tags'],
            ['value' => 'dependencies'],
            ['value' => 'scripts'],
          ],
        ],
      ],
      '#default_value' => $this->options['multi_type'],
    ];

    $form['separator'] = [
      '#type' => 'textfield',
      '#title' => t('Separator'),
      '#default_value' => $this->options['separator'],
      '#states' => [
        'visible' => [
          ':input[name="options[info_property]"]' => [
            ['value' => 'tags'],
            ['value' => 'dependencies'],
          ],
          ':input[name="options[multi_type]"]' => ['value' => 'separator'],
        ],
      ],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function render($values) {
    $value = $this->get_value($values);
    $info_array = unserialize($value);
    $info_property = $this->options['info_property'];
    if ($info_property == 'other') {
      $info_property = $this->options['other'];
    }
    if (empty($info_array[$info_property])) {
      return '';
    }

    switch ($info_property) {
      case 'tags':
      case 'dependencies':
      case 'scripts':
        $items = $info_array[$info_property];

        if ($info_property == 'dependencies' && $this->options['dependencies_display'] == 'project_names' && ($info_array['type'] == 'module' || $info_array['type'] == 'profile')) {
          // Display human names for projects.
          $modules = system_rebuild_module_data();
          $module_labels = [];
          foreach ($items as $item) {
            $module_labels[] = check_plain($modules[$item]->info['name']);
          }
          $items = $module_labels;
        }

        $multi_type = $this->options['multi_type'];
        $output = '';
        switch($multi_type) {
          case 'separator':
            $separator = $this->options['separator'];
            $output = implode($separator, $items);
            break;

          default:
            $output = theme('item_list', ['items' => $items,
                                               'type' => $multi_type]);
            break;
        }
        return $output;

      default:
        $output = $info_array[$info_property];
        if (is_array($output)) {
          $output = theme('item_list', ['items' => $this->_array2list($output)]);
        }
        return $output;
    }
  }

  /**
   * Get this field's label based on the selected information.
   */
  public function label() {
    if ($this->options['custom_label'] === 'default') {
      $property_options = $this->_getPropertyOptions();
      $label = !empty($property_options[$this->options['info_property']]) ? $property_options[$this->options['info_property']] : '';
      return $label;
    }
    elseif ($this->options['custom_label'] === 'custom' && isset($this->options['label'])) {
      return $this->options['label'];
    }
    return '';
  }

  /**
   * Helper function to return the options array.
   */
  protected function _getPropertyOptions() {
    return [
      'name' => t('Name'),
      'description' => t('Description'),
      'package' => t('Package'),
      'configure' => t('Configuration path'),
      'tags' => t('Tags'),
      'version' => t('Version'),
      'backdrop' => t('Backdrop version'),
      'dependencies' => t('Dependencies'),
      'required' => t('Required'),
      'base theme' => t('Base theme'),
      'scripts' => t('Scripts'),
      'stylesheets' => t('Stylesheets'),
    ];
  }

  /**
   * Helper to convert multilevel array to multilevel item list.
   */
  protected function _array2list($array) {
    $return = [];
    foreach ($array as $key => $item) {
      if (is_array($item)) {
        $return[] = [
          'data' => $key,
          'children' => $this->_array2list($item),
        ];
      }
      else {
        $return[] = [
          'data' => $key . ' = ' . $item,
        ];
      }
    }
    return $return;
  }
}
