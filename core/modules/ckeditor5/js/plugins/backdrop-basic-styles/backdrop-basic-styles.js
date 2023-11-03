(function (CKEditor5) {

"use strict";

/**
 * Backdrop-specific plugin to alter the CKEditor 5 basic tags.
 */
class BackdropBasicStyles extends CKEditor5.core.Plugin {
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
    const editor = this.editor;

    // CKEditor prefers <i> but Backdrop prefers <em>.
    editor.conversion.for('downcast').attributeToElement({
      model: 'italic',
      view: 'em',
      converterPriority: 'high',
    });
    // CKEditor prefers <s> but Backdrop prefers <del>.
    editor.conversion.for('downcast').attributeToElement({
      model: 'strikethrough',
      view: 'del',
      converterPriority: 'high',
    });
    // Backdrop previously preferred <span class="underline">. This converts it
    // to <u> tags preferred by CKEditor 5.
    editor.conversion.for('upcast').elementToAttribute({
      model: 'underline',
      view: {
        name: 'span',
        classes: ['underline'],
      },
      converterPriority: 'low',
    });

    // Support a minimum height option on the editor.
    // See https://stackoverflow.com/a/56550285/845793
    // @todo: Move this functionality or rename the entire plugin?
    const minHeight = editor.config.get('minHeight');
    if (minHeight) {
      editor.ui.view.editable.extendTemplate({
        attributes: {
          style: {
            minHeight: minHeight,
          }
        }
      });
    }
  }
}

// Expose the plugin to the CKEditor5 namespace.
CKEditor5.backdropBasicStyles = {
  'BackdropBasicStyles': BackdropBasicStyles
};

})(CKEditor5);
