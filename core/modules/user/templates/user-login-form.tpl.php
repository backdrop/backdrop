<?php
/**
 * @file
 * Renders the user login form.
 *
 * Use render($user_login_form) to print all form items, or print a subset
 * such as render($user_login_form['name']). Always call
 * backdrop_render_children($user_login_form) at the end in order to print all remaining items.
 *
 * Available variables:
 *   - $user_login_form: An array of form items. Use render() to print them.
 *
 * @ingroup themeable
 */
?>
<?php print backdrop_render_children($user_login_form); ?>
