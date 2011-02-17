<?php
// $Id: $

class views_ui extends ctools_export_ui {

  function init($plugin) {
    // We modify the plugin info here so that we take the defaults and
    // twiddle, rather than completely override them.

    // Reset the edit path to match what we're really using.
    $plugin['menu']['items']['edit']['path'] = 'view/%ctools_export_ui/edit';
    $plugin['menu']['items']['clone']['path'] = 'view/%ctools_export_ui/clone';
    $plugin['menu']['items']['export']['path'] = 'view/%ctools_export_ui/export';
    $plugin['menu']['items']['enable']['path'] = 'view/%ctools_export_ui/enable';
    $plugin['menu']['items']['disable']['path'] = 'view/%ctools_export_ui/disable';
    $plugin['menu']['items']['delete']['path'] = 'view/%ctools_export_ui/delete';
    $plugin['menu']['items']['revert']['path'] = 'view/%ctools_export_ui/revert';

    return parent::init($plugin);
  }

  function hook_menu(&$items) {
    // We are using our own 'edit' still, rather than having edit on this
    // object (maybe in the future) so unset the edit callbacks:

    // We leave these to make sure the operations still exist in the plugin so
    // that the path finder.
    unset($this->plugin['menu']['items']['edit']);
    unset($this->plugin['menu']['items']['add']);
    unset($this->plugin['menu']['items']['import']);
    unset($this->plugin['menu']['items']['edit callback']);

    parent::hook_menu($items);
  }

  function list_form(&$form, &$form_state) {
    $row_class = 'container-inline';
    if (!variable_get('views_ui_show_listing_filters', FALSE)) {
      $row_class .= " element-invisible";
    }

    views_include('admin');

    parent::list_form($form, $form_state);

    $form['top row']['#prefix'] = '<div class="' . $row_class . ' ctools-export-ui-row ctools-export-ui-top-row clearfix">';
    $form['bottom row']['#prefix'] = '<div class="' . $row_class . ' ctools-export-ui-row ctools-export-ui-bottom-row clearfix">';

    $form['bottom row']['sort']['#title'] = '';

    $form['top row']['disabled']['#title'] = '';
    $form['top row']['disabled']['#options']['all'] = t('All status');

    $form['top row']['storage']['#title'] = '';
    $form['top row']['storage']['#options'] = array(
      'all' => t('All storage'),
      t('Normal') => t('In database'),
      t('Default') => t('In code'),
      t('Overridden') => t('Database overriding code'),
    );

    $form['top row']['search']['#weight'] = -10;
    $form['top row']['search']['#size'] = 15;

    $this->bases = array();
    foreach (views_fetch_base_tables() as $table => $info) {
      $this->bases[$table] = $info['title'];
    }

    $form['top row']['base'] = array(
      '#type' => 'select',
      '#options' => array_merge(array('all' => t('All types')), $this->bases),
      '#default_value' => 'all',
      '#weight' => -1,
    );

    $tags = array();
    if (isset($form_state['views'])) {
      foreach ($form_state['views'] as $name => $view) {
        if (!empty($view->tag)) {
          $tags[$view->tag] = $view->tag;
        }
      }
    }

    asort($tags);

    $form['top row']['tag'] = array(
      '#type' => 'select',
      '#title' => t('Filter'),
      '#options' => array_merge(array('all' => t('All tags')), array('none' => t('No tags')), $tags),
      '#default_value' => 'all',
      '#weight' => -9,
    );

    $displays = array();
    foreach (views_fetch_plugin_data('display') as $id => $info) {
      if (!empty($info['admin'])) {
        $displays[$id] = $info['admin'];
      }
    }

    asort($displays);

    $form['top row']['display'] = array(
      '#type' => 'select',
      '#options' => array_merge(array('all' => t('All displays')), $displays),
      '#default_value' => 'all',
      '#weight' => -1,
    );

  }

  function list_filter($form_state, $view) {
    if ($form_state['values']['base'] != 'all' && $form_state['values']['base'] != $view->base_table) {
      return TRUE;
    }

    return parent::list_filter($form_state, $view);
  }

  function list_sort_options() {
    return array(
      'disabled' => t('Enabled, name'),
      'name' => t('Name'),
      'path' => t('Path'),
      'tag' => t('Tag'),
      'storage' => t('Storage'),
    );
  }


  function list_build_row($view, &$form_state, $operations) {
    if (!empty($view->human_name)) {
      $title = $view->human_name;
    }
    else {
      $title = $view->get_title();
      if (empty($title)) {
        $title = $view->name;
      }
    }

    $paths = _views_ui_get_paths($view);
    $paths = implode(", ", $paths);

    $base = !empty($this->bases[$view->base_table]) ? $this->bases[$view->base_table] : t('Broken');

    $info = theme('views_ui_view_info', array('view' => $view, 'base' => $base));

    // Set up sorting
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$view->name] = strtolower(empty($view->disabled) . $title);
        break;
      case 'name':
        $this->sorts[$view->name] = strtolower($title);
        break;
      case 'path':
        $this->sorts[$view->name] = strtolower($paths);
        break;
      case 'tag':
        $this->sorts[$view->name] = strtolower($view->tag);
        break;
      case 'storage':
        $this->sorts[$view->name] = strtolower($view->type . $title);
        break;
    }

    $this->rows[$view->name] = array(
      'data' => array(
        array('data' => $info, 'class' => array('views-ui-name')),
        array('data' => check_plain($view->description), 'class' => array('views-ui-description')),
        array('data' => check_plain($view->tag), 'class' => array('views-ui-tag')),
        array('data' => $paths, 'class' => array('views-ui-path')),
        array('data' => theme('links', array('links' => $operations, 'attributes' => array('class' => array('links', 'inline')))), 'class' => array('views-ui-operations')),
      ),
      'title' => t('Machine name: ') . check_plain($view->name),
      'class' => array(!empty($view->disabled) ? 'ctools-export-ui-disabled' : 'ctools-export-ui-enabled'),
    );
  }

  function list_render(&$form_state) {
    views_include('admin');
    views_ui_add_admin_css();
    if (empty($_REQUEST['js'])) {
      views_ui_check_advanced_help();
    }
    drupal_add_library('system', 'jquery.bbq');
    views_add_js('views-list');

    $this->active = $form_state['values']['order'];
    $this->order = $form_state['values']['sort'];

    $query    = tablesort_get_query_parameters();

    $header = array(
      $this->tablesort_link(t('View name'), 'name', 'views-ui-name'),
      array('data' => t('Description'), 'class' => array('views-ui-description')),
      $this->tablesort_link(t('Tag'), 'tag', 'views-ui-tag'),
      $this->tablesort_link(t('Path'), 'path', 'views-ui-path'),
      array('data' => t('Operations'), 'class' => array('views-ui-operations')),
    );

    $table = array(
      'header' => $header,
      'rows' => $this->rows,
      'attributes' => array('id' => 'ctools-export-ui-list-items'),
    );
    return theme('table', $table);
  }

  function tablesort_link($label, $field, $class) {
    $title = t('sort by @s', array('@s' => $label));
    $initial = 'asc';

    if ($this->active == $field) {
      $initial = ($this->order == 'asc') ? 'desc' : 'asc';
      $label .= theme('tablesort_indicator', array('style' => $initial));
    }

    $query['order'] = $field;
    $query['sort'] = $initial;
    $link_options = array(
      'html' => TRUE,
      'attributes' => array('title' => $title),
      'query' => $query,
    );
    $link = l($label, $_GET['q'], $link_options);
    if ($this->active == $field) {
      $class .= ' active';
    }

    return array('data' => $link, 'class' => $class);
  }

}

