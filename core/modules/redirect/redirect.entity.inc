<?php

/**
 * Defines the Redirect entity class.
 */
class Redirect extends Entity {

  /**
   * The redirect ID.
   *
   * @var integer
   */
  public $rid;

  /**
   * The type of the redirect entity. May be:
   *
   * - redirect: A redirect that is saved and loaded from the database.
   * - manual: A single-use redirect that was manually created on-the-fly.
   *
   * @var string
   */
  public $type = 'redirect';

  /**
   * The comment language code.
   *
   * @var string
   */
  public $langcode = LANGUAGE_NONE;

  /**
   * The from which the redirect will start.
   *
   * @var string
   */
  public $source = '';

  /**
   * An array of options for the source of the redirect.
   *
   * @var array
   */
  public $source_options = array();

  /**
   * The path to which the source will be redirected.
   *
   * @var string
   */
  public $redirect;

  /**
   * An array of options for the redirect.
   *
   * @var array
   */
  public $redirect_options = array();

  /**
   * The function callback that will execute the redirect.
   *
   * @var string
   */
  public $callback = 'redirect_goto';

  /**
   * The redirect creator ID.
   *
   * @var int
   */
  public $uid = 0;

  /**
   * The redirect status code.
   *
   * @var int
   */
  public $status_code = 0;

  /**
   * The total number of times this redirect has been followed.
   *
   * @var int
   */
  public $count = 0;

  /**
   * Hashes are an alternative ID by which a redirect may be loaded.
   *
   * @var string
   */
  public $hash = '';

  /**
   * Implements EntityInterface::id().
   */
  public function id() {
    return $this->rid;
  }

  /**
   * Implements EntityInterface::entityType().
   */
  public function entityType() {
    return 'redirect';
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return $this->subject;
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    return array(
      'path' => $this->redirect,
      'options' => $this->redirect_options,
    );
  }

  /**
   * Constructor for creating a new redirect.
   *
   * @param array $values
   *   An array of values to populate the redirect object.
   */
  public function __construct(array $values = array()) {
    parent::__construct($values);

    if (empty($this->status_code)) {
      $this->status_code = config_get('redirect.settings', 'default_status_code');
    }

    // Unserialize the URL option fields.
    if (is_string($this->source_options)) {
      $this->source_options = unserialize($this->source_options);
    }
    if (is_string($this->redirect_options)) {
      $this->redirect_options = unserialize($this->redirect_options);
    }

    module_invoke_all('redirect_prepare', $this);
  }
}

/**
 * Controller class for redirects.
 *
 * This extends the DefaultEntityController class, adding required
 * special handling for redirect objects.
 */
class RedirectController extends DefaultEntityController {
  public function save(EntityInterface $redirect) {
    $transaction = db_transaction();

    try {
      if (!empty($redirect->rid) && !isset($redirect->original)) {
        $redirect->original = entity_load_unchanged('redirect', $redirect->rid);
      }

      // Determine if we will be inserting a new node.
      if (!isset($redirect->is_new)) {
        $redirect->is_new = empty($redirect->rid);
      }

      // The changed timestamp is always updated for bookkeeping purposes.
      //$redirect->changed = time();

      redirect_hash($redirect);
      if ($redirect->is_new || $redirect->hash != $redirect->original->hash) {
        // Only new or changed redirects reset the last used value.
        $redirect->count = 0;
        $redirect->access = 0;
      }

      // Allow other modules to alter the redirect before saving.
      module_invoke_all('redirect_presave', $redirect);
      module_invoke_all('entity_presave', $redirect, 'redirect');

      // Save the redirect to the database and invoke the post-save hooks.
      if ($redirect->is_new) {
        backdrop_write_record('redirect', $redirect);
        module_invoke_all('redirect_insert', $redirect);
        module_invoke_all('entity_insert', $redirect, 'redirect');
      }
      else {
        backdrop_write_record('redirect', $redirect, array('rid'));
        module_invoke_all('redirect_update', $redirect);
        module_invoke_all('entity_update', $redirect, 'redirect');
      }

      // Clear internal properties.
      unset($redirect->is_new);
      unset($redirect->original);

      // Clear the static loading cache.
      $this->resetCache(array($redirect->rid));

      // Ignore slave server temporarily to give time for the
      // saved node to be propagated to the slave.
      db_ignore_slave();
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('redirect', $e);
      throw $e;
    }
  }

  public function delete($rids) {
    $transaction = db_transaction();
    if (!empty($rids)) {
      $redirects = redirect_load_multiple($rids);

      try {
        // Let modules react to the individual redirects being deleted.
        foreach ($redirects as $rid => $redirect) {
          module_invoke_all('redirect_delete', $redirect);
          module_invoke_all('entity_delete', $redirect, 'redirect');
        }

        db_delete('redirect')
          ->condition('rid', $rids, 'IN')
          ->execute();
      }
      catch (Exception $e) {
        $transaction->rollback();
        watchdog_exception('redirect', $e);
        throw $e;
      }

      $this->resetCache();
    }
  }
}
