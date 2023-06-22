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
    const editor = this.editor;
    const contextualBalloonPlugin = editor.plugins.get('ContextualBalloon');
    const options = editor.config.get('backdropLink');
    const supportedAttributes = new Map(Object.entries(options.extraAttributes || {}));
    this.linkUI = editor.plugins.get('LinkUI');

    // Add the backdropLink command using the class specified in this file.
    editor.commands.add('backdropLink', new BackdropLinkCommand(editor));

    // Add support for each of the attributes defined in
    // config.backdropLink.extraAttributes.
    supportedAttributes.forEach((attributeName, modelName) => {
      // Extend the schema to allow these attributes.
      editor.model.schema.extend('$text', {
        allowAttributes: modelName,
      });

      // See DowncastHelpers.attributeToElement().
      // https://ckeditor.com/docs/ckeditor5/latest/api/module_engine_conversion_downcasthelpers-DowncastHelpers.html#function-attributeToElement
      editor.conversion.for('downcast').attributeToElement({
        model: modelName,
        view: (attributeValue, conversionApi) => {
          const { writer } = conversionApi;
          let attributeMap = {};
          attributeMap[attributeName] = attributeValue;
          return writer.createAttributeElement('a', attributeMap, { priority: 5 });
        },
        converterPriority: 'low'
      });

      editor.conversion.for('upcast').attributeToAttribute({
        view: {
          name: 'a',
          key: attributeName
        },
        model: modelName,
        converterPriority: 'low'
      });
    });

    // Bind to the contextual
    this.listenTo(contextualBalloonPlugin, 'change:visibleView', (evt, name, visibleView) => {
      if (visibleView === this.linkUI.formView) {
        console.log(evt);
        // Detach the listener.
        this.stopListening(contextualBalloonPlugin, 'change:visibleView');

        this.button = this._createButton();

        // Render button's template.
        this.button.render();

        // Register the button under the link form view, it will handle its destruction.
        this.linkUI.formView.registerChild(this.button);

        // Inject the element into DOM.
        this.linkUI.formView.element.insertBefore(this.button.element, this.linkUI.formView.saveButtonView.element);
      }
    });
  }

  _createButton() {
    const editor = this.editor;
    const button = new CKEditor5.ui.ButtonView(this.locale);
    const linkCommand = editor.commands.get('link');
    const options = editor.config.get('backdropLink');

    button.set({
      label: options.buttonLabel || 'Advanced',
      withText: true,
      tooltip: true
    });

    // Probably this button should be also disabled when the link command is disabled.
    // Try setting editor.isReadOnly = true to see it in action.
    button.bind('isEnabled').to(linkCommand);

    button.on('execute', () => {
      // Attempts at getting the selected element:
      // const selection = editor.model.document.selection;
      // const selectedParent = selection.anchor.parent;
      // const selectedElement = selection.getSelectedElement() || CKEditor5.utils.first(selection.getSelectedBlocks());
      // const selectedContent = editor.model.getSelectedContent(selection);
      // const linkHref = selectedContent.getAttribute('linkHref');

      const dialogSettings = {
        dialogClass: 'editor-link-dialog'
      };
      let existingValues = {
        'href': this.linkUI.formView.urlInputView.fieldView.value
      };

      // Prepare a save callback to be used upon saving the dialog.
      var saveCallback = function(returnValues) {
        console.log(returnValues);

        const linkCommand = editor.commands.get('backdropLink');
        const newHref = returnValues.attributes.href;
        delete returnValues.href;
        // Ignore a disabled target attribute.
        if (returnValues.attributes.target === 0) {
          delete returnValues.attributes.target;
        }

        // Update the selected text with the returned href. The link.execute()
        // command takes the href attribute, plus an object mapping decorators,
        // which are not useful for setting attributes.
        // See https://github.com/ckeditor/ckeditor5/blob/master/packages/ckeditor5-link/src/linkcommand.ts
        linkCommand.execute(newHref, returnValues.attributes);

        // Set other attributes on the link from the returned values.
        // See https://github.com/rhysstubbs/ckeditor5-add-attribute-to-element/blob/main/src/add-attribute-to-element-command.js
        // const element = editor.model.document.selection.getSelectedElement();
        // editor.model.change(writer => {
        //   writer.setAttribute('linkTarget', '_blank', element);
        // });

        return;

        // If an image widget is focused, we're not editing an independent
        // link, but we're wrapping an image widget in a link.
        if (focusedImageWidget) {
          // Remove attributes that are inappropriate.
          delete returnValues.attributes['data-file-id'];
          delete returnValues.attributes.text;
          focusedImageWidget.setData('link', CKEDITOR.tools.extend(returnValues.attributes, focusedImageWidget.data.link));
          return;
        }
        else {
          // Create the new link by applying a style to the new text.
          // Remove attributes that are not required.
          if (!returnValues.attributes['data-file-id']) {
            delete returnValues.attributes['data-file-id'];
          }
          delete returnValues.attributes.text;
        }
      }

      Backdrop.ckeditor5.openDialog(editor, options.dialogUrl, existingValues, saveCallback, dialogSettings);
    });

    return button;
  }
}


/**
 * Provides a command to apply a link to a selections.
 *
 * Ideally we would not need to have our own link command. But the core
 * LinkCommand cannot set attributes beyond those provided by link decorators,
 * which only support on/off toggles. See
 * https://github.com/ckeditor/ckeditor5/issues/9730
 *
 * Heavily copied from CKEditor's built-in LinkCommand. See
 * https://github.com/ckeditor/ckeditor5/blob/master/packages/ckeditor5-link/src/linkcommand.ts
 */
class BackdropLinkCommand extends CKEditor5.core.Command {

  /**
   * @inheritDoc
   */
  refresh() {
    const model = this.editor.model;
    const selection = model.document.selection;
    const selectedElement = selection.getSelectedElement() || CKEditor5.utils.first(selection.getSelectedBlocks());

    // A check for any integration that allows linking elements (e.g. `LinkImage`).
    // Currently the selection reads attributes from text nodes only. See #7429 and #7465.
    if (model.schema.checkAttribute(selectedElement.name, 'linkHref')) {
      this.value = selectedElement.getAttribute('linkHref') || undefined;
      this.isEnabled = model.schema.checkAttribute(selectedElement, 'linkHref');
    }
    else {
      this.value = selection.getAttribute('linkHref') || undefined;
      this.isEnabled = model.schema.checkAttributeInSelection(selection, 'linkHref');
    }
  }

  /**
   * Executes a command to apply a link.
   *
   * @param newHref Link destination.
   * @param newAttributes Specific attributes to be set on the target link.
   */
  execute(newHref, newAttributes) {
    const editor = this.editor;
    const model = editor.model;
    const selection = model.document.selection;
    const options = editor.config.get('backdropLink');
    const supportedAttributes = new Map(Object.entries(options.extraAttributes || {}));

    model.change(writer => {
      const findAttributeRange = CKEditor5.typing.findAttributeRange;

      // If selection is collapsed then update selected link or insert new one at the place of caret.
      if (selection.isCollapsed) {
        const position = selection.getFirstPosition();

        // When selection is inside text with `linkHref` attribute.
        if (selection.hasAttribute('linkHref')) {
          const linkText = extractTextFromSelection(selection);
          // Then update `linkHref` value.
          let linkRange = findAttributeRange( position, 'linkHref', selection.getAttribute( 'linkHref' ), model );

          if (selection.getAttribute('linkHref') === linkText ) {
            linkRange = this._updateLinkContent(model, writer, linkRange, newHref);
          }

          writer.setAttribute('linkHref', newHref, linkRange);
          supportedAttributes.forEach((attributeName, modelName) => {
            if (newAttributes.hasOwnProperty(attributeName))  {
              writer.setAttribute(modelName, newAttributes[attributeName], linkRange);
            }
          });

          // Put the selection at the end of the updated link.
          writer.setSelection(writer.createPositionAfter(linkRange.end.nodeBefore));
        }
        // If not then insert text node with `linkHref` attribute in place of caret.
        // However, since selection is collapsed, attribute value will be used as data for text node.
        // So, if `href` is empty, do not create text node.
        else if (newHref !== '') {
          const attributes = CKEditor5.utils.toMap(selection.getAttributes());

          attributes.set('linkHref', newHref);
          supportedAttributes.forEach((attributeName, modelName) => {
            if (newAttributes.hasOwnProperty(attributeName))  {
              attributes.set(modelName, newAttributes[attributeName]);
            }
          });

          const { end: positionAfter } = model.insertContent(writer.createText(newHref, attributes), position);

          // Put the selection at the end of the inserted link.
          // Using end of range returned from insertContent in case nodes with the same attributes got merged.
          writer.setSelection(positionAfter);
        }

        // Remove the `linkHref` attribute and all link decorators from the selection.
        // It stops adding a new content into the link element.
        //[ 'linkHref', ...truthyManualDecorators, ...falsyManualDecorators ].forEach(item => {
        [ 'linkHref' ].forEach(item => {
          writer.removeSelectionAttribute(item);
        });
      }
      else {
        // If selection has non-collapsed ranges, we change attribute on nodes inside those ranges
        // omitting nodes where the `linkHref` attribute is disallowed.
        const ranges = model.schema.getValidRanges(selection.getRanges(), 'linkHref');

        // But for the first, check whether the `linkHref` attribute is allowed on selected blocks (e.g. the "image" element).
        const allowedRanges = [];

        for (const element of selection.getSelectedBlocks()) {
          if (model.schema.checkAttribute( element, 'linkHref')) {
            allowedRanges.push(writer.createRangeOn(element));
          }
        }

        // Ranges that accept the `linkHref` attribute. Since we will iterate over `allowedRanges`, let's clone it.
        const rangesToUpdate = allowedRanges.slice();

        // For all selection ranges we want to check whether given range is inside an element that accepts the `linkHref` attribute.
        // If so, we don't want to propagate applying the attribute to its children.
        for (const range of ranges) {
          if (this._isRangeToUpdate( range, allowedRanges)) {
            rangesToUpdate.push( range );
          }
        }

        for (const range of rangesToUpdate) {
          let linkRange = range;

          if (rangesToUpdate.length === 1) {
            // Current text of the link in the document.
            const linkText = extractTextFromSelection(selection);

            if (selection.getAttribute('linkHref') === linkText) {
              linkRange = this._updateLinkContent(model, writer, range, newHref);
              writer.setSelection(writer.createSelection(linkRange));
            }
          }

          writer.setAttribute('linkHref', newHref, linkRange);
          supportedAttributes.forEach((attributeName, modelName) => {
            if (newAttributes.hasOwnProperty(attributeName))  {
              writer.setAttribute(modelName, newAttributes[attributeName], linkRange);
            }
          });
        }
      }
    });
  }

  /**
   * Checks whether specified `range` is inside an element that accepts the `linkHref` attribute.
   *
   * @param range A range to check.
   * @param allowedRanges An array of ranges created on elements where the attribute is accepted.
   */
  _isRangeToUpdate(range, allowedRanges) {
    for (const allowedRange of allowedRanges) {
      // A range is inside an element that will have the `linkHref` attribute. Do not modify its nodes.
      if ( allowedRange.containsRange( range ) ) {
        return false;
      }
    }

    return true;
  }

  /**
   * Updates selected link with a new value as its content and as its href attribute.
   *
   * @param model Model is need to insert content.
   * @param writer Writer is need to create text element in model.
   * @param range A range where should be inserted content.
   * @param href A link value which should be in the href attribute and in the content.
   */
  _updateLinkContent(model, writer, range, href) {
    const text = writer.createText( href, { linkHref: href } );
    return model.insertContent( text, range );
  }

}

// Returns a text of a link under the collapsed selection or a selection that contains the entire link.
function extractTextFromSelection( selection ) {
  if ( selection.isCollapsed ) {
    const firstPosition = selection.getFirstPosition();

    return firstPosition.textNode && firstPosition.textNode.data;
  }
  else {
    const rangeItems = Array.from(selection.getFirstRange().getItems());

    if (rangeItems.length > 1) {
      return null;
    }

    const firstNode = rangeItems[ 0 ];

    if (firstNode.is('$text') || firstNode.is('$textProxy')) {
      return firstNode.data;
    }

    return null;
  }
}

// Expose the plugin to the CKEditor5 namespace.
CKEditor5.backdropLink = {
  'BackdropLink': BackdropLink,
  'BackdropLinkCommand': BackdropLinkCommand,
};

})(Backdrop, CKEditor5);
