// $Id $

(function($) {

/**
 * Toggle the visibility of a fieldset using smooth animations.
 */
Drupal.toggleVis = function (element, baseClass) {
  var $element = $(element);
  if ($element.is('.collapsed')) {
    var $content = $('> .'+ baseClass +'-wrapper', element).hide();
    $element
      .removeClass('collapsed')
      .trigger({ type: 'collapsed', value: false })
    $content.slideDown({
      duration: 'fast',
      easing: 'linear',
      complete: function () {
        Drupal.collapseScrollIntoView(element);
        element.animating = false;
      },
      step: function () {
        // Scroll the element into view.
        Drupal.collapseScrollIntoView(element);
      }
    });
  }
  else {
    $element.trigger({ type: 'collapsed', value: true });
    $('> .'+ baseClass +'-wrapper', element).slideUp('fast', function () {
      $element
        .addClass('collapsed')
      element.animating = false;
    });
  }
};
	
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

      // Turn the legend into a clickable link, but retain span.field-group-format-toggler
      // for CSS positioning.
      var $toggler = $('span.field-group-format-toggler', this);
      $('<span class="field-group-format-toggler element-invisible"></span>')
        .prependTo($toggler)
        .after(' ');

      // .wrapInner() does not retain bound events.
      var $link = $('<a class="field-group-format-title" href="#"></a>')
        .prepend($toggler.contents())
        .appendTo($toggler)
        .click(function () {
          var wrapper = $wrapper.get(0);
          // Don't animate multiple times.
          if (!wrapper.animating) {
            wrapper.animating = true;
            Drupal.toggleVis(wrapper, 'field-group-format');
          }
          return false;
        });
    });
  }
};

})(jQuery);