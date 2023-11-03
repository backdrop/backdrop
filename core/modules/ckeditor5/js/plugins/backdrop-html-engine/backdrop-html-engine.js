(function (CKEditor5) {

"use strict";

/**
 * A plugin that overrides the CKEditor HTML writer.
 *
 * Overrides the CKEditor 5 HTML writer to properly escape HTML attributes.
 *
 * In particular this makes it so that `<` and `>` characters are escaped when
 * used within the data-caption attribute, allowing caption text to be linked
 * or styled as bold, italic, etc.
 *
 * @see https://github.com/ckeditor/ckeditor5/issues/15293
 * @see https://github.com/backdrop-contrib/ckeditor5/issues/68
 */
class BackdropHtmlEngine extends CKEditor5.core.Plugin {
  /**
   * @inheritdoc
   */
  static get pluginName() {
    return 'BackdropHtmlEngine';
  }

  /**
   * @inheritdoc
   */
  init() {
    this.editor.data.processor.htmlWriter = new BackdropHtmlWriter();
  }
}

// Expose the plugin to the CKEditor5 namespace.
CKEditor5.backdropHtmlEngine = {
  'BackdropHtmlEngine': BackdropHtmlEngine
};

/**
 * Custom HTML writer. It creates HTML by traversing DOM nodes.
 *
 * It differs to CKEditor's core BasicHtmlWriter in the way it encodes entities
 * in element attributes.
 *
 * @see https://ckeditor.com/docs/ckeditor5/latest/api/module_engine_dataprocessor_basichtmlwriter-BasicHtmlWriter.html
 */
class BackdropHtmlWriter {
  /**
   * Returns an HTML string created from the document fragment.
   *
   * @param {Element} fragment
   * @return {String}
   */
  getHtml(fragment) {
    const builder = new BackdropHtmlBuilder();
    builder.appendNode(fragment);

    return builder.build();
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
   * @param {Element} node
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
   * @param {Element} node
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
    Object.keys(node.attributes).forEach((attr) => {
      this._append(' ');
      this._append(node.attributes[attr].name);
      this._append('="');
      this._append(
        this._escapeAttribute(node.attributes[attr].value),
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

})(CKEditor5);
