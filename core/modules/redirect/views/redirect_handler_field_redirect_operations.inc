<?php
/**
 * @file
 * Redirect field handler for redirect operations.
 */

class redirect_handler_field_redirect_operations extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['rid'] = 'rid';
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['edit_text'] = ['default' => '', 'translatable' => TRUE];
    $options['delete_text'] = ['default' => '', 'translatable' => TRUE];
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['edit_text'] = [
      '#type' => 'textfield',
      '#title' => t('Text to display for edit links'),
      '#default_value' => $this->options['edit_text'],
    ];
    $form['delete_text'] = [
      '#type' => 'textfield',
      '#title' => t('Text to display for delete links'),
      '#default_value' => $this->options['delete_text'],
    ];
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $rid = $values->{$this->aliases['rid']};
    $redirect = redirect_load($rid);
    $destination = backdrop_get_destination();

    $operations = [];
    if (redirect_access('update', $redirect)) {
      $operations['edit'] = [
        'title' => !empty($this->options['edit_text']) ? $this->options['edit_text'] : t('Edit'),
        'href' => 'admin/config/urls/redirect/edit/' . $rid,
        'query' => $destination,
      ];
    }
    if (redirect_access('delete', $redirect)) {
      $operations['delete'] = [
        'title' => !empty($this->options['delete_text']) ? $this->options['delete_text'] : t('Delete'),
        'href' => 'admin/config/urls/redirect/delete/' . $rid,
        'query' => $destination,
      ];
    }

    if (!empty($operations)) {
      return theme('links', ['links' => $operations, 'attributes' => ['class' => ['links', 'inline', 'nowrap']]]);
    }
    else {
      return '';
    }
  }
}
