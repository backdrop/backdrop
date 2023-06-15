import { Plugin } from 'ckeditor5/src/core';

/**
 * Backdrop-specific plugin to alter the CKEditor 5 basic tags.
 *
 * @private
 */
class BackdropBasicStyles extends Plugin {
  /**
   * @inheritdoc
   */
  static get pluginName() {
    return 'BackdropBasicStyles';
  }

  /**
   * @inheritdoc
   */
  init() {
    this.editor.conversion.for('downcast').attributeToElement({
      model: 'italic',
      view: 'em',
      converterPriority: 'high',
    });
    this.editor.conversion.for('downcast').attributeToElement({
      model: 'strikethrough',
      view: 'del',
      converterPriority: 'high',
    });
    this.editor.conversion.for('downcast').attributeToElement({
      model: 'underline',
      // @todo: Determine how to add class underline to this element.
      view: 'span',
      converterPriority: 'high',
    });
  }
}

export default BackdropBasicStyles;
