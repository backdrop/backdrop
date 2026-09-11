/**
 * @file
 * Attaches behaviors for the User module.
 */
(function ($) {

Backdrop.behaviors.userFieldsetSummaries = {
  // Provide the vertical tab summaries.
  attach: function (context) {
    var $context = $(context);

    // Account settings.
    $context.find('fieldset#edit-account-settings').backdropSetSummary(function() {
      var vals = [];

      // Status.
      var status = $context.find('input[name="status"]:checked').parent().find('label').text();
      vals.push(status.trim());

      // Roles.
      var roles = [];
      var $checkedRoles = $context.find('input[name^="roles"]:not([disabled]):checked');
      $checkedRoles.each(function() {
        roles.push($(this).parent().find('label').text().trim());
      });
      if (roles.length) {
        var rolesText = Backdrop.t('Roles:') + ' ' + roles.join(', ');
      }
      else {
        var rolesText = Backdrop.t('No roles');
      }
      vals.push(rolesText);

      return Backdrop.checkPlain(vals.join(', '));
    });

    // Personalization.
    $context.find('fieldset#edit-personalization').backdropSetSummary(function() {
      var vals = [];

      // Signature.
      var $signature = $context.find('textarea[name="signature[value]"]');
      if ($signature.length && $signature.val().length) {
        var signatureText = Backdrop.t('Signature');
      }
      else {
        var signatureText = Backdrop.t('No signature');
      }
      vals.push(signatureText);

      // Picture.
      var $pictureNew = $context.find('input[name="files[picture_upload]"]');
      var $pictureExisting = $context.find('.user-picture');
      if (($pictureNew.length && $pictureNew.val().length) || $pictureExisting.length) {
        var pictureText = Backdrop.t('Picture');
      }
      else {
        var pictureText = Backdrop.t('No picture');
      }
      vals.push(pictureText);

      return Backdrop.checkPlain(vals.join(', '));
    });

    // Region and language.
    $context.find('fieldset#edit-region-language').backdropSetSummary(function() {
      var vals = [];

      // Timezone.
      var $timezone = $context.find('select[name="timezone"]');
      if ($timezone.length && $timezone.val().length) {
        var timezoneText = Backdrop.t('Time zone:') + ' ' + $timezone.find(':selected').text().trim();
        vals.push(timezoneText);
      }

      // Language.
      var $language = $context.find('input[name="language"]:checked');
      if ($language.length) {
        var languageText = Backdrop.t('Language:') + ' ' + $language.parent().find('label').text().trim();
        vals.push(languageText);
      }

      return Backdrop.checkPlain(vals.join(', '));
    });

    // Contact form.
    $context.find('fieldset#edit-contact').backdropSetSummary(function() {
      if ($context.find('input[name="contact"]:checked').length) {
        return Backdrop.t('Enabled');
      }
      else {
        return Backdrop.t('Disabled');
      }
    });
  }
};

})(jQuery);
