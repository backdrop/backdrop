<?php

/**
 * Preprocessor for Date Views filter form.
 */
function template_preprocess_date_views_filter_form(&$vars) {
  $form = $vars['form'];
  $vars['date'] = backdrop_render($form['valuedate']);
  $vars['mindate'] = backdrop_render($form['mindate']);
  $vars['maxdate'] = backdrop_render($form['maxdate']);
  $vars['adjustment'] = backdrop_render($form['valueadjustment']);
  $vars['minadjustment'] = backdrop_render($form['minadjustment']);
  $vars['maxadjustment'] = backdrop_render($form['maxadjustment']);
  $vars['description'] = backdrop_render($form['description']) . backdrop_render($form);
}
