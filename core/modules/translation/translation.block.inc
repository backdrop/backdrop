<?php
/**
 * @file
 * Extend the NodeBlock class to find the appropriate translation of a node,
 * if available.
 *
 * This class overrides the prepare method of Block to replace the nid in the
 * settings with the nid of the (node) translation of the content just before
 * it is being rendered (primarily for the getTitle and getContent methods).
 */

class TranslatedNodeBlock extends NodeBlock {
  protected $already_replaced = FALSE;

  function getAdminTitle() {
    return t('Existing content (translated)');
  }

  function prepare() {
	  if (!$this->already_replaced) {
		  $this->settings['nid'] = $this->findTranslation($this->settings['nid']);
		  $this->already_replaced = TRUE;
	  }
  }

  function findTranslation($nid) {
    global $language;

    $node = node_load($nid);

    if (empty($node->tnid) || empty($node->langcode)) {
      return $nid;
    }

    $translations = translation_node_get_translations($node->tnid);

    foreach ($translations as $code => $translation) {
      if ($code == $language->langcode)
        return $translation->nid;
    }

    return $nid;
  }
}
