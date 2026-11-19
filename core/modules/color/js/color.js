/**
 * @file
 * Attaches the behaviors for the Color module.
 */

(function ($) {

if (typeof Backdrop.featureDetect !== 'object') {
  Backdrop.featureDetect = {};
}

/**
 * Test to see if browser has ability to use input with a type of color
 */
Backdrop.featureDetect.inputTypeColor = function() {
  var $body = $('body'),
      $inputTypeColor = $('<div style="width: 0; height: 0; overflow: hidden;"><input type="color" id="featureDetectInputTypeColor" name="featureDetectInputTypeColor"/></div>');

  if ($body.hasClass('has-input-type-color')) {
    return true;
  }
  else if ($body.hasClass('no-input-type-color')) {
    return false;
  }
  else {
    // Run our test by adding a color field into the DOM and checking it's type
    $body.append($inputTypeColor);
    // By default browsers that don't understand color fields will fall back to text
    if (document.getElementById('featureDetectInputTypeColor').type === 'text') {
      $body.addClass('no-input-type-color');
      $inputTypeColor.remove();
      return false;
    }
    else {
      $body.addClass('has-input-type-color');
      $inputTypeColor.remove();
      return true;
    }
  }
};

Backdrop.behaviors.color = {
  attach: function (context) {
    var settings = document.getElementById('edit-scheme').dataset;
    var schemes = JSON.parse(settings.colorSchemes);
    var customSchemes = JSON.parse(settings.colorCustomSchemes);

    // Hide the custom name field on first load.
    var $nameField = $('input[data-custom-name]').parent();
    $nameField.hide();

    // This behavior attaches by ID, so is only valid once on a page.
    var form = $('#system-theme-settings .color-form', context).once('color');
    if (form.length === 0) {
      return;
    }

    // Set up colorScheme selector.
    $('#edit-scheme', form).on('change', function () {
      var schemeName = this.value;
      if (schemeName !== '' && schemes[schemeName]) {
        // Get colors of active scheme.
        var colors = schemes[schemeName];
        for (var fieldName in colors) {
          if (colors.hasOwnProperty(fieldName)) {
            var input = $("input[data-color-name='" + fieldName + "']");
            if (input.val() && input.val() != colors[fieldName]) {
              input.val(colors[fieldName]);
            }
          }
        }
        updatePreview();
        $nameField.hide();
      }
      else {
        $nameField.show();
      }
    });

    $('input[data-color-name]').on('change', function () {
      var schemeName =  document.getElementById('edit-scheme').value;
      var isCustomScheme =  $.inArray(schemeName, customSchemes) > -1;
      var key = this.dataset.colorName;
      if (schemeName === '') {
        $nameField.show();
        resetScheme();
      }
      else if (this.value !== schemes[schemeName][key] && !isCustomScheme) {
        $nameField.show();
        resetScheme();
      }
      else {
        $nameField.hide();
      }
      updatePreview();
    });

    // Setup the preview.
    $('#system-theme-settings').addClass('has-preview').after(settings.colorPreviewMarkup);
    // Wait for the iframe to be loaded before attempting to apply the preview
    // settings for the first time.
    $(window).on('load', function () {
      updatePreview();
    });

    /**
     * Saves the current form values and refreshes the preview.
     */
    function updatePreview() {
      // Save the form values.
      var values = {
        scheme: $('#edit-scheme').val(),
        palette: {}
      };
      values['scheme'] = $('#edit-scheme').val();
      $('input[data-color-name]').each(function () {
        values['palette'][this.dataset.colorName] = this.value;
      });

      $.ajax({
        type: 'POST',
        dataType: 'json',
        url : Backdrop.settings.basePath + 'color/save_preview_settings/' + settings.colorThemeName + '/?token=' + settings.colorPreviewToken,
        data: values,
        complete : function(response) {
          // Refresh the preview.
          var preview = document.getElementById('preview').contentDocument.location.reload(true);
        }
      });
    }

    /**
     * Resets the color scheme selector.
     */
    function resetScheme() {
      $('#edit-scheme', form).each(function () {
        this.selectedIndex = this.options.length - 1;
      });
    }

  }
};

})(jQuery);
