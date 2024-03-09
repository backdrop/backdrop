/**
 * @file
 * Backdrop Link plugin.
 */

(function (Backdrop, CKEditor5) {

"use strict";

/**
 * The BackdropLink plugin replaces and extends the CKEditor core Link plugin.
 *
 * - Adds its own buttons for backdropLink (main toolbar) and backdropLinkImage
 *   (in the image balloon toolbar).
 * - Modifies the balloon toolbar for links. Instead of the edit action using
 *   the balloon to enter the URL, the Backdrop link dialog is used instead.
 * - Extends the editor.execute('link') function to take a 3rd parameter to
 *   apply attributes such as id, rel, and class to links.
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
    const editor = this.editor;
    const config = editor.config.get('backdropLink');

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
    this._addBackdropLinkButtons();
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
    // The class attribute should be passed using the special selector 'classes'
    // rather than attributes['class'].
    // See https://ckeditor.com/docs/ckeditor5/latest/support/error-codes.html#error-matcher-pattern-deprecated-attributes-class-key
    const upcastView = { name: 'a' };
    if (viewName === 'class') {
      upcastView['classes'] = true;
    }
    else {
      upcastView['attributes'] = { [ viewName ]: true }
    }
    editor.conversion.for('upcast').elementToAttribute({
      view: upcastView,
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

  _addBackdropLinkButtons() {
    const editor = this.editor;
    const config = editor.config.get('backdropLink');
    const editLabel = config.editLabel || 'Edit Link';
    const insertLabel = config.insertLabel || 'Insert Link';

    // Add the backdropLink command.
    editor.commands.add('backdropLink', new BackdropLinkCommand(editor));
    const backdropLinkCommand = editor.commands.get('backdropLink');

    // Add the backdropLink button for use in the main toolbar. This can
    // insert a new link or edit an existing one if selected.
    editor.ui.componentFactory.add('backdropLink', (locale) => {
      const buttonView = new CKEditor5.ui.ButtonView(locale);

      buttonView.set({
        label: insertLabel,
        icon: backdropLinkIcon,
        tooltip: true
      });

      // Highlight the link button when a link is selected.
      buttonView.bind('isOn').to(backdropLinkCommand, 'value');

      // Change the label when an image is selected.
      buttonView.bind('label').to(backdropLinkCommand, 'value', (value) => {
        return value ? editLabel : insertLabel
      });

      // Disable the button when the command is disabled by source mode.
      buttonView.bind('isEnabled').to(backdropLinkCommand, 'isEnabled');

      // When clicking the toolbar button, execute the backdropLink command.
      buttonView.on('execute', () => {
        // Remove focus from the toolbar button when opening the dialog.
        // Otherwise, the button may receive focus again after closing the
        // dialog.
        buttonView.element.blur();
        // See BackdropLinkCommand::execute().
        backdropLinkCommand.execute();
      });

      return buttonView;
    });

    // Add the backdropLinkImage button for use in the image toolbar. This can
    // insert a new link or edit an existing one if selected.
    editor.ui.componentFactory.add('backdropLinkImage', (locale) => {
      const buttonView = new CKEditor5.ui.ButtonView(locale);
      const backdropLinkCommand = editor.commands.get('backdropLink');
      buttonView.set( {
        isEnabled: true,
        // Translation provided by CKEditor link plugin:
        label: editor.t('Link image'),
        icon: backdropLinkIcon,
        keystroke: 'Ctrl+K',
        tooltip: true,
        isToggleable: true
      });

      // Bind button to the command.
      buttonView.bind('isEnabled').to(backdropLinkCommand, 'isEnabled');
      buttonView.bind('isOn').to(backdropLinkCommand, 'value');

      this.listenTo(buttonView, 'execute', () => {
        // Show the normal link UI balloon if an image already has a link.
        // This allows unlinking an image or previewing the link URL.
        const selectedModelElement = editor.model.document.selection.getSelectedElement();
        const imageUtils = editor.plugins.get('ImageUtils');
        const linkUI = editor.plugins.get('LinkUI');
        if (imageUtils.isImage(selectedModelElement) && selectedModelElement.hasAttribute('linkHref')) {
          // This is not ideal to call an internal method to show the balloon,
          // but this is the same approach used by LinkImageUI.
          // See https://github.com/ckeditor/ckeditor5/blob/master/packages/ckeditor5-link/src/linkimageui.ts
          linkUI._addActionsView();
        }
        // For new links, open the link dialog directly.
        else {
          backdropLinkCommand.execute();
        }
      });

      return buttonView;
    });

    // Claim the keyboard shortcut for making a link to open the dialog rather
    // than CKEditor's bubble toolbar.
    editor.keystrokes.set('Ctrl+K', (keyEvtData, cancel) => {
      // Prevent focusing the search bar in FF, Chrome and Edge. See https://github.com/ckeditor/ckeditor5/issues/4811.
      cancel();
      if (editor.commands.get('backdropLink').isEnabled) {
        backdropLinkCommand.execute();
      }
    }, { priority: 'high' });
  }

  _bindBalloon() {
    const editor = this.editor;
    const contextualBalloonPlugin = editor.plugins.get('ContextualBalloon');
    const linkUI = editor.plugins.get('LinkUI');
    const backdropLinkCommand = editor.commands.get('backdropLink');
    let linkUiModified = false;

    // Bind to the balloon being shown and check for the link UI.
    this.listenTo(contextualBalloonPlugin, 'change:visibleView', (evt, name, visibleView) => {
      const actionsView = linkUI.actionsView;
      if (actionsView && visibleView === actionsView) {
        if (!linkUiModified) {
          linkUiModified = true;
          // Turn off the normal link editing action.
          // See LinkUI::_createActionsView().
          linkUI.stopListening(actionsView, 'edit');
          // Replace with firing the backdropLink action instead.
          this.listenTo(actionsView, 'edit', () => {
            contextualBalloonPlugin.remove(actionsView);
            backdropLinkCommand.execute();
          });
        }
      }
    });
  }
}

// Expose the plugin to the CKEditor5 namespace.
CKEditor5.backdropLink = {
  'BackdropLink': BackdropLink
};

/**
 * CKEditor command that opens the Backdrop link editing dialog.
 */
class BackdropLinkCommand extends CKEditor5.core.Command {
  /**
   * @inheritdoc
   */
  refresh() {
    const editor = this.editor;
    const linkCommand = editor.commands.get('link');
    this.isEnabled = linkCommand.isEnabled;
    this.value = linkCommand.value;
  }

  /**
   * @inheritdoc
   */
  execute() {
    const editor = this.editor;
    const config = editor.config.get('backdropLink');
    const selection = editor.model.document.selection;
    const linkCommand = editor.commands.get('link');
    const imageUtils = editor.plugins.get('ImageUtils');
    const linkUI = editor.plugins.get('LinkUI');

    const closestImage = imageUtils.getClosestSelectedImageElement(selection);
    const extraAttributes = new Map(Object.entries(config.extraAttributes));
    const dialogSettings = {
      dialogClass: 'editor-link-dialog'
    };

    // Pull in existing values from the model to be sent to the dialog.
    let existingValues = {
      'href': linkUI.formView ? linkUI.formView.urlInputView.fieldView.value : '',
    };

    // Images store link values in a special 'htmlLinkAttributes' attribute.
    if (closestImage) {
      const htmlLinkAttributes = closestImage.getAttribute('htmlLinkAttributes');
      if (htmlLinkAttributes && htmlLinkAttributes.attributes) {
        existingValues = Object.assign(existingValues, htmlLinkAttributes.attributes);
      }
    }
    // For normal links, pull link values from individual attributes.
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
  }
}

// The CKEditor core link icon is not in a reusable location, so this is a
// duplicated version for Backdrop use.
const backdropLinkIcon = '<svg viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"><path d="m11.077 15 .991-1.416a.75.75 0 1 1 1.229.86l-1.148 1.64a.748.748 0 0 1-.217.206 5.251 5.251 0 0 1-8.503-5.955.741.741 0 0 1 .12-.274l1.147-1.639a.75.75 0 1 1 1.228.86L4.933 10.7l.006.003a3.75 3.75 0 0 0 6.132 4.294l.006.004zm5.494-5.335a.748.748 0 0 1-.12.274l-1.147 1.639a.75.75 0 1 1-1.228-.86l.86-1.23a3.75 3.75 0 0 0-6.144-4.301l-.86 1.229a.75.75 0 0 1-1.229-.86l1.148-1.64a.748.748 0 0 1 .217-.206 5.251 5.251 0 0 1 8.503 5.955zm-4.563-2.532a.75.75 0 0 1 .184 1.045l-3.155 4.505a.75.75 0 1 1-1.229-.86l3.155-4.506a.75.75 0 0 1 1.045-.184z"/></svg>';

})(Backdrop, CKEditor5);
