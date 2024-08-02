/**
 * Adds an HTML element and method to trigger audio UAs to read system messages.
 *
 * Use Backdrop.announce() to indicate to screen reader users that an element on
 * the page has changed state. For instance, if clicking a link loads 10 more
 * items into a list, one might announce the change like this.
 * $('#search-list')
 *   .on('itemInsert', function (event, data) {
 *     // Insert the new items.
 *     $(data.container.el).append(data.items.el);
 *     // Announce the change to the page contents.
 *     Backdrop.announce(Backdrop.t('@count items added to @container',
 *       {'@count': data.items.length, '@container': data.container.title}
 *     ));
 *   });
 */
(function (Backdrop) {
  var liveElement;
  var announcements = [];
  Backdrop.behaviors.backdropAnnounce = {
    attach: function attach(context) {
      if (!liveElement) {
        liveElement = document.createElement('div');
        liveElement.id = 'backdrop-live-announce';
        liveElement.className = 'element-invisible';
        liveElement.setAttribute('aria-live', 'polite');
        liveElement.setAttribute('aria-busy', 'false');
        document.body.appendChild(liveElement);
      }
    }
  };

  function announce() {
    var text = [];
    var priority = 'polite';
    var announcement;
    var il = announcements.length;

    for (var i = 0; i < il; i++) {
      announcement = announcements.pop();
      text.unshift(announcement.text);

      if (announcement.priority === 'assertive') {
        priority = 'assertive';
      }
    }

    if (text.length) {
      liveElement.innerHTML = '';
      liveElement.setAttribute('aria-busy', 'true');
      liveElement.setAttribute('aria-live', priority);
      liveElement.innerHTML = text.join('\n');
      liveElement.setAttribute('aria-busy', 'false');
    }
  }

  /**
   * Triggers audio UAs to read the supplied text.
   *
   * The aria-live region will only read the text that currently populates its
   * text node. Replacing text quickly in rapid calls to announce results in
   * only the text from the most recent call to Backdrop.announce() being
   * read. By wrapping the call to announce in a debounce function, we allow for
   * time for multiple calls to Backdrop.announce() to queue up their
   * messages. These messages are then joined and append to the aria-live region
   * as one text node.
   *
   * @param string text
   *   A string to be read by the UA.
   * @param string priority
   *   A string to indicate the priority of the message. Can be either
   *   'polite' or 'assertive'.
   *
   * @return function
   *   The return of the call to debounce.
   *
   * @see http://www.w3.org/WAI/PF/aria-practices/#liveprops
   * @since 1.18.2 Method added.
   */
    Backdrop.announce = function (text, priority) {
    announcements.push({
      text: text,
      priority: priority
    });
    return Backdrop.debounce(announce, 300)();
  };
})(Backdrop);
