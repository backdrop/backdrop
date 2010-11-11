<?php
// $Id: base.inc,v 1.3.4.5 2010/08/26 09:41:55 dereine Exp $
/**
 * @file
 *
 * Provides the basic object definitions used by plugins and handlers.
 */

/**
 * Basic definition for many views objects
 */
class views_object {
  /**
   * Except for displays, options for the object will be held here.
   */
  var $options = array();
  /**
   * Information about options for all kinds of purposes will be held here.
   * @code
   * 'option_name' => array(
   *  - 'default' => default value,
   *  - 'translatable' => TRUE/FALSE (wrap in t() on export if true),
   *  - 'contains' => array of items this contains, with its own defaults, etc.
   *      If contains is set, the default will be ignored and assumed to
   *      be array()
   *
   *  ),
   *  @endcode
   * Each option may have any of the following functions:
   *  - export_option_OPTIONNAME -- Special export handling if necessary.
   *  - translate_option_OPTIONNAME -- Special handling for translating data
   *    within the option, if necessary.
   */
  function option_definition() { return array(); }

  /**
   * Views handlers use a special construct function so that we can more
   * easily construct them with variable arguments.
   */
  function construct() { $this->set_default_options(); }

  /**
   * Set default options on this object. Called by the constructor in a
   * complex chain to deal with backward compatibility.
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
      elseif (!empty($definition[$key]['translatable']) && !empty($value)) {
        $storage[$key] = t($value);
      }
      else if ($all || !empty($definition[$key])) {
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

  function export_options($indent, $prefix) {
    $output = '';
    foreach ($this->option_definition() as $option => $definition) {
      $output .= $this->export_option($indent, $prefix, $this->options, $option, $definition, array());
    }

    return $output;
  }

  function export_option($indent, $prefix, $storage, $option, $definition, $parents) {
    // Do not export options for which we have no settings.
    if (!isset($storage[$option])) {
      return;
    }

    if (isset($definition['export'])) {
      if ($definition['export'] === FALSE) {
        return;
      }

      // Special handling for some items
      if (method_exists($this, $definition['export'])) {
        return $this->{$definition['export']}($indent, $prefix, $storage, $option, $definition, $parents);
      }
    }

    // Add the current option to the parents tree.
    $parents[] = $option;
    $output = '';

    // If it has child items, export those separately.
    if (isset($definition['contains'])) {
      foreach ($definition['contains'] as $sub_option => $sub_definition) {
        $output .= $this->export_option($indent, $prefix, $storage[$option], $sub_option, $sub_definition, $parents);
      }
    }
    // Otherwise export just this item.
    else {
      $default = isset($definition['default']) ? $definition['default'] : NULL;
      $value = $storage[$option];
      if (isset($definition['bool'])) {
        $value = (bool) $value;
      }

      if ($value !== $default) {
        $output .= $indent . $prefix . "['" . implode("']['", $parents) . "'] = ";
        if (isset($definition['bool'])) {
          $output .= empty($storage[$option]) ? 'FALSE' : 'TRUE';
        }
        else {
          $output .= views_var_export($storage[$option], $indent);
        }

        $output .= ";\n";
      }
    }
    return $output;
  }
}
