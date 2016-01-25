<?php
/**
 * @file
 * Class providing forms and settings for menu block styles.
 */
class MenuStyleSlim extends MenuStyle {

  /**
   * Specifies the settings form for configuring the style.
   */
  function form($settings) {
    //parent::form($form, $form_state);
    $config = config('system.menu_block_style_simple');

    $form['resize_width'] = array(
      '#type' => 'textfield',
      '#title' => t('Resize width'),
      '#default_value' => isset($settings['resize_width']) ? $settings['resize_width'] : 800,
      '#description' => t('Minimum width to collapse the navigation menu. Navigation menu will be collapsed when document width is below this size or equal to it.'),
    );
    $form['initially_visible'] = array(
      '#type' => 'checkbox',
      '#title' => t('Initially visible'),
      '#default_value' => isset($settings['initially_visible']) ? $settings['initially_visible'] : FALSE,
      '#description' => t('Make main navigation menu initially visible on mobile devices without the need to click on expand/collapse icon.'),
    );
    $form['collapser_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Collapsed menu title'),
      '#default_value' => isset($settings['collapser_title']) ? $settings['collapser_title'] : 'Menu',
      '#description' => t('Collapsed menu title.'),
    );
    $form['anim_speed'] = array(
      '#type' => 'textfield',
      '#title' => t('Hover intent'),
      '#default_value' => isset($settings['anim_speed']) ? $settings['anim_speed'] : 'medium', // accepts slow, medium, fast or miliseconds
      '#description' => t('Speed of the submenu expand and collapse animation.'),
    );
    $form['easing_effect'] = array(
      '#type' => 'textfield',
      '#title' => t('Easing effect'),
      '#default_value' => isset($settings['easing_effect']) ? $settings['easing_effect'] : 'null', // any easing effect.
      '#description' => t('Easing effect that will be used when expanding and collapsing menu and submenus.'),
    );
    $form['indent_children'] = array(
      '#type' => 'checkbox',
      '#title' => t('Indent children'),
      '#default_value' => isset($settings['indent_children']) ? $settings['indent_children'] : FALSE,
      '#description' => t('Indentation option for the responsive collapsed submenus. If set to true, all submenus will be indented with the value of the childrenIndenter option.'),
    );
    $form['children_indenter'] = array(
      '#type' => 'textfield',
      '#title' => t('Children indenter'),
      '#default_value' => isset($settings['children_indenter']) ? $settings['children_indenter'] : '&nbsp;',
      '#description' => t('This character will be prepended and responsive submenus will be indented with this character according to their level.'),
    );
    $form['expand_icon'] = array(
      '#type' => 'textfield',
      '#title' => t('Expand icon'),
      '#default_value' => isset($settings['expand_icon']) ? $settings['expand_icon'] : '<i>&#9660;<i>',
      '#description' => t('An icon to be displayed next to parent menu of collapsed sub menus.'),
    );
    $form['collapse_icon'] = array(
      '#type' => 'textfield',
      '#title' => t('Collapse icon'),
      '#default_value' => isset($settings['collapse_icon']) ? $settings['collapse_icon'] : '<i>&#9650;<i>',
      '#description' => t('An icon to be displayed next to parent menu of expanded sub menus.'),
    );
    
    return $form;
  }

  /**
   * Build settings.
   */
  function buildSettings($settings = array()) {
    $style = $settings['style_settings'];

    $style['resizeWidth'] = isset($style['resize_width']) ? $style['resize_width'] : 800;
    $style['initiallyVisible'] = isset($style['initially_visible']) ? $style['initially_visible'] : 'false';
    $style['collapserTitle'] = isset($style['collapser_title']) ? $style['collapser_title'] : 'Menu';
    $style['animSpeed'] = isset($style['anim_speed']) ? $style['anim_speed'] : 'medium';
    $style['easingEffect'] = isset($style['easing_effect']) ? $style['easing_effect'] : 'null';
    $style['indentChildren'] = isset($style['indent_children']) ? $style['indent_children'] : 'false';
    $style['childrenIndenter'] = isset($style['children_indenter']) ? $style['children_indenter'] : '&nbsp;';
    $style['expandIcon'] = isset($style['expand_icon']) ? $style['expand_icon'] : '<i>&#9660;<i>';
    $style['collapseIcon'] = isset($style['collapse_icon']) ? $style['collapse_icon'] : '<i>&#9650;<i>';
    $style['menuName'] = isset($settings['menu_name']) ? $settings['menu_name'] : NULL;
    return $style;
  }

  /**
   * Submit handler for saving the form settings.
   */
  function formSubmit($form, $form_state) {
    $this->settings = $form_state['values']['style_settings'];
  }
}
