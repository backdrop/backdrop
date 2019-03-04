<?php
/**
 * @file
 * "Update" dashboard block.
 */

class DashboardUpdateBlock extends Block {

  /**
   *  Sets block subject on block view.
   */
  function getTitle() {
    return !empty($this->settings['title']) ? check_plain($this->settings['title']) : t('Available Updates');
  }

  /**
   * 'Admin title' callback for the content type.
   */
  function getAdminTitle() {
    return t('Available updates');
  }

  /**
   * 'Admin info' callback for the content type.
   */
  function getAdminPreview() {
    return t('Displays available updates for core, modules, themes, and layouts.');
  }

  /**
   * Run-time rendering of the body of the block.
   */
  function getContent() {
    if (!module_exists('update')) {
      return;
    }

    $items = array();
    $available = update_get_available();
    module_load_include('inc', 'update', 'update.compare');
    $data = update_calculate_project_data($available);


    foreach ($data as $key => $project) {
      $type = $project['project_type'];
      if ($this->settings['display'][$type] === $type) {

        $name = $project['info']['name'];
        if ($project['name'] == 'backdrop') {
          $name = t('Backdrop');
        }

        $new = t('version @new available', array('@new' => $project['recommended']));
        if (user_access('access_site_reports')) {
          $new = l($new, 'admin/reports/updates');
        }

        $item = $name . ' ' . $type . ' (' . $new . ')';

        if (module_exists('installer') && user_access('administer software updates')) {
          $options = array('attributes' => array('class' => array('hi')));
          $item .= ' - ' . l(t('Update'), 'admin/modules/update', $options);
        }

        $items[] = $item;
      }
    }


    return theme('item_list', array('items' => $items));
  }

  /**
   * 'Edit form' callback for the content type.
   */
  function form(&$form, &$form_state) {
    parent::form($form, $form_state);

    if (!module_exists('update')) {
      return $form;
    }

    $form['display'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Include Vocabularies'),
      '#multiple' => TRUE,
      '#options' => array(
        'core' => t('Backdrop core'),
        'module' => t('Modules'),
        'theme' => t('Themes'),
        'layout' => t('Layouts'),
      ),
      '#default_value' => $this->settings['display'],
    );
  }

  /**
   * 'Edit form' submit callback for the content type.
   */
  function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);

    $this->settings['display'] = $form_state['values']['display'];
  }
}
