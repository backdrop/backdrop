// $Id $

(function($) {

/**
 * Drupal FieldGroup object.
 */
Drupal.FieldGroup = Drupal.FieldGroup || {};

/**
 * Create a link to animate on.
 */
Drupal.FieldGroup.createLink = function (element) {
  // Turn the legend into a clickable link, but retain span.field-group-format-toggler
  // for CSS positioning.
  var $toggler = $('span.field-group-format-toggler', element);
  $('<span class="field-group-format-toggler element-invisible"></span>')
    .prependTo($toggler)
    .after(' ');
  var $link = $('<a class="field-group-format-title" href="#"></a>');
  $link.prepend($toggler.contents())
    .appendTo($toggler);
  return $link;
};

/**
 * Behaviors.
 */
Drupal.behaviors.fieldGroup = {
  attach: function (context, settings) {
    $('div.collapsible', context).once('togglevis', function () {
    	
      var $wrapper = $(this);
      // Expand wrapper if there are errors inside, or if it contains an
      // element that is targeted by the uri fragment identifier. 
      var anchor = location.hash && location.hash != '#' ? ', ' + location.hash : '';
      if ($('.error' + anchor, $wrapper).length) {
        $wrapper.removeClass('collapsed');
      }
      
      var $link = Drupal.FieldGroup.createLink(this);

      // .wrapInner() does not retain bound events.
      $link.click(function () {
          var wrapper = $wrapper.get(0);
          // Don't animate multiple times.
          if (!wrapper.animating) {
            wrapper.animating = true;
            $('> .'+ baseClass +'-wrapper', element).toggle('blind', {}, 500);
            element.animating = false;
          }
          return false;
        });
    });
    
	var accordionWrapper = $('h3.accordion', context).parent();
	accordionWrapper.once('accordion', function () {
      $(this).accordion({ autoHeight: false });
    });
  }
};

})(jQuery);