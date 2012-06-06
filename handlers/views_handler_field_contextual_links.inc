<?php

/**
 * @file
 * Definition of views_handler_field_contextual_links.
 */

/**
 * Provides a handler that adds contextual links.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_contextual_links extends views_handler_field_links {
  function pre_render(&$values) {
    // Add a row plugin css class for the contextual link.
    $class = 'contextual-links-region';
    if (!empty($this->view->style_plugin->options['row_class'])) {
      $this->view->style_plugin->options['row_class'] .= " $class";
    }
    else {
      $this->view->style_plugin->options['row_class'] = $class;
    }
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['fields']['#description'] = t('Fields to be included as contextual links.');
    $form['destination']['#description'] = t('Include a "destination" parameter in the link to return the user to the original view upon completing the contextual action.');
  }

  /**
   * Render the contextual fields.
   */
  function render($values) {
    $links = $this->get_links();
    if (!empty($links)) {
      $build = array(
        '#prefix' => '<div class="contextual-links-wrapper">',
        '#suffix' => '</div>',
        '#theme' => 'links__contextual',
        '#links' => $links,
        '#attributes' => array('class' => array('contextual-links')),
        '#attached' => array(
          'library' => array(array('contextual', 'contextual-links')),
        ),
        '#access' => user_access('access contextual links'),
      );
      return drupal_render($build);
    }
    else {
      return '';
    }
  }
}
