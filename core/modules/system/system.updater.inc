<?php

/**
 * @file
 * Subclasses of the Updater class to update Backdrop core knows how to update.
 * At this time, only modules, themes and layouts are supported.
 */

/**
 * Class for updating modules using FileTransfer classes via authorize.php.
 */
class ModuleUpdater extends Updater implements BackdropUpdaterInterface {

  /**
   * Return the directory where a module should be installed.
   *
   * If the module is already installed, backdrop_get_path() will return
   * a valid path and we should install it there (although we need to use an
   * absolute path, so we prepend BACKDROP_ROOT). If we're installing a new
   * module, we always want it to go into /modules, since that's
   * where all the documentation recommends users install their modules, and
   * there's no way that can conflict on a multi-site installation, since
   * the Update manager won't let you install a new module if it's already
   * found on your system, and if there was a copy in top-level, we'd see it.
   */
  public function getInstallDirectory() {
    if ($relative_path = backdrop_get_path('module', $this->name)) {
      $relative_path = dirname($relative_path);
    }
    else {
      $relative_path = 'modules';
    }
    return BACKDROP_ROOT . '/' . $relative_path;
  }

  public function isInstalled() {
    return (bool) backdrop_get_path('module', $this->name);
  }

  public static function canUpdateDirectory($directory) {
    return (update_get_package_type($directory) == 'module');
  }

  public static function canUpdate($project_name) {
    return (bool) backdrop_get_path('module', $project_name);
  }

  /**
   * Return available database schema updates one a new version is installed.
   */
  public function getSchemaUpdates() {
    require_once BACKDROP_ROOT . '/core/includes/install.inc';
    require_once BACKDROP_ROOT . '/core/includes/update.inc';

    if (_update_get_project_type($project) != 'module') {
      return array();
    }
    module_load_include('install', $project);

    if (!$updates = backdrop_get_schema_versions($project)) {
      return array();
    }
    $updates_to_run = array();
    $modules_with_updates = update_get_update_list();
    if ($updates = $modules_with_updates[$project]) {
      if ($updates['start']) {
        return $updates['pending'];
      }
    }
    return array();
  }

  /**
   * List of post install actions.
   */
  public function postInstallTasks() {
    return array(
      l(t('Install another module'), 'admin/modules/install'),
      l(t('Enable newly added modules'), 'admin/modules'),
      l(t('Administration pages'), 'admin'),
    );
  }

  public function postUpdateTasks() {
    // We don't want to check for DB updates here, we do that once for all
    // updated modules on the landing page.
  }

}

/**
 * Class for updating themes using FileTransfer classes via authorize.php.
 */
class ThemeUpdater extends Updater implements BackdropUpdaterInterface {

  /**
   * Return the directory where a theme should be installed.
   *
   * If the theme is already installed, backdrop_get_path() will return
   * a valid path and we should install it there (although we need to use an
   * absolute path, so we prepend BACKDROP_ROOT). If we're installing a new
   * theme, we always want it to go into /themes, since that's
   * where all the documentation recommends users install their themes, and
   * there's no way that can conflict on a multi-site installation, since
   * the Update manager won't let you install a new theme if it's already
   * found on your system, and if there was a copy in top-level, we'd see it.
   */
  public function getInstallDirectory() {
    if ($relative_path = backdrop_get_path('theme', $this->name)) {
      $relative_path = dirname($relative_path);
    }
    else {
      $relative_path = 'themes';
    }
    return BACKDROP_ROOT . '/' . $relative_path;
  }

  public function isInstalled() {
    return (bool) backdrop_get_path('theme', $this->name);
  }

  static function canUpdateDirectory($directory) {
    return (update_get_package_type($directory) == 'theme');
  }

  public static function canUpdate($project_name) {
    return (bool) backdrop_get_path('theme', $project_name);
  }

  public function postInstall() {
    // Update the system table.
    clearstatcache();
    system_rebuild_theme_data();
  }

  public function postInstallTasks() {
    return array(
      l(t('Enable newly added themes'), 'admin/appearance'),
      l(t('Administration pages'), 'admin'),
    );
  }
}

/**
 * Class for updating layouts using FileTransfer classes via authorize.php.
 */
class LayoutUpdater extends Updater implements BackdropUpdaterInterface {

  /**
   * Return the directory where a layout should be installed.
   *
   * If the layout is already installed, backdrop_get_path() will return
   * a valid path and we should install it there (although we need to use an
   * absolute path, so we prepend BACKDROP_ROOT). If we're installing a new
   * layout, we always want it to go into /layouts, since that's
   * where all the documentation recommends users install their layouts, and
   * there's no way that can conflict on a multi-site installation, since
   * the Update manager won't let you install a new layout if it's already
   * found on your system, and if there was a copy in top-level, we'd see it.
   */
  public function getInstallDirectory() {
    if ($relative_path = backdrop_get_path('layout', $this->name)) {
      $relative_path = dirname($relative_path);
    }
    else {
      $relative_path = 'layouts';
    }
    return BACKDROP_ROOT . '/' . $relative_path;
  }

  public function isInstalled() {
    return (bool) backdrop_get_path('layout', $this->name);
  }

  static function canUpdateDirectory($directory) {
    return (update_get_package_type($directory) == 'layout');
  }

  public static function canUpdate($project_name) {
    return (bool) backdrop_get_path('layout', $project_name);
  }

  public function postInstall() {
    layout_reset_caches();
    clearstatcache();
  }

  public function postInstallTasks() {
    return array(
      l(t('Use newly added layouts'), 'admin/structure/layouts'),
      l(t('Administration pages'), 'admin'),
    );
  }
}
