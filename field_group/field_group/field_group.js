// $Id $

(function($) {

/**
 * Drupal FieldGroup object.
 */
Drupal.FieldGroup = Drupal.FieldGroup || {};
Drupal.FieldGroup.Effects = Drupal.FieldGroup.Effects || {};

/**
 * Implements Drupal.FieldGroup.processHook().
 */
Drupal.FieldGroup.Effects.processAccordion = {
  execute: function (context, settings) {
    var accordionWrapper = $('div.field-group-accordion-wrapper', context);
    accordionWrapper.accordion({ autoHeight: false });
  }
};

/**
 * Implements Drupal.FieldGroup.processHook().
 * 
 * TODO clean this up meaning check if this is really 
 *      necessary.
 */
Drupal.FieldGroup.Effects.processDiv = {
  execute: function (context, settings) {

    $('div.collapsible', context).each(function() {
      var $wrapper = $(this);

      // Turn the legend into a clickable link, but retain span.field-group-format-toggler
      // for CSS positioning.
      var $toggler = $('span.field-group-format-toggler', $wrapper);
      $('<span class="field-group-format-toggler element-invisible"></span>')
        .prependTo($toggler).after(' ');
      var $link = $('<a class="field-group-format-title" href="#"></a>');
      $link.prepend($toggler.contents()).appendTo($toggler);
      
      // .wrapInner() does not retain bound events.
      $link.click(function () {
        var wrapper = $wrapper.get(0);
        // Don't animate multiple times.
        if (!wrapper.animating) {
          wrapper.animating = true;
          $('> .field-group-format-wrapper', wrapper).toggle('blind', {}, 500);
          wrapper.animating = false;
        }
        return false;
      });
      
    });
  }
};

/**
 * Behaviors.
 */
Drupal.behaviors.fieldGroup = {
  attach: function (context, settings) {
    $('.field-group-content-wrapper', context).once('fieldgroup.effects', function () {
      // Execute all of them.
      $.each(Drupal.FieldGroup.Effects, function (func) {
        // We check for a wrapper function in Drupal.field_group as 
        // alternative for dynamic string function calls.
        if (settings.field_group[func.toLowerCase().replace("process", "")] != undefined && $.isFunction(this.execute)) {
          this.execute(context, settings);
        }
      });
    });
  }
};

})(jQuery);