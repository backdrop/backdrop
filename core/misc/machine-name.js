(function ($) {

/**
 * Attach the machine-readable name form element behavior.
 */
Backdrop.behaviors.machineName = {
  /**
   * Attaches the behavior.
   *
   * @param settings.machineName
   *   A list of elements to process, keyed by the HTML ID of the form element
   *   containing the human-readable value. Each element is an object defining
   *   the following properties:
   *   - target: The HTML ID of the machine name form element.
   *   - suffix: The HTML ID of a container to show the machine name preview in
   *     (usually a field suffix after the human-readable name form element).
   *   - label: The label to show for the machine name preview.
   *   - replace_pattern: A regular expression (without modifiers) matching
   *     disallowed characters in the machine name; e.g., '[^a-z0-9]+'.
   *   - replace: A character to replace disallowed characters with; e.g., '_'
   *     or '-'.
   *   - standalone: Whether the preview should stay in its own element rather
   *     than the suffix of the source element.
   *   - field_prefix: The #field_prefix of the form element.
   *   - field_suffix: The #field_suffix of the form element.
   */
  attach: function (context, settings) {
    var self = this;
    var source_id, options, machine, eventData;
    var $context = $(context);

     function clickEditHandler(e) {
       var data = e.data;
       e.preventDefault();
       data.$wrapper.show();
       data.$target.focus();
       data.$suffix.hide();
       data.$source.unbind('.machineName');
     }

     function machineNameHandler(e) {
       var data = e.data;
       self.transliterate($(e.target).val(), data.options).done(function (machine) {
         showMachineName(machine, data);
       });
     }

     function showMachineName(machine, data) {
       // Respect maximum length
         machine_short = machine.substr(0, data.options.maxlength);
         // Set the machine name to the transliterated value.
         if (machine_short !== '') {
           if (machine_short !== data.options.replace) {
             data.$target.val(machine_short);
             data.$preview.html(data.options.field_prefix + Backdrop.checkPlain(machine_short) + data.options.field_suffix);
           }
           data.$suffix.show();
         }
         else {
           data.$suffix.hide();
           data.$target.val(machine_short);
           data.$preview.empty();
         }
     }

     function appendMachineName(machine, data) {
       
     }

     for (source_id in settings.machineName) {
       if (settings.machineName.hasOwnProperty(source_id)) {
         options =  settings.machineName[source_id];

         var $source = $context.find(source_id).addClass('machine-name-source');
         var $target = $context.find(options.target).addClass('machine-name-target');
         var $suffix = $context.find(options.suffix);
         var $wrapper = $target.closest('.form-item');
         // All elements have to exist.
         if (!$source.length || !$target.length || !$suffix.length || !$wrapper.length) {
           return;
         }
         // Skip processing upon a form validation error on the machine name.
         if ($target.hasClass('error')) {
           return;
         }
         // Figure out the maximum length for the machine name.
         options.maxlength = $target.attr('maxlength');
         // Hide the form item container of the machine name form element.
         $wrapper.hide();
         // Determine the initial machine name value. Unless the machine name form
         // element is disabled or not empty, the initial default value is based on
         // the human-readable form element value.
         var field_needs_transliteration = false;
         if ($target.is(':disabled') || $target.val() !== '') {
           machine = $target.val();
         }
         else {
           machine = $source.val();
           field_needs_transliteration = true;
         }
         // Append the machine name preview to the source field.
         var $preview = $('<span class="machine-name-value">' + options.field_prefix + Backdrop.checkPlain(machine) + options.field_suffix + '</span>');
         $suffix.empty();
         if (options.label) {
           $suffix.append(' ').append('<span class="machine-name-label">' + options.label + ':</span>');
         }
         $suffix.append(' ').append($preview);

         // If the machine name cannot be edited, stop further processing.
         if ($target.is(':disabled')) {
           return;
         }

        eventData = {
          $source: $source,
          $target: $target,
          $suffix: $suffix,
          $wrapper: $wrapper,
          $preview: $preview,
          options: options
        };

        if (field_needs_transliteration) {
          self.transliterate(machine, options).done(function (machine) {
            showMachineName(machine, eventData);
          });
        }

        // If it is editable, append an edit link.
        var $link = $('<span class="admin-link"><a href="#">' + Backdrop.t('Edit') + '</a></span>').bind('click', eventData, clickEditHandler);
        $suffix.append(' ').append($link);

        // Preview the machine name in realtime when the human-readable name
        // changes, but only if there is no machine name yet; i.e., only upon
        // initial creation, not when editing.
        if ($target.val() === '') {
          $source.bind('keyup.machineName change.machineName', eventData, machineNameHandler)
          // Initialize machine name preview.
          .keyup();
        }
      }
    }
  },

  /**
   * Transliterate a human-readable name to a machine name.
   *
   * @param source
   *   A string to transliterate.
   * @param settings
   *   The machine name settings for the corresponding field, containing:
   *   - replace: A character to replace disallowed characters with; e.g., '_'
   *     or '-'.
   *   - maxlength: The maximum length of the machine name.
   *
   * @return
   *   The transliterated source string.
   */
  transliterate: function (source, settings) {
    return $.ajax({
      url: Backdrop.settings.basePath + "machine_name/transliterate/" + source.toLowerCase() + "/" + settings.replace,
      dataType: "json"
    }); 
  }
};

})(jQuery);
