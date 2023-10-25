/**
 * @file
 * Backdrop Link plugin.
 */

(function (Backdrop, CKEditor5) {

/**
 * Based on https://github.com/ckeditor/ckeditor5/issues/4836.
 */
class BackdropLink extends CKEditor5.core.Plugin {
  /**
   * @inheritdoc
   */
  static get requires() {
    return ['Link'];
  }

  /**
   * @inheritdoc
   */
  static get pluginName() {
    return 'BackdropLink';
  }

  /**
   * @inheritdoc
   */

  init() {
    const config = this.editor.config.get('backdropLink');

    if (!config.extraAttributes) {
      return;
    }
    // Convert attributes to map for easier looping.
    const extraAttributes = new Map(Object.entries(config.extraAttributes));

    extraAttributes.forEach((attributeName, modelName) => {
      this._allowAndConvertExtraAttribute(modelName, attributeName);
      this._removeExtraAttributeOnUnlinkCommandExecute(modelName);
      this._refreshExtraAttributeValue(modelName);
    });

    this._addExtraAttributeOnLinkCommandExecute(extraAttributes);
    this._bindBalloon(extraAttributes);
  }

  _allowAndConvertExtraAttribute(modelName, viewName) {
    const editor = this.editor;

    editor.model.schema.extend('$text', { allowAttributes: modelName });

    // Model -> View (DOM)
    editor.conversion.for('downcast').attributeToElement({
      model: modelName,
      view: (value, { writer }) => {
        const linkViewElement = writer.createAttributeElement('a', {
          [ viewName ]: value
        }, { priority: 5 });

        // Without it the isLinkElement() will not recognize the link and the UI
        // will not show up when the user clicks a link.
        writer.setCustomProperty('link', true, linkViewElement);

        return linkViewElement;
      }
    });

    // View (DOM/DATA) -> Model
    editor.conversion.for('upcast').elementToAttribute({
      view: {
        name: 'a',
        attributes: {
          [ viewName ]: true
        }
      },
      model: {
        key: modelName,
        value: viewElement => viewElement.getAttribute(viewName)
      }
    });
  }

  _addExtraAttributeOnLinkCommandExecute(extraAttributes) {
    const editor = this.editor;
    const linkCommand = editor.commands.get( 'link' );
    let linkCommandExecuting = false;

    linkCommand.on('execute', (evt, args) => {
      // Custom handling is only required if an extra attribute was passed into
      // editor.execute('link', ...).
      if (args.length < 3) {
        return;
      }
      if (linkCommandExecuting) {
        linkCommandExecuting = false;
        return;
      }

      // If the additional attribute was passed, we stop the default execution
      // of the LinkCommand. We're going to create Model#change() block for undo
      // and execute the LinkCommand together with setting the extra attribute.
      evt.stop();

      // Prevent infinite recursion by keeping records of when link command is
      // being executed by this function.
      linkCommandExecuting = true;
      const extraAttributeValues = args[args.length - 1];
      const model = this.editor.model;
      const selection = model.document.selection;
      const imageUtils = editor.plugins.get('ImageUtils');
      const closestImage = imageUtils.getClosestSelectedImageElement(selection);

      // Wrapping the original command execution in a model.change() block to
      // make sure there's a single undo step when the extra attribute is added.
      model.change((writer) => {
        editor.execute('link', ...args);

        // If there is an image within this link, apply the link attributes to
        // the image model's htmlLinkAttributes attribute.
        if (closestImage) {
          const htmlLinkAttributes = { attributes: extraAttributeValues };
          writer.setAttribute('htmlLinkAttributes', htmlLinkAttributes, closestImage);
        }
        // Otherwise find the selected link and apply each attribute.
        else {
          extraAttributes.forEach((attributeName, modelName) => {
            let ranges = [];
            if (selection.isCollapsed) {
              const firstPosition = selection.getFirstPosition();
              const node = firstPosition.textNode || firstPosition.nodeBefore;
              ranges = [ writer.createRangeOn(node) ];
            }
            else {
              ranges = model.schema.getValidRanges(selection.getRanges(), modelName);
            }

            for (const range of ranges) {
              if (extraAttributeValues[attributeName]) {
                writer.setAttribute(modelName, extraAttributeValues[attributeName], range);
              } else {
                writer.removeAttribute(modelName, range);
              }
            }
            writer.removeSelectionAttribute(modelName);
          });
        }
      });
    }, { priority: 'highest' } );
  }

  _removeExtraAttributeOnUnlinkCommandExecute(modelName) {
    const editor = this.editor;
    const unlinkCommand = editor.commands.get('unlink');
    const model = this.editor.model;
    const selection = model.document.selection;

    let isUnlinkingInProgress = false;

    // Make sure all changes are in a single undo step so cancel the original unlink first in the high priority.
    unlinkCommand.on('execute', evt => {
      if (isUnlinkingInProgress) {
        return;
      }

      evt.stop();

      // This single block wraps all changes that should be in a single undo step.
      model.change(() => {
        // Now, in this single "undo block" let the unlink command flow naturally.
        isUnlinkingInProgress = true;

        // Do the unlinking within a single undo step.
        editor.execute('unlink');

        // Let's make sure the next unlinking will also be handled.
        isUnlinkingInProgress = false;

        // The actual integration that removes the extra attribute.
        model.change(writer => {
          // Get ranges to unlink.
          let ranges;

          if (selection.isCollapsed) {
            ranges = [CKEditor5.typing.findAttributeRange(
              selection.getFirstPosition(),
              modelName,
              selection.getAttribute( modelName ),
              model
            )];
          }
          else {
            ranges = model.schema.getValidRanges(selection.getRanges(), modelName);
          }

          // Remove the extra attribute from specified ranges.
          for (const range of ranges) {
            writer.removeAttribute(modelName, range);
          }
        });
      });
    }, { priority: 'high' });
  }

  _refreshExtraAttributeValue(modelName) {
    const editor = this.editor;
    const linkCommand = editor.commands.get('link');
    const model = this.editor.model;
    const selection = model.document.selection;

    linkCommand.set(modelName, null);

    model.document.on('change', () => {
      linkCommand[modelName] = selection.getAttribute(modelName);
    });
  }

  _bindBalloon(extraAttributes) {
    const editor = this.editor;
    const contextualBalloonPlugin = editor.plugins.get('ContextualBalloon');
    const linkUI = editor.plugins.get('LinkUI');
    let additionalUiSet = false;

    // Bind to the balloon being shown and check for the link UI.
    this.listenTo(contextualBalloonPlugin, 'change:visibleView', (evt, name, visibleView) => {
      if (linkUI.formView && visibleView === linkUI.formView) {
        if (!additionalUiSet) {
          additionalUiSet = true;
          this.button = this._createButton(extraAttributes);

          // Render button's template.
          this.button.render();

          // Register the button under the link form view, it will handle its destruction.
          linkUI.formView.registerChild(this.button);

          // Inject the element into DOM.
          linkUI.formView.element.insertBefore(this.button.element, linkUI.formView.saveButtonView.element);
        }

        // Immediately click the newly added button.
        this.button.fire('execute');

        // @todo: Find a way to close the balloon but maintain focus.
        // linkUI._hideUI();
      }
    });
  }

  _createButton(extraAttributes) {
    const editor = this.editor;
    const button = new CKEditor5.ui.ButtonView(this.locale);
    const linkCommand = editor.commands.get('link');
    const linkUI = editor.plugins.get('LinkUI');
    const config = this.editor.config.get('backdropLink')

    button.set({
      label: config.buttonLabel || 'Advanced',
      withText: true,
      tooltip: true
    });

    // Probably this button should be also disabled when the link command is disabled.
    // Try setting editor.isReadOnly = true to see it in action.
    button.bind('isEnabled').to(linkCommand);

    button.on('execute', () => {
      const dialogSettings = {
        dialogClass: 'editor-link-dialog'
      };
      const selection = editor.model.document.selection;
      const imageUtils = editor.plugins.get('ImageUtils');
      const closestImage = imageUtils.getClosestSelectedImageElement(selection);

      // Pull in existing values from the model to be sent to the dialog.
      let existingValues = {
        'href': linkUI.formView.urlInputView.fieldView.value,
      };
      if (closestImage) {
        const htmlLinkAttributes = closestImage.getAttribute('htmlLinkAttributes');
        if (htmlLinkAttributes && htmlLinkAttributes.attributes) {
          existingValues = Object.assign(existingValues, htmlLinkAttributes.attributes);
        }
      }
      else {
        extraAttributes.forEach((attributeName, modelName) => {
          existingValues[attributeName] = linkCommand[modelName];
        });
      }

      // Prepare a save callback to be used upon saving the dialog.
      const saveCallback = function(returnValues) {
        const linkCommand = editor.commands.get('link');
        const newHref = returnValues.attributes.href;
        delete returnValues.href;
        // Ignore a disabled target attribute.
        if (returnValues.attributes.target === 0) {
          delete returnValues.attributes.target;
        }
        // Remove empty file IDs.
        if (!returnValues.attributes['data-file-id']) {
          delete returnValues.attributes['data-file-id'];
        }
        // Remove "text" key intended to update the link text (not supported).
        if (returnValues.attributes.hasOwnProperty('text')) {
          delete returnValues.attributes['text'];
        }

        // The normal link command does not support a 3rd argument natively.
        // This has been extended in _addExtraAttributeOnLinkCommandExecute()
        // to also accept an array of attributes to be saved.
        // See https://github.com/ckeditor/ckeditor5/blob/master/packages/ckeditor5-link/src/linkcommand.ts
        // There is also a feature request to make this native to CKEditor
        // here: https://github.com/ckeditor/ckeditor5/issues/9730
        linkCommand.execute(newHref, {}, returnValues.attributes);
      }

      Backdrop.ckeditor5.openDialog(editor, config.dialogUrl, existingValues, saveCallback, dialogSettings);
    });

    return button;
  }
}

// Expose the plugin to the CKEditor5 namespace.
CKEditor5.backdropLink = {
  'BackdropLink': BackdropLink
};

})(Backdrop, CKEditor5);
