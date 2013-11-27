<?
/**
 * Declare Plugins Provided by the module.
 *
 * A plugin is a class with meta-data which can be loaded and modified by
 * modules without the necessity of loading the class itself, describing classes
 * that can be autoloaded. Not all classes are plugins, but all plugins are
 * classes.
 *
 * Plugin Classes should be in one class per file, with the file named the same
 * as the class, with a '.inc' extension.
 *
 * @see plugin_info()
 */
function hook_plugin_info() {
  $plugins['the_type']['mymodule\FullyNameSpaced\MyPluginClassName'] = array(
    'title' => t('My own plugin'),
    // Optional sub-directory of module where class file is kept. Defaults to module root.
    'path' => 'lib/classes',
    // Optional properties that can be defined by the class implementing the plugins.
    'properties' => array('something' => 'anything I want'),
  );

  return $plugins;
}