/**
 * show/hide password on user_register form.
 */
(function ($, Drupal, window, document, undefined) {
    $(document).ready(function () {
        $('#edit-show-password').click(function () {
            if ($('#edit-show-password').is(':checked')) {
                $('#edit-pass').replaceWith($('#edit-pass').clone().attr('type', 'text'));
            }
            else {
                $('#edit-pass').replaceWith($('#edit-pass').clone().attr('type', 'password'));
            }
        });
    });
}(jQuery, Drupal, this, this.document));