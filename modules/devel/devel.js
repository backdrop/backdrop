(function ($) {

// Explain link in query log
Backdrop.behaviors.devel_explain = {
  attach: function() {
    $('a.dev-explain').click(function () {
      qid = $(this).attr("qid");
      cell = $('#devel-query-' + qid);
      $('.dev-explain', cell).load(Backdrop.settings.basePath + '?q=devel/explain/' + Backdrop.settings.devel.request_id + '/' + qid).show();
      $('.dev-placeholders', cell).hide();
      $('.dev-arguments', cell).hide();
      return false;
    });
  }
}

// Arguments link in query log
Backdrop.behaviors.devel_arguments = {
  attach: function() {
    $('a.dev-arguments').click(function () {
      qid = $(this).attr("qid");
      cell = $('#devel-query-' + qid);
      $('.dev-arguments', cell).load(Backdrop.settings.basePath + '?q=devel/arguments/' + Backdrop.settings.devel.request_id + '/' + qid).show();
      $('.dev-placeholders', cell).hide();
      $('.dev-explain', cell).hide();
      return false;
    });
  }
}

// Placeholders link in query log
Backdrop.behaviors.devel_placeholders = {
  attach: function() {
    $('a.dev-placeholders').click(function () {
      qid = $(this).attr("qid");
      cell = $('#devel-query-' + qid);
      $('.dev-explain', cell).hide();
      $('.dev-arguments', cell).hide();
      $('.dev-placeholders', cell).show();
      return false;
    });
  }
}

})(jQuery);
