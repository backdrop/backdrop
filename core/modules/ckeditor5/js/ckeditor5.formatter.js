/**
 * @file
 *
 * Contains HTML exporting and formatting code for CKEditor 5 module.
 */
(function (Backdrop) {
  "use strict";

  /**
   * A simple (and naive) HTML code formatter that returns a formatted HTML
   * markup that can be easily parsed by human eyes. It beautifies the HTML code
   * by adding new lines between elements that behave like block elements
   * (https://developer.mozilla.org/en-US/docs/Web/HTML/Block-level_elements
   * and a few more like `tr`, `td`, and similar ones) and inserting indents for
   * nested content.
   *
   * WARNING: This function works only on a text that does not contain any
   * indentations or new lines. Calling this function on the already formatted
   * text will damage the formatting.
   *
   * @param {String} input
   *   An HTML string to format.
   * @param {String} input
   *   A chunk of unformatted HTML.
   * @returns {String}
   *   The same HTML formatted for easier readability and diffing.
   */
  Backdrop.ckeditor5.formatHtml = function(input) {
    const htmlFormatter = new BackdropHtmlFormatter();
    return htmlFormatter.formatHtml(input);
  };

  /**
   * Convert an Element into an HTML string, with attributes properly escaped.
   *
   * In some cases, the value returned from this function may want to be
   * formatted using Backdrop.ckeditor5.formatHtml().
   *
   * @param {Element|DocumentFragment} element
   *   The DOM Element from which HTML should be exported.
   * @returns {String}
   *   An unformatted HTML string.
   */
  Backdrop.ckeditor5.elementGetHtml = function(element) {
    const htmlBuilder = new BackdropHtmlBuilder();
    htmlBuilder.appendNode(element);
    return htmlBuilder.build();
  };

  /**
   * HTML formatting utility for use by Backdrop's CKEditor 5 integration.
   *
   * This formatter is based off CKEditor 5 source code for HTML Source Editing.
   *
   * See https://github.com/ckeditor/ckeditor5/blob/master/packages/ckeditor5-source-editing/src/utils/formathtml.ts
   *
   * Eventually the code in this file may not be necessary, in the event that
   * CKEditor 5 allows public API access to the HTML formatter.
   *
   * See https://github.com/ckeditor/ckeditor5/issues/8668
   */
  class BackdropHtmlFormatter {
    /**
     * Constructs a new object.
     */
    constructor() {
      // A list of block-like elements around which the new lines should be
      // inserted, and within which the indentation of their children should be
      // increased. The list is partially based on
      // https://developer.mozilla.org/en-US/docs/Web/HTML/Block-level_elements
      // that contains a full list of HTML block-level elements.
      // A void element is an element that cannot have any child:
      // https://html.spec.whatwg.org/multipage/syntax.html#void-elements.
      // Note that <pre> element is not listed on this list to avoid breaking
      // whitespace formatting.
      // Note that <br> element is not listed and handled separately so no
      // additional white spaces are injected.
      this.elementsToFormat = [
        { name: 'address', isVoid: false },
        { name: 'article', isVoid: false },
        { name: 'aside', isVoid: false },
        { name: 'blockquote', isVoid: false },
        { name: 'details', isVoid: false },
        { name: 'dialog', isVoid: false },
        { name: 'dd', isVoid: false },
        { name: 'div', isVoid: false },
        { name: 'dl', isVoid: false },
        { name: 'dt', isVoid: false },
        { name: 'fieldset', isVoid: false },
        { name: 'figcaption', isVoid: false },
        { name: 'figure', isVoid: false },
        { name: 'footer', isVoid: false },
        { name: 'form', isVoid: false },
        { name: 'h1', isVoid: false },
        { name: 'h2', isVoid: false },
        { name: 'h3', isVoid: false },
        { name: 'h4', isVoid: false },
        { name: 'h5', isVoid: false },
        { name: 'h6', isVoid: false },
        { name: 'header', isVoid: false },
        { name: 'hgroup', isVoid: false },
        { name: 'hr', isVoid: true },
        { name: 'li', isVoid: false },
        { name: 'main', isVoid: false },
        { name: 'nav', isVoid: false },
        { name: 'ol', isVoid: false },
        { name: 'p', isVoid: false },
        { name: 'section', isVoid: false },
        { name: 'table', isVoid: false },
        { name: 'tbody', isVoid: false },
        { name: 'td', isVoid: false },
        { name: 'th', isVoid: false },
        { name: 'thead', isVoid: false },
        { name: 'tr', isVoid: false },
        { name: 'ul', isVoid: false }
      ];
    }

    /**
     * Format an HTML string with indentation and newlines for readability.
     * @param {String} input
     *   A chunk of unformatted HTML.
     * @returns {String}
     *   The same HTML formatted for easier readability and diffing.
     */
    formatHtml(input) {
      const elementNamesToFormat = this.elementsToFormat.map(element => element.name).join('|');

      // It is not the fastest way to format the HTML markup but the performance
      // should be good enough.
      const lines = input
        // Add new line before and after `<tag>` and `</tag>`. It may separate
        // individual elements with two new lines, but this will be fixed below.
        .replace(new RegExp( `</?(${ elementNamesToFormat })( .*?)?>`, 'g' ), '\n$&\n')
        // Keep `<br>`s at the end of line to avoid adding additional whitespaces
        // before `<br>`.
        .replace(/<br[^>]*>/g, '$&\n')
        // Divide input string into lines, which start with either an opening tag,
        // a closing tag, or just a text.
        .split('\n');

      let indentCount = 0;

      return lines
        .filter((line) => { return line.trim().length })
        .map((line) => {
          if (this._isNonVoidOpeningTag(line)) {
            return this._indentLine(line, indentCount++);
          }

          if (this._isClosingTag(line)) {
            return this._indentLine(line, --indentCount);
          }

          return this._indentLine(line, indentCount);
        })
        .join('\n');
    }

    /**
     * Checks if an argument is an opening tag of a non-void element.
     *
     * @param {String} line
     *   String to check.
     */
    _isNonVoidOpeningTag(line) {
      return this.elementsToFormat.some((element) => {
        if (element.isVoid) {
          return false;
        }

        if (!new RegExp('<' + element.name  + '( .*?)?>').test(line)) {
          return false;
        }

        return true;
      });
    }

    /**
     * Checks if an argument is a closing tag.
     *
     * @param {String} line
     *   String to check.
     */
    _isClosingTag(line) {
      return this.elementsToFormat.some((element) => {
        return new RegExp('</' + element.name + '>').test(line);
      });
    }

    /**
     * Indents a line by a specified number of characters.
     *
     * @param {String} line
     *   Line to indent.
     * @param {Number} indentCount
     *   Number of characters to use for indentation.
     * @param {String} indentChar
     *   Indentation character(s). 4 spaces by default.
     */
    _indentLine(line, indentCount, indentChar = '    ' ) {
      // More about Math.max() here in https://github.com/ckeditor/ckeditor5/issues/10698.
      return indentChar.repeat(Math.max(0, indentCount)) + line.trim();
    }
  }

  /**
   * HTML builder that converts document fragments into strings.
   *
   * Escapes ampersand characters (`&`) and angle brackets (`<` and `>`) when
   * transforming data to HTML. This is required because filter_xss() fails to
   * parse element attributes values containing unescaped HTML entities.
   */
  class BackdropHtmlBuilder {
    /**
     * Constructs a new object.
     */
    constructor() {
      this.chunks = [];
      // @see https://html.spec.whatwg.org/multipage/syntax.html#elements-2
      this.selfClosingTags = [
        'area',
        'base',
        'br',
        'col',
        'embed',
        'hr',
        'img',
        'input',
        'link',
        'meta',
        'param',
        'source',
        'track',
        'wbr',
      ];
    }

    /**
     * Returns the current HTML string built from document fragments.
     *
     * @return {string}
     *   The HTML string built from document fragments.
     */
    build() {
      return this.chunks.join('');
    }

    /**
     * Converts a document fragment into an HTML string appended to the value.
     *
     * @param {Element|DocumentFragment} node
     *   A DOM element to be appended to the value.
     */
    appendNode(node) {
      if (node.nodeType === Node.TEXT_NODE) {
        this._appendText(node);
      } else if (node.nodeType === Node.ELEMENT_NODE) {
        this._appendElement(node);
      } else if (node.nodeType === Node.DOCUMENT_FRAGMENT_NODE) {
        this._appendChildren(node);
      }
    }

    /**
     * Appends an element node to the value.
     *
     * @param {Element} node
     *   A DOM element to be appended to the value.
     */
    _appendElement(node) {
      const nodeName = node.nodeName.toLowerCase();

      this._append('<');
      this._append(nodeName);
      this._appendAttributes(node);
      this._append('>');
      if (!this.selfClosingTags.includes(nodeName)) {
        this._appendChildren(node);
        this._append('</');
        this._append(nodeName);
        this._append('>');
      }
    }

    /**
     * Appends child nodes to the value.
     *
     * @param {Element|DocumentFragment} node
     *  A DOM element to be appended to the value.
     */
    _appendChildren(node) {
      Object.keys(node.childNodes).forEach((child) => {
        this.appendNode(node.childNodes[child]);
      });
    }

    /**
     * Appends attributes to the value.
     *
     * @param {Element} node
     *  A DOM element to be appended to the value.
     */
    _appendAttributes(node) {
      // Sort attributes alphabetically for consistent output across browsers.
      const sortedAttributes = Object.values(node.attributes).sort((a, b) => {
        return a.name > b.name;
      });
      sortedAttributes.forEach((attribute) => {
        this._append(' ');
        this._append(attribute.name);
        this._append('="');
        this._append(
          this._escapeAttribute(attribute.value),
        );
        this._append('"');
      });
    }

    /**
     * Appends text to the value.
     *
     * @param {Element} node
     *  A DOM element to be appended to the value.
     */
    _appendText(node) {
      // Repack the text into another node and extract using innerHTML. This
      // works around text nodes not having an innerHTML property and textContent
      // not encoding entities.
      // entities. That's why the text is repacked into another node and extracted
      // using innerHTML.
      const doc = document.implementation.createHTMLDocument('');
      const container = doc.createElement('p');
      container.textContent = node.textContent;

      this._append(container.innerHTML);
    }

    /**
     * Appends a string to the value.
     *
     * @param {string} str
     *  A string to be appended to the value.
     */
    _append(str) {
      this.chunks.push(str);
    }

    /**
     * Escapes attribute value for compatibility with Backdrop's XSS filtering.
     *
     * Backdrop's XSS filtering does not handle entities inside element attribute
     * values. The XSS filtering was written based on W3C XML recommendations
     * which constituted that the ampersand character (&) and the angle
     * brackets (< and >) must not appear in their literal form in attribute
     * values. This differs from the HTML living standard which permits (but
     * discourages) unescaped angle brackets.
     *
     * @param {string} text
     *  A string to be escaped.
     *
     * @return {string}
     *  Escaped string.
     *
     * @see https://www.w3.org/TR/2008/REC-xml-20081126/#NT-AttValue
     * @see https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(single-quoted)-state
     * @see https://github.com/whatwg/html/issues/6235
     */
    _escapeAttribute(text) {
      return text
        .replace(/&/g, '&amp;')
        .replace(/'/g, '&apos;')
        .replace(/"/g, '&quot;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/\r\n/g, '&#13;')
        .replace(/[\r\n]/g, '&#13;');
    }
  }

})(Backdrop);
