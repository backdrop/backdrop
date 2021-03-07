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
(function (document, Backdrop) {

var liveElement;

/**
 * Builds a div element with the aria-live attribute and attaches it
 * to the DOM.
 */
Backdrop.behaviors.backdropAnnounce = {
  attach: function (settings, context) {
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

/**
 * Triggers audio UAs to read the supplied text.
 *
 * @param {String} text
 *   - A string to be read by the UA.
 *
 * @param {String} priority
 *   - A string to indicate the priority of the message. Can be either
 *   'polite' or 'assertive'. Polite is the default.
 *
 * Use Backdrop.announce to indicate to screen reader users that an element on
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
 *
 * @see http://www.w3.org/WAI/PF/aria-practices/#liveprops
 */
Backdrop.announce = function (text, priority) {
  if (typeof text === 'string') {
    // Clear the liveElement so that repeated strings will be read.
    liveElement.innerHTML = '';
    // Set the busy state to true until the node changes are complete.
    liveElement.setAttribute('aria-busy', 'true');
    // Set the priority to assertive, or default to polite.
    liveElement.setAttribute('aria-live', (priority === 'assertive') ? 'assertive' : 'polite');
    // Print the text to the live region.
    liveElement.innerHTML = Backdrop.checkPlain(text);
    // The live text area is updated. Allow the AT to announce the text.
    liveElement.setAttribute('aria-busy', 'false');
  }
};

}(document, Backdrop));

