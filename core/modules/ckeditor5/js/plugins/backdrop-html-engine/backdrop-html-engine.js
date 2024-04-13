(function (Backdrop, CKEditor5) {

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
    return Backdrop.ckeditor5.elementGetHtml(fragment);
  }
}

})(Backdrop, CKEditor5);
