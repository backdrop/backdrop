<?php
/**
 * @file
 * Layout context handler for entities.
 */
class EntityLayoutContext extends LayoutContext {
  /**
   * The entity type for this context.
   *
   * @var string
   */
  protected $entity_type;

  function __construct($plugin_name, $config = array()) {
    parent::__construct($plugin_name, $config);

    $this->entity_type = $plugin_name;
    $this->aliases = array('entity');
  }

  function type() {
    return 'entity';
  }

  function label() {
    $info = entity_get_info($this->entity_type);
    // @todo: Contexts do not yet have settings to make this possible. Once
    // the ability to add manual contexts (e.g. an arbitrary node), then this
    // will actually do something.
    if (isset($this->settings['id'])) {
      if ($entity = entity_load($this->entity_type, $this->settings['id'])) {
        return t('@type: @id (@title)', array('@type' => $info['label'], '@id' => $entity->id(), '@title' => $entity->label()));
      }
      else {
        return t('Unknown @type: @id', array('@type' => $info['label'], '@id' =>$this->settings['id']));
      }
    }
    if ($this->entity_type === 'user' && isset($this->name) && $this->name === 'current_user') {
      return t('Current user');
    }
    else {
      return parent::label();
    }
  }

  function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    // $conf = &$form_state['conf'];


    $form['entity'] = array(
      '#title' => t('Enter the title or ID of a @entity entity', array('@entity' => $this->plugin)),
      '#type' => 'textfield',
      '#maxlength' => 512,
      '#autocomplete_path' => 'path-autocomplete',
      '#weight' => -10,
    );

/*     if (!empty($conf['entity_id'])) {
      $info = entity_load($plugin['keyword'], array($conf['entity_id']));
      $info = $info[$conf['entity_id']];
      if ($info) {
        $entity = entity_get_info($plugin['keyword']);
        $uri = entity_uri($plugin['keyword'], $info);
        if (is_array($uri) && $entity['entity keys']['label']) {
          $link = l(t("'%title' [%type id %id]", array('%title' => $info->{$entity['entity keys']['label']}, '%type' => $plugin['keyword'], '%id' => $conf['entity_id'])), $uri['path'], array('attributes' => array('target' => '_blank', 'title' => t('Open in new window')), 'html' => TRUE));
        }
        elseif (is_array($uri)) {
          $link = l(t("[%type id %id]", array('%type' => $plugin['keyword'], '%id' => $conf['entity_id'])), $uri['path'], array('attributes' => array('target' => '_blank', 'title' => t('Open in new window')), 'html' => TRUE));
        }
        elseif ($entity['entity keys']['label']) {
          $link = l(t("'%title' [%type id %id]", array('%title' => $info->{$entity['entity keys']['label']}, '%type' => $plugin['keyword'], '%id' => $conf['entity_id'])), file_create_url($uri), array('attributes' => array('target' => '_blank', 'title' => t('Open in new window')), 'html' => TRUE));
        }
        else {
          $link = t("[%type id %id]", array('%type' => $plugin['keyword'], '%id' => $conf['entity_id']));
        }
        $form['entity']['#description'] = t('Currently set to !link', array('!link' => $link));
      }
    }

    $form['entity_id'] = array(
      '#type' => 'value',
      '#value' => $conf['entity_id'],
    );

    $form['entity_type'] = array(
      '#type' => 'value',
      '#value' => $plugin['keyword'],
    );

    $form['set_identifier'] = array(
      '#type' => 'checkbox',
      '#default_value' => FALSE,
      '#title' => t('Reset identifier to entity label'),
      '#description' => t('If checked, the identifier will be reset to the entity label of the selected entity.'),
    );
 */
  }

  /**
   * Submit handler for the settings form for this context item.
   */
  function formSubmit($form, &$form_state) {
    $excluded = backdrop_map_assoc(array('cancel', 'submit', 'form_build_id', 'form_token', 'form_id', 'op', 'add', 'context'));
    $settings = array_diff_key($form_state['values'], $excluded);
    $this->name = $this->plugin . $settings['entity'];
    // @todo deal with name changes.
    $this->settings = array_merge($this->settings, $settings);
  }
}
