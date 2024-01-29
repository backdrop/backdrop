<?php
/**
 * @file
 * Entity controller and class for nodes.
 */

/**
 * Defines the node entity class.
 */
class Node extends Entity {

  /**
   * The node ID.
   *
   * @var integer
   */
  public $nid;

  /**
   * The node revision ID.
   *
   * @var integer
   */
  public $vid;

  /**
   * Indicates whether this is the active node revision.
   *
   * The active revision of a node is the one loaded when no specific revision
   * has been specified. Only active revisions are saved to the node table.
   *
   * @var boolean
   */
  public $is_active_revision = TRUE;

  /**
   * The node content type (bundle).
   *
   * @var string
   */
  public $type;

  /**
   * The node language code.
   *
   * @var string
   */
  public $langcode = LANGUAGE_NONE;

  /**
   * The node title.
   *
   * @var string
   */
  public $title;

  /**
   * The node owner's user ID.
   *
   * @var integer
   */
  public $uid;

  /**
   * The node published status indicator.
   *
   * Unpublished nodes are only visible to their authors and to administrators.
   * The value is either NODE_PUBLISHED or NODE_NOT_PUBLISHED.
   *
   * @var integer
   */
  public $status;

  /**
   * The node creation timestamp.
   *
   * @var integer
   */
  public $created;

  /**
   * The node modification timestamp.
   *
   * @var integer
   */
  public $changed;

  /**
   * The timestamp at which this content should be automatically published.
   *
   * @var integer
   */
  public $scheduled;

  /**
   * The node comment status indicator.
   *
   * COMMENT_NODE_HIDDEN => no comments
   * COMMENT_NODE_CLOSED => comments are read-only
   * COMMENT_NODE_OPEN => open (read/write)
   *
   * @var integer
   */
  public $comment;

  /**
   * The node promotion status.
   *
   * Promoted nodes should be displayed on the home page of the site. The
   * value is either NODE_PROMOTED or NODE_NOT_PROMOTED.
   *
   * @var integer
   */
  public $promote;

  /**
   * The node sticky status.
   *
   * Sticky nodes should be displayed at the top of lists in which they appear.
   * The value is either NODE_STICKY or NODE_NOT_STICKY.
   *
   * @var integer
   */
  public $sticky;

  /**
   * The node translation set ID.
   *
   * Translations sets are based on the ID of the node containing the source
   * text for the translation set.
   *
   * @var integer
   */
  public $tnid;

  /**
   * The node translation status.
   *
   * If the translation page needs to be updated the value is 1, otherwise 0.
   *
   * @var integer
   */
  public $translate;

  /**
   * The node revision creation timestamp.
   *
   * @var integer
   */
  public $revision_timestamp;

  /**
   * The node revision author's user ID.
   *
   * @var integer
   */
  public $revision_uid;

  /**
   * The node preview status.
   *
   * @var boolean
   *
   * @since 1.11.0
   */
  public $in_preview;


  /**
   * Create a new Node instance.
   */
  function __construct(array $values = array()) {
    parent::__construct($values);

    // Set the created time to now.
    if (empty($this->created)) {
      $this->created = REQUEST_TIME;
    }
  }

  /**
   * Implements EntityInterface::id().
   */
  public function id() {
    return isset($this->nid) ? $this->nid : NULL;
  }

  /**
   * Implements EntityInterface::entityType().
   */
  public function entityType() {
    return 'node';
  }

  /**
   * Implements EntityInterface::bundle().
   */
  public function bundle() {
    return $this->type;
  }

  /**
   * Implements EntityInterface::label().
   */
  public function label() {
    return $this->title;
  }

  /**
   * Implements EntityInterface::uri().
   */
  public function uri() {
    return array(
      'path' => 'node/' . $this->nid,
      'options' => array(),
    );
  }

  /**
   * Overrides Entity::createAccess().
   */
  public static function createAccess($bundle = NULL, $account = NULL) {
    $rights = &backdrop_static('node_access', array());

    // If no user object is supplied, the access check is for the current user.
    if (empty($account)) {
      $account = $GLOBALS['user'];
    }

    // If we've already checked access for this node, user and op, return from
    // cache.
    if (isset($rights[$account->uid][$bundle])) {
      return $rights[$account->uid][$bundle];
    }

    if (user_access('bypass node access', $account)) {
      $rights[$account->uid][$bundle] = TRUE;
      return $rights[$account->uid][$bundle];
    }
    if (!user_access('access content', $account)) {
      $rights[$account->uid][$bundle] = FALSE;
      return $rights[$account->uid][$bundle];
    }

    // We grant access to the node if both of the following conditions are met:
    // - No modules say to deny access.
    // - At least one module says to grant access.
    // If no module specified either allow or deny, we fall back to the
    // node_access table.
    $access = module_invoke_all('node_access', $bundle, 'create', $account);
    if (in_array(NODE_ACCESS_DENY, $access, TRUE)) {
      $rights[$account->uid][$bundle] = FALSE;
      return $rights[$account->uid][$bundle];
    }
    elseif (in_array(NODE_ACCESS_ALLOW, $access, TRUE)) {
      $rights[$account->uid][$bundle] = TRUE;
      return $rights[$account->uid][$bundle];
    }

    $rights[$account->uid][$bundle] = FALSE;
    return $rights[$account->uid][$bundle];
  }

  /**
   * Overrides Entity::access().
   *
   * @param string $op
   *   The operation to be performed on the node. Possible values are:
   *   - create
   *   - view
   *   - update
   *   - delete
   * @param User|AnonymousUser|object $account
   *   (optional) The user to check for. Leave it to NULL to check for the
   *   global user.
   *
   * @return bool|NULL
   *   TRUE if access is granted, FALSE otherwise.
   */
  public function access($op, $account = NULL) {
    // Casting class with private property causes errors due to added prefix.
    // e.g. "\0" . 'Node' . "\0". So use static array instead.
    $rights = &backdrop_static('node_access', array());

    if ($op == 'create') {
      return self::createAccess($this->bundle(), $account);
    }
    elseif (!in_array($op, array('view', 'update', 'delete'), TRUE)) {
      // If the $op was not one of the supported ones, we return access denied.
      return FALSE;
    }
    // If no user object is supplied, the access check is for the current user.
    if (empty($account)) {
      $account = $GLOBALS['user'];
    }

    $cid = $this->id();

    // If we've already checked access for this node, user and op, return from
    // cache.
    if (isset($rights[$account->uid][$cid][$op])) {
      return $rights[$account->uid][$cid][$op];
    }

    if (user_access('bypass node access', $account)) {
      $rights[$account->uid][$cid][$op] = TRUE;
      return $rights[$account->uid][$cid][$op];
    }
    if (!user_access('access content', $account)) {
      $rights[$account->uid][$cid][$op] = FALSE;
      return $rights[$account->uid][$cid][$op];
    }

    // We grant access to the node if both of the following conditions are met:
    // - No modules say to deny access.
    // - At least one module says to grant access.
    // If no module specified either allow or deny, we fall back to the
    // node_access table.
    $access = module_invoke_all('node_access', $this, $op, $account);
    if (in_array(NODE_ACCESS_DENY, $access, TRUE)) {
      $rights[$account->uid][$cid][$op] = FALSE;
      return $rights[$account->uid][$cid][$op];
    }
    elseif (in_array(NODE_ACCESS_ALLOW, $access, TRUE)) {
      $rights[$account->uid][$cid][$op] = TRUE;
      return $rights[$account->uid][$cid][$op];
    }

    // Check if user can view any unpublished nodes.
    if ($op == 'view' && !$this->status && user_access('view any unpublished content', $account) && $account->uid != 0) {
      $rights[$account->uid][$cid][$op] = TRUE;
      return $rights[$account->uid][$cid][$op];
    }

    // Check if authors can view their own unpublished nodes.
    if ($op == 'view' && !$this->status && user_access('view own unpublished content', $account) && $account->uid == $this->uid && $account->uid != 0) {
      $rights[$account->uid][$cid][$op] = TRUE;
      return $rights[$account->uid][$cid][$op];
    }

    // If the module did not override the access rights, use those set in the
    // node_access table.
    if ($this->id()) {
      if (module_implements('node_grants')) {
        $query = db_select('node_access');
        $query->addExpression('1');
        $query->condition('grant_' . $op, 1, '>=');
        $nids = db_or()->condition('nid', $this->id());
        if ($this->status) {
          $nids->condition('nid', 0);
        }
        $query->condition($nids);
        $query->range(0, 1);

        $grants = db_or();
        foreach (node_access_grants($op, $account) as $realm => $gids) {
          foreach ($gids as $gid) {
            $grants->condition(db_and()
              ->condition('gid', $gid)
              ->condition('realm', $realm)
            );
          }
        }
        if (count($grants) > 0) {
          $query->condition($grants);
        }
        $result = (bool) $query
          ->execute()
          ->fetchField();
        $rights[$account->uid][$cid][$op] = $result;
        return $rights[$account->uid][$cid][$op];
      }
      elseif ($op == 'view' && $this->status) {
        // If no modules implement hook_node_grants(), the default behavior is
        // to allow all users to view published nodes, so reflect that here.
        $rights[$account->uid][$cid][$op] = TRUE;
        return $rights[$account->uid][$cid][$op];
      }
    }

    $rights[$account->uid][$cid][$op] = FALSE;
    return $rights[$account->uid][$cid][$op];
  }

  /**
   * Overrides Entity::createDuplicate().
   */
  public function createDuplicate() {
    $duplicate = clone $this;
    $duplicate->nid = NULL;
    $duplicate->vid = NULL;
    return $duplicate;
  }

  /**
   * Overrides Backdrop\entity\Entity::getRevisionId().
   */
  public function getRevisionId() {
    return $this->vid;
  }
}


/**
 * Controller class for nodes.
 *
 * This extends the EntityDatabaseStorageController class, adding required
 * special handling for node entities.
 */
class NodeStorageController extends EntityDatabaseStorageController {

  /**
   * Overrides EntityDatabaseStorageController::delete().
   */
  public function delete($ids) {
    $entities = $ids ? $this->load($ids) : FALSE;
    if (!$entities) {
      // If no IDs or invalid IDs were passed, do nothing.
      return;
    }
    $transaction = db_transaction();

    try {
      $this->preDelete($entities);
      foreach ($entities as $id => $entity) {
        $this->invokeHook('predelete', $entity);
      }
      $ids = array_keys($entities);

      db_delete($this->entityInfo['base table'])
        ->condition($this->idKey, $ids, 'IN')
        ->execute();

      if ($this->revisionKey) {
        db_delete($this->revisionTable)
          ->condition($this->idKey, $ids, 'IN')
          ->execute();
      }

      // Reset the cache as soon as the changes have been applied.
      $this->resetCache($ids);

      $this->postDelete($entities);
      foreach ($entities as $id => $entity) {
        $this->invokeHook('delete', $entity);
      }
      // Ignore replica server temporarily.
      db_ignore_replica();
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception($this->entityType, $e);
      throw new EntityStorageException($e->getMessage(), (int) $e->getCode(), $e);
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::save().
   *
   * @param Node $entity
   *   The node entity being saved.
   */
  public function save(EntityInterface $entity) {
    $transaction = db_transaction();
    try {
      // Load the stored entity, if any.
      if (!$entity->isNew() && !isset($entity->original)) {
        $entity->original = entity_load_unchanged($this->entityType, $entity->id());
      }

      $this->preSave($entity);
      $this->invokeHook('presave', $entity);

      if ($entity->isNew()) {
        $op = 'insert';
        $return = backdrop_write_record($this->entityInfo['base table'], $entity);
        unset($entity->is_new);
      }
      else {
        $op = 'update';
        // Update the base node table, but only if this revision is marked as
        // the active revision.
        if ($entity->isActiveRevision()) {
          $return = backdrop_write_record($this->entityInfo['base table'], $entity, $this->idKey);
        }
        else {
          $return = SAVED_UPDATED;
        }
      }

      if ($this->revisionKey) {
        $this->saveRevision($entity);
      }

      // Reset the persistent and static cache for this node.
      if ($op == 'update') {
        $this->resetCache(array($entity->{$this->idKey}));
      }

      $this->postSave($entity, $op == 'update');
      $this->invokeHook($op, $entity);

      // Ignore replica server temporarily.
      db_ignore_replica();
      unset($entity->original);

      return $return;
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception($this->entityType, $e);
      throw new EntityStorageException($e->getMessage(), (int) $e->getCode(), $e);
    }
  }

  /**
   * Saves a node revision.
   *
   * @param Node $entity
   *   The node entity whose revision should be saved.
   */
  protected function saveRevision(EntityInterface $entity) {
    $record = clone $entity;
    $record->uid = $entity->revision_uid;
    $record->timestamp = $entity->revision_timestamp;

    if (empty($entity->{$this->revisionKey}) || !empty($entity->revision)) {
      backdrop_write_record($this->revisionTable, $record);
      // Only update the base node table if this revision is the active
      // revision.
      if ($entity->isActiveRevision()) {
        db_update($this->entityInfo['base table'])
          ->fields(array($this->revisionKey => $record->{$this->revisionKey}))
          ->condition($this->idKey, $entity->{$this->idKey})
          ->execute();
      }
    }
    else {
      backdrop_write_record($this->revisionTable, $record, $this->revisionKey);
    }
    // Make sure to update the new revision key for the entity.
    $entity->{$this->revisionKey} = $record->{$this->revisionKey};
  }

  /**
   * Overrides DefaultEntityController::attachLoad().
   *
   * @param Node[] $nodes
   *   An array of nodes on which fields should be attached.
   */
  protected function attachLoad(&$nodes, $revision_id = FALSE) {
    // Create an array of nodes for each content type and pass this to the
    // object type specific callback.
    $typed_nodes = array();
    foreach ($nodes as $id => $entity) {
      $typed_nodes[$entity->type][$id] = $entity;
    }

    // Call object type specific callbacks on each typed array of nodes.
    foreach ($typed_nodes as $node_type => $nodes_of_type) {
      if (node_hook($node_type, 'load')) {
        $function = node_type_get_base($node_type) . '_load';
        $function($nodes_of_type);
      }
    }
    // Besides the list of nodes, pass one additional argument to
    // hook_node_load(), containing a list of node types that were loaded.
    $argument = array_keys($typed_nodes);
    $this->hookLoadArguments = array($argument);
    parent::attachLoad($nodes, $revision_id);
  }

  /**
   * Overrides DefaultEntityController::buildQuery().
   */
  protected function buildQuery($ids, $conditions = array(), $revision_id = FALSE) {
    // Ensure that uid is taken from the {node} table,
    // alias timestamp to revision_timestamp and add revision_uid.
    $query = parent::buildQuery($ids, $conditions, $revision_id);
    $fields =& $query->getFields();
    unset($fields['timestamp']);
    $query->addField('revision', 'timestamp', 'revision_timestamp');
    $fields['uid']['table'] = 'base';
    $query->addField('revision', 'uid', 'revision_uid');
    return $query;
  }

  /**
   * Overrides EntityDatabaseStorageController::invokeHook().
   *
   * @param string $hook
   *   One of 'presave', 'insert', 'update', 'predelete', or 'delete'.
   * @param Node $node
   *   The entity object, always a Node object in this case.
   */
  protected function invokeHook($hook, EntityInterface $node) {
    if ($hook == 'insert' || $hook == 'update') {
      node_invoke($node, $hook);
    }
    elseif ($hook == 'predelete') {
      // 'delete' is triggered in 'predelete' is here to preserve hook ordering
      // from Drupal 7.
      node_invoke($node, 'delete');
    }

    parent::invokeHook($hook, $node);

    if ($hook == 'presave') {
      if ($node->isNew() || !empty($node->revision)) {
        // When inserting either a new node or a new node revision, $node->log
        // must be set because {node_revision}.log is a text column and therefore
        // cannot have a default value. However, it might not be set at this
        // point (for example, if the user submitting a node form does not have
        // permission to create revisions), so we ensure that it is at least an
        // empty string in that case.
        // @todo: Make the {node_revision}.log column nullable so that we can
        // remove this check.
        if (!isset($node->log)) {
          $node->log = '';
        }
      }
      elseif (!isset($node->log) || $node->log === '') {
        // If we are updating an existing node without adding a new revision, we
        // need to make sure $node->log is unset whenever it is empty. As long as
        // $node->log is unset, backdrop_write_record() will not attempt to update
        // the existing database column when re-saving the revision; therefore,
        // this code allows us to avoid clobbering an existing log entry with an
        // empty one.
        unset($node->log);
      }

      // When saving a new node revision, unset any existing $node->vid so as to
      // ensure that a new revision will actually be created, then store the old
      // revision ID in a separate property for use by node hook implementations.
      if (!$node->isNew() && !empty($node->revision) && $node->vid) {
        $node->old_vid = $node->vid;
        $node->vid = NULL;
      }
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::preSave().
   *
   * @param Node $node
   *   The node object about to be saved.
   */
  protected function preSave(EntityInterface $node) {
    // Before saving the node, set changed and revision times.
    $node->changed = REQUEST_TIME;

    if ($this->revisionKey && !empty($node->revision)) {
      $node->revision_timestamp = REQUEST_TIME;

      if (!isset($node->revision_uid)) {
        $node->revision_uid = $GLOBALS['user']->uid;
      }
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::postSave().
   *
   * @param Node $node
   *   The node object that has just been saved.
   */
  function postSave(EntityInterface $node, $update) {
    // Update the node access table for this node, but only if it is the
    // active revision. There's no need to delete existing records if the node
    // is new.
    if ($node->isActiveRevision()) {
      node_access_acquire_grants($node, $update);
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::preDelete().
   *
   * @param Node[] $entities
   *   An array of node entities about to be deleted.
   */
  function preDelete($nodes) {
    if (module_exists('search')) {
      foreach ($nodes as $nid => $node) {
        search_reindex($node->nid, 'node');
      }
    }
  }

  /**
   * Overrides EntityDatabaseStorageController::postDelete().
   *
   * @param Node[] $nodes
   *   An array of node entities that have just been deleted.
   */
  protected function postDelete($nodes) {
    // Delete values from other tables also referencing this node.
    $ids = array_keys($nodes);

    db_delete('history')
      ->condition('nid', $ids, 'IN')
      ->execute();
    db_delete('node_access')
      ->condition('nid', $ids, 'IN')
      ->execute();
  }

  /**
   * Implements EntityControllerInterface::buildContent().
   */
  public function buildContent(EntityInterface $node, $view_mode = 'full', $langcode = NULL) {
    global $language_content;
    $langcode = $langcode ? $langcode : $language_content->langcode;

    // Remove previously built content, if exists.
    $node->content = array();

    // Allow modules to change the display mode.
    $view_mode = key(entity_view_mode_prepare('node', array($node->nid => $node), $view_mode, $langcode));

    // The 'view' hook can be implemented to overwrite the default function
    // to display nodes.
    if (node_hook($node, 'view')) {
      $node = node_invoke($node, 'view', $view_mode, $langcode);
    }

    // Build fields content.
    // In case of a multiple view, node_view_multiple() already ran the
    // 'prepare_view' step. An internal flag prevents the operation from running
    // twice.
    field_attach_prepare_view('node', array($node->nid => $node), $view_mode, $langcode);
    entity_prepare_view('node', array($node->nid => $node));
    $node->content += field_attach_view('node', $node, $view_mode, $langcode);

    // Always display a read more link on teasers because we have no way
    // to know when a teaser view is different than a full view.
    $links = array();
    $node->content['links'] = array(
      '#theme' => 'links__node',
      '#pre_render' => array('backdrop_pre_render_links'),
      '#attributes' => array('class' => array('links', 'inline')),
    );
    if ($view_mode == 'teaser') {
      $type = node_type_get_type($node);
      // Don't add the "read more" link to the node if the type has a hidden path.
      if (!($type->settings['hidden_path'] && !user_access('view hidden paths'))) {
        $node_title_stripped = strip_tags($node->title);
        $links['node-readmore'] = array(
          'title' => t('Read more<span class="element-invisible"> about @title</span>', array('@title' => $node_title_stripped)),
          'href' => 'node/' . $node->nid,
          'html' => TRUE,
          'attributes' => array('rel' => 'tag', 'title' => $node_title_stripped),
        );
      }
    }
    $node->content['links']['node'] = array(
      '#theme' => 'links__node__node',
      '#links' => $links,
      '#attributes' => array('class' => array('links', 'inline')),
    );

    // Allow modules to make their own additions to the node.
    module_invoke_all('node_view', $node, $view_mode, $langcode);
    module_invoke_all('entity_view', $node, 'node', $view_mode, $langcode);

    // Make sure the current display mode is stored if no module has already
    // populated the related key.
    $node->content += array('#view_mode' => $view_mode);
  }

  /**
   * Overrides DefaultEntityController::view().
   */
  public function view($nodes, $view_mode = 'full', $langcode = NULL, $page = NULL) {
    global $language_content;
    $langcode = $langcode ? $langcode : $language_content->langcode;

    $view = array();
    foreach ($nodes as $node) {
      /* @var Node $node */
      if (isset($node->preview) && ($node->preview == 'Preview')) {
        backdrop_set_message(t('This is a preview. Links within the page are disabled.'), 'warning', FALSE);
        backdrop_set_message(t('<strong>Changes are stored temporarily</strong>. Click <em>Save</em> to make your changes permanent, or click <em>Back to content editing</em> to make additional changes.'), 'warning', FALSE);
      }

      // Populate $node->content with a render() array.
      $this->buildContent($node, $view_mode, $langcode);

      $build = $node->content;
      // We don't need duplicate rendering info in node->content.
      unset($node->content);

      $build += array(
        '#theme' => 'node__' . $node->type . '__' . $view_mode,
        '#node' => $node,
        '#view_mode' => $view_mode,
        '#langcode' => $langcode,
        '#page' => $page,
      );

      // Add contextual links for this node, except when the node is already
      // being displayed on its own page. Modules may alter this behavior (for
      // example, to restrict contextual links to certain display modes) by
      // implementing hook_node_view_alter().
      if (!empty($node->nid) && !($view_mode == 'full' && node_is_page($node))) {
        $build['#contextual_links']['node'] = array('node', array($node->nid));
      }

      // Allow modules to modify the structured node.
      $type = 'node';
      backdrop_alter(array('node_view', 'entity_view'), $build, $type);
      $view[$type][$node->id()] = $build;
    }

    return $view;
  }
}
