<?php
/**
 * @file
 * User page callbacks for the book module.
 */

/**
 * Menu callback: Prints a listing of all books.
 *
 * @return string
 *   A HTML-formatted string with the listing of all books content.
 *
 * @see book_menu()
 */
function book_render() {
  $book_list = array();
  foreach (book_get_books() as $book) {
    $book_list[] = l($book['title'], $book['href'], $book['options']);
  }

  return theme('item_list', array('items' => $book_list));
}

/**
 * Form constructor to confirm removal of a node from a book.
 *
 * @param Node $node
 *   The node to delete.
 *
 * @see book_remove_form_submit()
 * @see book_menu()
 * @ingroup forms
 */
function book_remove_form($form, &$form_state, Node $node) {
  $form['#node'] = $node;
  $title = array('%title' => $node->title);

  if ($node->book['has_children']) {
    $description = t('%title has associated child pages, which will be relocated automatically to maintain their connection to the book. To recreate the hierarchy (as it was before removing this page), %title may be added again using the Outline tab, and each of its former child pages will need to be relocated manually.', $title);
  }
  else {
    $description = t('%title may be added to hierarchy again from the Book outline tab on the edit form.', $title);
  }

  return confirm_form($form, t('Are you sure you want to remove %title from the book hierarchy?', $title), 'node/' . $node->nid, $description, t('Remove'));
}

/**
 * Form submission handler for book_remove_form().
 */
function book_remove_form_submit($form, &$form_state) {
  $node = $form['#node'];
  if (_book_node_is_removable($node)) {
    menu_link_delete($node->book['mlid']);
    db_delete('book')
      ->condition('nid', $node->nid)
      ->execute();
    cache_clear_all($node->nid, 'cache_entity_node');
    backdrop_set_message(t('The post has been removed from the book.'));
  }
  $form_state['redirect'] = 'node/' . $node->nid;
}
