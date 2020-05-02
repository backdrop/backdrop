<?php
/**
 * @file
 * Provides the basic object definitions used by plugins and handlers.
 */

/**
 * Basic definition for many views objects.
 */
class views_object {
  /**
   * Except for displays, options for the object will be held here.
   */
  var $options = array();

  /**
   * The top object of a view.
   *
   * @var view
   */
  var $view = NULL;

  /**
   * Handler's definition
   *
   * @var array
   */
  var $definition;

  /**
   * Information about options for all kinds of purposes will be held here.
   * @code
   * 'option_name' => array(
   *  - 'default' => default value,
   *  - 'translatable' => (optional) TRUE/FALSE,
   *  - 'contains' => (optional) array of items this contains, with its own
   *      defaults, etc. If contains is set, the default will be ignored and
   *      assumed to be array().
   *  - 'bool' => (optional) TRUE/FALSE Is the value a boolean value. This will
   *      change the internal format to TRUE/FALSE instead of 1/0.
   *  ),
   *
   * @return array
   *   Returns the options of this handler/plugin.
   *
   * @see views_object::unpack_translatable()
   */
  function option_definition() { return array(); }

  /**
   * Views handlers use a special construct function so that we can more easily
   * construct them with variable arguments.
   */
  function construct() { $this->set_default_options(); }

  /**
   * Set default options on this object. Called by the constructor in a
   * complex chain to deal with backward compatibility.
   *
   * @deprecated since views2
   */
  function options(&$options) { }

  /**
   * Set default options.
   * For backward compatibility, it sends the options array; this is a
   * feature that will likely disappear at some point.
   */
  function set_default_options() {
    $this->_set_option_defaults($this->options, $this->option_definition());

    // Retained for complex defaults plus backward compatibility.
    $this->options($this->options);
  }

  function _set_option_defaults(&$storage, $options, $level = 0) {
    foreach ($options as $option => $definition) {
      if (isset($definition['contains']) && is_array($definition['contains'])) {
        $storage[$option] = array();
        $this->_set_option_defaults($storage[$option], $definition['contains'], $level++);
      }
      elseif (!empty($definition['translatable']) && !empty($definition['default'])) {
        $storage[$option] = t($definition['default']);
      }
      else {
        $storage[$option] = isset($definition['default']) ? $definition['default'] : NULL;
      }
    }
  }

  /**
   * Unpack options over our existing defaults, drilling down into arrays
   * so that defaults don't get totally blown away.
   */
  function unpack_options(&$storage, $options, $definition = NULL, $all = TRUE, $check = TRUE) {
    if ($check && !is_array($options)) {
      return;
    }

    if (!isset($definition)) {
      $definition = $this->option_definition();
    }

    if (!empty($this->view)) {
      // Ensure we have a localization plugin.
      $this->view->init_localization();

      // Set up default localization keys. Handlers and such set this for us.
      if (empty($localization_keys) && isset($this->localization_keys)) {
        $localization_keys = $this->localization_keys;
      }
      // but plugins don't because there isn't a common init() these days.
      elseif (!empty($this->is_plugin) && empty($localization_keys)) {
        if ($this->plugin_type != 'display') {
          $localization_keys = array($this->view->current_display);
          $localization_keys[] = $this->plugin_type;
        }
      }
    }

    foreach ($options as $key => $value) {
      if (is_array($value)) {
        // Ignore arrays with no definition.
        if (!$all && empty($definition[$key])) {
          continue;
        }

        if (!isset($storage[$key]) || !is_array($storage[$key])) {
          $storage[$key] = array();
        }

        // If we're just unpacking our known options, and we're dropping an
        // unknown array (as might happen for a dependent plugin fields) go
        // ahead and drop that in.
        if (!$all && isset($definition[$key]) && !isset($definition[$key]['contains'])) {
          $storage[$key] = $value;
          continue;
        }

        $this->unpack_options($storage[$key], $value, isset($definition[$key]['contains']) ? $definition[$key]['contains'] : array(), $all, FALSE);
      }
      elseif ($all || !empty($definition[$key])) {
        $storage[$key] = $value;
      }
    }
  }

  /**
   * Let the handler know what its full definition is.
   */
  function set_definition($definition) {
    $this->definition = $definition;
    if (isset($definition['field'])) {
      $this->real_field = $definition['field'];
    }
  }

  function destroy() {
    if (isset($this->view)) {
      unset($this->view);
    }

    if (isset($this->display)) {
      unset($this->display);
    }

    if (isset($this->query)) {
      unset($this->query);
    }
  }

  /**
   * Unpacks each handler to store translatable texts.
   */
  public function unpack_translatables(&$translatable, $parents = array()) {
    foreach ($this->option_definition() as $option => $definition) {
      $this->unpack_translatable($translatable, $this->options, $option, $definition, $parents, array());
    }
  }

  /**
   * Unpack a single option definition.
   *
   * This function run's through all suboptions recursive.
   *
   * @param array $translatable
   *   Stores all available translatable items.
   * @param array $storage
   * @param string $option
   * @param string $definition
   * @param array $parents
   * @param array $keys
   */
  public function unpack_translatable(&$translatable, $storage, $option, $definition, $parents, $keys = array()) {
    // Do not export options for which we have no settings.
    if (!isset($storage[$option])) {
      return;
    }

    // Special handling for some items.
    if (isset($definition['unpack_translatable']) && method_exists($this, $definition['unpack_translatable'])) {
      return $this->{$definition['unpack_translatable']}($translatable, $storage, $option, $definition, $parents, $keys);
    }

    if (isset($definition['translatable'])) {
      if ($definition['translatable'] === FALSE) {
        return;
      }
    }
    if (!empty($definition['translatable']) && !empty($options)) {
      $value = $options;
      // Build source data and add to the array.
      $format = NULL;
      if (isset($definition['format_key']) && isset($storage[$definition['format_key']])) {
        $format = $storage[$definition['format_key']];
      }
      $translatable[] = array(
        'value' => $value,
        'keys' => isset($translation_keys) ? $translation_keys : $parents,
        'format' => $format,
      );
    }
  }

}
