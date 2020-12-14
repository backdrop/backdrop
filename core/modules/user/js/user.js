(function ($) {

/**
 * Attach handlers to evaluate the strength of any password fields.
 */
Backdrop.behaviors.passwordStrength = {
  attach: function (context, settings) {
    $('input[data-password-strength]', context).once('password-strength', function () {
      var $passwordInput = $(this);
      var passwordStrengthSettings = $passwordInput.data('passwordStrength');
      var passwordMeter = '<span class="password-strength"><span class="password-strength-title">' + passwordStrengthSettings.labels.strengthTitle + ': </span><span class="password-strength-text" aria-live="assertive"></span><span class="password-indicator"><span class="indicator"></span></span></span>';
      $passwordInput.wrap('<span class="password-strength-wrapper"></span>').after(passwordMeter);
      var $innerWrapper = $passwordInput.parent();
      var $indicatorBar = $innerWrapper.find('.indicator');
      var $strengthText = $innerWrapper.find('.password-strength-text');
      var $strengthWrapper = $innerWrapper.find('.password-strength');

      // Check the password strength.
      var passwordCheck = function () {
        // Evaluate the password strength.
        var result = Backdrop.evaluatePasswordStrength($passwordInput.val(), passwordStrengthSettings);

        // Adjust the length of the strength indicator.
        $indicatorBar.css('width', result.strength + '%');

        // Update the strength indication text.
        $strengthText.html(passwordStrengthSettings.labels[result.level]);

        // Give a class to the strength.
        $strengthWrapper.attr('class', 'password-strength ' + result.level);
      };

      // Monitor keyup and blur events.
      // Blur must be used because a mouse paste does not trigger keyup.
      $passwordInput.on('keyup focus blur', passwordCheck).triggerHandler('blur');
    });
  }
};

/**
 * Attach handlers to evaluate the strength of any password fields.
 */
Backdrop.behaviors.passwordToggle = {
  attach: function (context, settings) {
    $('input[data-password-toggle]', context).once('password-toggle', function () {
      var $passwordInput = $(this);
      var passwordToggleSettings = $passwordInput.data('passwordToggle');
      var $passwordToggle = $('<a href="#" class="password-toggle" />').text(passwordToggleSettings.toggleShowTitle);

      // Use the same wrapper as the password strength indicator, if it's
      // already been added by the above behavior.
      if ($passwordInput.parent().is('.password-strength-wrapper')) {
        var $passwordWrapper = $passwordInput.parent().addClass('password-toggle-wrapper');
      }
      else {
        var $passwordWrapper = $passwordInput.wrap('<span class="password-toggle-wrapper"></span>').parent();
      }

      $passwordWrapper.addClass('password-hidden');
      $passwordInput.after($passwordToggle);

      var passwordToggle = function (e) {
        var showPassword = $passwordWrapper.is('.password-hidden');
        if (showPassword) {
          // Set the element to text and set the toggle to be "Hide".
          $passwordInput.attr('type', 'text');
          $passwordWrapper.removeClass('password-hidden').addClass('password-shown');
          $passwordToggle.text(passwordToggleSettings.toggleHideTitle);
        }
        else {
          // Set the element to password and set the toggle to be "Show".
          $passwordInput.attr('type', 'password');
          $passwordWrapper.removeClass('password-shown').addClass('password-hidden');
          $passwordToggle.text(passwordToggleSettings.toggleShowTitle);
        }
        e.preventDefault();
      };

      $passwordToggle.on('click', passwordToggle);
      if (passwordToggleSettings.toggleDefault === 'show') {
        $passwordToggle.triggerHandler('click');
      }

      // When submitting the form, convert back to a password field for the
      // sake of password managers.
      $($passwordInput[0].form).submit(function() {
        if ($passwordWrapper.is('.password-shown')) {
          $passwordToggle.triggerHandler('click');
        }
      });
    });

  }
};

/**
 * Attach handlers to password confirmation elements.
 */
Backdrop.behaviors.passwordConfirm = {
  attach: function (context, settings) {
    $('input[data-password-confirm]', context).once('password-confirm', function () {
      var $confirmInput = $(this);
      var $innerWrapper = $confirmInput.parent();
      var $outerWrapper = $innerWrapper.parent();
      var passwordConfirmSettings = $confirmInput.data('passwordConfirm');

      // Add the password confirmation layer.
      $outerWrapper.find('input.password-confirm').wrap('<span class="password-confirm-wrapper"></span>').after('<span class="password-match"><span class="password-match-title">' + passwordConfirmSettings.confirmTitle + '</span><span class="password-match-text"></span></span>').addClass('confirm-parent');
      var $passwordInput = $outerWrapper.find('input.password-field');
      var $matchResult = $outerWrapper.find('span.password-match');

      // Check that password and confirmation inputs match.
      var passwordCheckMatch = function () {

        if ($confirmInput.val()) {
          var success = $passwordInput.val() === $confirmInput.val();

          // Remove the previous styling if any exists.
          if (this.confirmClass) {
            $matchResult.removeClass(this.confirmClass);
          }

          // Fill in the success message and set the class accordingly.
          this.confirmClass = success ? 'match' : 'mismatch';
          $matchResult.addClass(this.confirmClass).find('.password-match-text').html(passwordConfirmSettings['confirm' + (success ? 'Success' : 'Failure')]);
        }
        else {
          this.confirmClass = 'empty';
          $matchResult.addClass(this.confirmClass);
        }
      };

      // Monitor keyup and blur events.
      // Blur must be used because a mouse paste does not trigger keyup.
      $passwordInput.on('keyup blur', passwordCheckMatch);
      $confirmInput.on('keyup blur', passwordCheckMatch).triggerHandler('blur');
    });
  }
};

/**
 * Evaluate the strength of a user's password.
 *
 * Returns the estimated strength and the relevant output message.
 */
Backdrop.evaluatePasswordStrength = function (password, settings) {
  var strength = 0;
  var level = 'empty';
  var data = settings.data;
  var config = settings.config;
  var username = data.username;
  var email = data.email;
  var hasLowercase = /[a-z]+/.test(password);
  var hasUppercase = /[A-Z]+/.test(password);
  var hasNumbers = /[0-9]+/.test(password);
  var hasPunctuation = /[^a-zA-Z0-9]+/.test(password);

  // If there is a username or email field on the page, compare the password to
  // that; otherwise use the value from the database.
  var usernameBox = $('input.username');
  if (usernameBox.length > 0) {
    username = usernameBox.val();
  }
  var emailBox = $('input.form-email');
  if (emailBox.length > 0) {
    email = emailBox.val();
  }

  // Calculate the number of unique character sets within a string.
  // Adapted from https://github.com/dropbox/zxcvbn.
  var cardinality = (hasLowercase * 26) + (hasUppercase * 26) + (hasNumbers * 10) + (hasPunctuation * 33);

  // Assign strength based on the level of entropy within the password, times
  // its length. Again, adapted from zxcvbn.
  strength = (Math.log(cardinality) / Math.log(2)) * password.length + 1;

  // Check if password is the same as the username or email.
  if (password !== '') {
    password = password.toLowerCase();
    username = username.toLowerCase();
    email = email.toLowerCase();

    if (password === username || password === email) {
      strength = 5;
    }
  }

  // Based on the strength, work out what text should be shown by the password strength meter.
  if (strength >= 90) {
    level = 'strong';
  }
  else if (strength > 70) {
    level = 'good';
  }
  else if (strength > 50) {
    level = 'fair';
  }
  else if (strength > 0) {
    level = 'weak';
  }

  // Cap at 100 and round to the nearest integer.
  strength = parseInt(Math.min(strength, 100));

  // Assemble the final message.
  return { strength: strength, level: level };
};

/**
 * Field instance settings screen: force the 'Display on registration form'
 * checkbox checked whenever 'Required' is checked.
 */
Backdrop.behaviors.fieldUserRegistration = {
  attach: function (context, settings) {
    var $checkbox = $('form#field-ui-field-edit-form input#edit-instance-settings-user-register-form');

    if ($checkbox.length) {
      $('input#edit-instance-required', context).once('user-register-form-checkbox', function () {
        $(this).bind('change', function (e) {
          if ($(this).prop('checked')) {
            $checkbox.prop('checked', true);
          }
        });
      });

    }
  }
};

})(jQuery);
