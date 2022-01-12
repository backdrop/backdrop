(function ($) {

/**
 * Toggle the visibility of a fieldset using smooth animations.
 */
Backdrop.toggleFieldset = function (fieldset) {
  var $fieldset = $(fieldset);
  if ($fieldset.is('.collapsed')) {
    var $content = $('> .fieldset-wrapper', fieldset).hide();
    var insideDialog = Boolean($fieldset.parents('.ui-dialog-content').length);
    $fieldset
      .removeClass('collapsed')
      .find('> legend span.fieldset-legend-prefix').html(Backdrop.t('Hide'));
    $content.slideDown({
      duration: 'fast',
      easing: 'linear',
      complete: function () {
        $fieldset.trigger({ type: 'collapsed', value: false });
        $(window).triggerHandler('resize');
        Backdrop.optimizedResize.trigger();
        if (insideDialog === false) {
          Backdrop.collapseScrollIntoView(fieldset);
        }
        fieldset.animating = false;
      }
    });
  }
  else {
    $('> .fieldset-wrapper', fieldset).slideUp('fast', function () {
      $fieldset
        .addClass('collapsed')
        .find('> legend span.fieldset-legend-prefix').html(Backdrop.t('Show'));
      $fieldset.trigger({ type: 'collapsed', value: true });
      $(window).triggerHandler('resize');
      Backdrop.optimizedResize.trigger();
      fieldset.animating = false;
    });
  }
};

/**
 * Scroll a given fieldset into view as much as possible.
 */
Backdrop.collapseScrollIntoView = function (node) {
  var h = document.documentElement.clientHeight || document.body.clientHeight || 0;
  var offset = document.documentElement.scrollTop || document.body.scrollTop || 0;
  var posY = $(node).offset().top;
  if (posY + node.offsetHeight > h + offset) {
    if (node.offsetHeight > h) {
      node.scrollIntoView({behavior: "smooth"});
    }
    else {
      node.scrollIntoView({behavior: "smooth", block: "end"});
    }
  }
};

Backdrop.behaviors.collapse = {
  attach: function (context, settings) {
    var hasHash = location.hash && location.hash != '#' && $(window).find(location.hash).length;
    $('fieldset.collapsible', context).once('collapse', function () {
      var $fieldset = $(this);
      // Expand fieldset if there are errors inside, or if it contains an
      // element that is targeted by the URI fragment identifier.
      var anchor = hasHash ? ', ' + location.hash : '';
      if ($fieldset.find('.error' + anchor).length) {
        $fieldset.removeClass('collapsed');
      }

      var summary = $('<span class="summary"></span>');
      $fieldset.
        bind('summaryUpdated', function () {
          var text = $.trim($fieldset.backdropGetSummary());
          summary.html(text ? text : '');
        })
        .trigger('summaryUpdated');

      // Turn the legend into a clickable link, but retain span.fieldset-legend
      // for CSS positioning.
      var $legend = $('> legend .fieldset-legend', this);

      $('<span class="fieldset-legend-prefix element-invisible"></span>')
        .append($fieldset.hasClass('collapsed') ? Backdrop.t('Show') : Backdrop.t('Hide'))
        .prependTo($legend)
        .after(document.createTextNode(' '));

      // .wrapInner() does not retain bound events.
      var $link = $('<a class="fieldset-title" href="#"></a>')
        .prepend($legend.contents())
        .appendTo($legend)
        .click(function () {
          var fieldset = $fieldset.get(0);
          // Don't animate multiple times.
          if (!fieldset.animating) {
            fieldset.animating = true;
            Backdrop.toggleFieldset(fieldset);
          }
          return false;
        });

      $legend.append(summary);
    });
  }
};

})(jQuery);
