(function ($) {
  $(document).ready(function() {
    $('.project-browser-releases-wrapper').hide();
    $('.project-browser-selected-release').show();
    
    $('.project-browser-show-releases-link').click(function() {
      var target = $(this).attr('rel');
      $('.project-browser-release-' + target).show();
      $('.project-browser-selected-release-' + target).hide();
    })
  });
})(jQuery);
