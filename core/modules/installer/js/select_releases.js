(function ($) {
  $(document).ready(function() {
    $('.installer-browser-releases-wrapper').hide();
    $('.installer-browser-selected-release').show();
    
    $('.installer-browser-show-releases-link').click(function() {
      var target = $(this).attr('rel');
      $('.installer-browser-release-' + target).show();
      $('.installer-browser-selected-release-' + target).hide();
    })
  });
})(jQuery);
