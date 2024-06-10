
(function($) {

/**
 * Backdrop FieldGroup object.
 */
Backdrop.FieldGroup = Backdrop.FieldGroup || {};
Backdrop.FieldGroup.Effects = Backdrop.FieldGroup.Effects || {};

/**
 * Behaviors.
 */
Backdrop.behaviors.fieldGroup = {
  attach: function (context, settings) {
    settings.field_group = settings.field_group || Backdrop.settings.field_group;
    if (settings.field_group == undefined) {
      return;
    }

    // Execute all of them.
    $.each(Backdrop.FieldGroup.Effects, function (func) {
      // We check for a wrapper function in Backdrop.field_group as
      // alternative for dynamic string function calls.
      var type = func.toLowerCase().replace("process", "");
      if (settings.field_group[type] != undefined && typeof this.execute === "function") {
        this.execute(context, settings, settings.field_group[type]);
      }
    });

  }
};

})(jQuery);
