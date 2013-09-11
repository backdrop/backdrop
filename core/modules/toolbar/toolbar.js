(function ($) {

Backdrop.toolbar = Backdrop.toolbar || {};

/**
 * Attach toggling behavior and notify the overlay of the toolbar.
 */
Backdrop.behaviors.toolbar = {
  attach: function(context) {

    // Set the initial state of the toolbar.
    $('#toolbar', context).once('toolbar', Backdrop.toolbar.init);

    // Toggling toolbar drawer.
    $('#toolbar a.toggle', context).once('toolbar-toggle').click(function(e) {
      Backdrop.toolbar.toggle();
      // Allow resize event handlers to recalculate sizes/positions.
      $(window).triggerHandler('resize');
      return false;
    });
  }
};

/**
 * Retrieve last saved cookie settings and set up the initial toolbar state.
 */
Backdrop.toolbar.init = function() {
  // Retrieve the collapsed status from a stored cookie.
  var collapsed = $.cookie('Backdrop.toolbar.collapsed');

  // Expand or collapse the toolbar based on the cookie value.
  if (collapsed == 1) {
    Backdrop.toolbar.collapse();
  }
  else {
    Backdrop.toolbar.expand();
  }
};

/**
 * Collapse the toolbar.
 */
Backdrop.toolbar.collapse = function() {
  var toggle_text = Backdrop.t('Show shortcuts');
  $('#toolbar div.toolbar-drawer').addClass('collapsed');
  $('#toolbar a.toggle')
    .removeClass('toggle-active')
    .attr('title',  toggle_text)
    .html(toggle_text);
  $('body').removeClass('toolbar-drawer').css('paddingTop', Backdrop.toolbar.height());
  $.cookie(
    'Backdrop.toolbar.collapsed',
    1,
    {
      path: Backdrop.settings.basePath,
      // The cookie should "never" expire.
      expires: 36500
    }
  );
};

/**
 * Expand the toolbar.
 */
Backdrop.toolbar.expand = function() {
  var toggle_text = Backdrop.t('Hide shortcuts');
  $('#toolbar div.toolbar-drawer').removeClass('collapsed');
  $('#toolbar a.toggle')
    .addClass('toggle-active')
    .attr('title',  toggle_text)
    .html(toggle_text);
  $('body').addClass('toolbar-drawer').css('paddingTop', Backdrop.toolbar.height());
  $.cookie(
    'Backdrop.toolbar.collapsed',
    0,
    {
      path: Backdrop.settings.basePath,
      // The cookie should "never" expire.
      expires: 36500
    }
  );
};

/**
 * Toggle the toolbar.
 */
Backdrop.toolbar.toggle = function() {
  if ($('#toolbar div.toolbar-drawer').hasClass('collapsed')) {
    Backdrop.toolbar.expand();
  }
  else {
    Backdrop.toolbar.collapse();
  }
};

Backdrop.toolbar.height = function() {
  var height = $('#toolbar').outerHeight();
  // In IE, Shadow filter adds some extra height, so we need to remove it from
  // the returned height.
  if ($('#toolbar').css('filter').match(/DXImageTransform\.Microsoft\.Shadow/)) {
    height -= $('#toolbar').get(0).filters.item("DXImageTransform.Microsoft.Shadow").strength;
  }
  return height;
};

})(jQuery);
