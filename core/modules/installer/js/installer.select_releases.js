/**
 * @file
 * Hides the releases radio elements if only one release is available.
 */
Backdrop.behaviors.installerSelectReleases = {
  attach: function (context, settings) {
    $('.installer-browser-releases-wrapper').hide();
    $('.installer-browser-selected-release').show();
    
    $('.installer-browser-show-releases-link').click(function() {
      var target = $(this).attr('rel');
      $('.installer-browser-release-' + target).show();
      $('.installer-browser-selected-release-' + target).hide();
    })
  }
}
