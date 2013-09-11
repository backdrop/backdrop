(function ($) {

$.datepicker.regional['backdrop-locale'] = {
  closeText: Backdrop.t('Done'),
  prevText: Backdrop.t('Prev'),
  nextText: Backdrop.t('Next'),
  currentText: Backdrop.t('Today'),
  monthNames: [
    Backdrop.t('January'),
    Backdrop.t('February'),
    Backdrop.t('March'),
    Backdrop.t('April'),
    Backdrop.t('May'),
    Backdrop.t('June'),
    Backdrop.t('July'),
    Backdrop.t('August'),
    Backdrop.t('September'),
    Backdrop.t('October'),
    Backdrop.t('November'),
    Backdrop.t('December')
  ],
  monthNamesShort: [
    Backdrop.t('Jan'),
    Backdrop.t('Feb'),
    Backdrop.t('Mar'),
    Backdrop.t('Apr'),
    Backdrop.t('May'),
    Backdrop.t('Jun'),
    Backdrop.t('Jul'),
    Backdrop.t('Aug'),
    Backdrop.t('Sep'),
    Backdrop.t('Oct'),
    Backdrop.t('Nov'),
    Backdrop.t('Dec')
  ],
  dayNames: [
    Backdrop.t('Sunday'),
    Backdrop.t('Monday'),
    Backdrop.t('Tuesday'),
    Backdrop.t('Wednesday'),
    Backdrop.t('Thursday'),
    Backdrop.t('Friday'),
    Backdrop.t('Saturday')
  ],
  dayNamesShort: [
    Backdrop.t('Sun'),
    Backdrop.t('Mon'),
    Backdrop.t('Tue'),
    Backdrop.t('Wed'),
    Backdrop.t('Thu'),
    Backdrop.t('Fri'),
    Backdrop.t('Sat')
  ],
  dayNamesMin: [
    Backdrop.t('Su'),
    Backdrop.t('Mo'),
    Backdrop.t('Tu'),
    Backdrop.t('We'),
    Backdrop.t('Th'),
    Backdrop.t('Fr'),
    Backdrop.t('Sa')
  ],
  dateFormat: Backdrop.t('mm/dd/yy'),
  firstDay: Backdrop.settings.jqueryuidatepicker.firstDay,
  isRTL: Backdrop.settings.jqueryuidatepicker.rtl
};
$.datepicker.setDefaults($.datepicker.regional['backdrop-locale']);

})(jQuery);
