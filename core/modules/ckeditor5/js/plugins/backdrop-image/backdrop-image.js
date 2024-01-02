/**
 * @file
 * Contains BackdropImage CKEditor 5 plugin and its dependent classes.
 */
(function (CKEditor5) {

"use strict";

/**
 * BackdropImage CKEditor 5 plugin.
 *
 * This complex plugin provides several pieces of key functionality:
 *
 * - Provides an image upload adapter to save images to Backdrop's file system.
 * - Saves data-file-id attributes on img tags which Backdrop uses to track
 *   where files are being used.
 * - Connects to a Backdrop-native dialog via an AJAX request.
 *
 * If this were developed under the normal CKEditor build process, this would
 * likely be split into multiple plugins and files. Backdrop does not use a
 * compilation step, so what is normally 5-8 files is all done in a single file.
 */
class BackdropImage extends CKEditor5.core.Plugin {
  /**
   * @inheritdoc
   */
  static get requires() {
    return ['ImageUtils', 'FileRepository', 'Image', 'WidgetToolbarRepository', 'ContextualBalloon'];
  }

  /**
   * @inheritdoc
   */
  static get pluginName() {
    return 'BackdropImage';
  }

  /**
   * @inheritdoc
   */
  init() {
    const editor = this.editor;
    const { conversion } = editor;
    const { schema } = editor.model;
    const config = editor.config.get('backdropImage');
    const editLabel = config.editLabel || 'Edit Image';
    const insertLabel = config.insertLabel || 'Insert Image';

    if (!config.extraAttributes) {
      return;
    }

    // Add extra supported attributes to the image models.
    if (schema.isRegistered('imageInline')) {
      schema.extend('imageInline', {
        allowAttributes: Object.keys(config.extraAttributes)
      });
    }
    if (schema.isRegistered('imageBlock')) {
      schema.extend('imageBlock', {
        allowAttributes: Object.keys(config.extraAttributes)
      });
    }

    // Upcast from the raw <img> element to the CKEditor model.
    conversion
      .for('upcast')
      .add(viewImageToModelImage(editor));

    // Upcast any wrapping <a> element to be part of the CKEditor model.
    if (editor.plugins.has('DataFilter')) {
      const dataFilter = editor.plugins.get('DataFilter');
      conversion
        .for('upcast')
        .add(upcastImageBlockLinkGhsAttributes(dataFilter));
    }

    // Downcast from the CKEditor model to an HTML <img> element. This generic
    // downcast runs both while editing and when saving the final HTML.
    conversion
      .for('downcast')
      // Convert the FileId to data-file-id attribute.
      .add(modelFileIdToDataAttribute());

    // Downcast from the CKEditor model to an HTML <img> element. A dataDowncast
    // conversion like this only runs on saving the final HTML, not while the
    // editor is being used.
    conversion
      .for('dataDowncast')
      // Pull out the caption if present. This needs to be done before other
      // conversions because afterward the caption element is eleminated.
      .add(viewCaptionToCaptionAttribute(editor))
      // Create a blank image element, removing any wrapping figure element.
      .elementToElement({
        model: 'imageBlock',
        view: (modelElement, { writer }) => {
          return writer.createEmptyElement('img');
        },
        converterPriority: 'high',
      })
      .elementToElement({
        model: 'imageInline',
        view: (modelElement, { writer }) => {
          return writer.createEmptyElement('img');
        },
        converterPriority: 'high',
      })
      // Convert ImageStyle to data-align attribute.
      .add(modelImageStyleToDataAttribute(editor))
      // Convert height and width attributes.
      .add(modelImageWidthAndHeightToAttributes(editor))
      // Convert any link to wrap the <img> tag.
      .add(downcastBlockImageLink());

    // Add the backdropImage command.
    editor.commands.add('backdropImage', new BackdropImageCommand(editor));

    // Add the editBackdropImage button, for use in the balloon toolbar.
    // This button has a different icon than the main toolbar button.
    editor.ui.componentFactory.add('editBackdropImage', (locale) => {
      const command = editor.commands.get('backdropImage');
      const buttonView = new CKEditor5.ui.ButtonView(locale);
      buttonView.set({
        label: editLabel,
        icon: CKEditor5.core.icons.pencil,
        tooltip: true
      });

      // When clicking the balloon button, execute the backdropImage command.
      buttonView.on('execute', () => {
        // See BackdropImageCommand::execute().
        command.execute();
      });

      return buttonView;
    });

    // Add the backdropImage button for use in the main toolbar. This can
    // insert a new image or edit an existing one if selected.
    editor.ui.componentFactory.add('backdropImage', (locale) => {
      const buttonView = new CKEditor5.ui.ButtonView(locale);
      const command = editor.commands.get('backdropImage');

      buttonView.set({
        label: insertLabel,
        icon: CKEditor5.core.icons.image,
        tooltip: true
      });

      // Highlight the image button when an image is selected.
      buttonView.bind('isOn').to(command, 'value');

      // Change the label when an image is selected.
      buttonView.bind('label').to(command, 'value', (value) => {
        return value ? editLabel : insertLabel
      });

      // Disable the button when the command is disabled by source mode.
      buttonView.bind('isEnabled').to(command, 'isEnabled');

      // When clicking the toolbar button, execute the backdropImage command.
      buttonView.on('execute', () => {
        // Remove focus from the toolbar button when opening the dialog.
        // Otherwise, the button may receive focus again after closing the
        // dialog.
        buttonView.element.blur();
        // See BackdropImageCommand::execute().
        command.execute();
      });

      return buttonView;
    });

    // Attach the file upload adapter to handle saving files.
    if (!config.uploadUrl) {
      throw new Error('Missing backdropImage.uploadUrl configuration option.');
    }
    editor.plugins.get('FileRepository').createUploadAdapter = (loader) => {
      return new BackdropImageUploadAdapter(loader, editor, config);
    };

    // Upon completing an uploaded file, save the returned File ID.
    const imageUploadEditing = editor.plugins.get('ImageUploadEditing');
    imageUploadEditing.on('uploadComplete', (evt, { data, imageElement }) => {
      editor.model.change((writer) => {
        writer.setAttribute('dataFileId', data.response.fileId, imageElement);
      });
    });
  }

}

// Expose the plugin to the CKEditor5 namespace.
CKEditor5.backdropImage = {
  'BackdropImage': BackdropImage
};

/**
 * Helper function for the downcast converter.
 *
 * Sets attributes on the given view element. This function is copied from
 * the General HTML Support plugin.
 * See https://ckeditor.com/docs/ckeditor5/latest/api/module_html-support_conversionutils.html#function-setViewAttributes
 *
 * @param writer The view writer.
 * @param viewAttributes The GHS attribute value.
 * @param viewElement The view element to update.
 */
function setViewAttributes(writer, viewAttributes, viewElement) {
  if (viewAttributes.attributes) {
    for (const [key, value] of Object.entries(viewAttributes.attributes)) {
      writer.setAttribute(key, value, viewElement);
    }
  }

  if (viewAttributes.styles) {
    writer.setStyle(viewAttributes.styles, viewElement);
  }

  if (viewAttributes.classes ) {
    writer.addClass(viewAttributes.classes, viewElement);
  }
}

/**
 * Generates a callback that saves the File ID to an attribute on downcast.
 *
 * @return {function}
 *  Callback that binds an event to its parameter.
 *
 * @private
 */
function modelFileIdToDataAttribute() {
  /**
   * Callback for the attribute:dataFileId event.
   *
   * Saves the File ID value to the data-file-id attribute.
   *
   * @param {Event} event
   * @param {object} data
   * @param {module:engine/conversion/downcastdispatcher~DowncastConversionApi} conversionApi
   */
  function converter(event, data, conversionApi) {
    const { item } = data;
    const { consumable, writer } = conversionApi;

    if (!consumable.consume(item, event.name)) {
      return;
    }

    const viewElement = conversionApi.mapper.toViewElement(item);
    const imageInFigure = Array.from(viewElement.getChildren()).find(
      (child) => child.name === 'img',
    );

    writer.setAttribute(
      'data-file-id',
      data.attributeNewValue,
      imageInFigure || viewElement,
    );
  }

  return (dispatcher) => {
    dispatcher.on('attribute:dataFileId', converter);
  };
}

/**
 * A mapping between the CKEditor model names and the data-align attribute.
 *
 * @type {Array.<{dataValue: string, modelValue: string}>}
 */
const alignmentMapping = [
  {
    modelValue: 'alignCenter',
    dataValue: 'center',
  },
  {
    modelValue: 'alignRight',
    dataValue: 'right',
  },
  {
    modelValue: 'alignLeft',
    dataValue: 'left',
  },
];

/**
 * Given an imageStyle, return the associated Backdrop data-align attribute.
 *
 * @param string imageStyle
 *   The image style such as alignLeft, alignCenter, or alignRight.
 * @returns {string|undefined}
 *   The data attribute value such as left, center, or right.
 * @private
 */
function _getDataAttributeFromModelImageStyle(imageStyle) {
  const mappedAlignment = alignmentMapping.find(
    (value) => value.modelValue === imageStyle,
  );
  return mappedAlignment ? mappedAlignment.dataValue : undefined;
}

/**
 * Given a data-attribute, return the associated CKEditor image style.
 *
 * @param string dataAttribute
 *   The data attribute value such as left, center, or right.
 * @returns {string|undefined}
 *   The image style such as alignLeft, alignCenter, or alignRight.
 * @private
 */
function _getModelImageStyleFromDataAttribute(dataAttribute) {
  const mappedAlignment = alignmentMapping.find(
    (value) => value.dataValue === dataAttribute,
  );
  return mappedAlignment ? mappedAlignment.modelValue : undefined;
}

/**
 * Given width, height, and resizedWidth, return updated width and height.
 *
 * @param string width
 *   The image attribute width in pixels, may contain a 'px' suffix.
 * @param string height
 *   The image attribute height in pixels, may contain a 'px' suffix.
 * @param string resizedWidth
 *   The displayed width after resizing, may contain a 'px' suffix.
 * @returns {Array}
 *   The calculated width and height based on the ratio from the resizedWidth.
 * @private
 */
function _getResizedWidthHeight(width, height, resizedWidth) {
  // Normalize each of the attributes.
  width = Number(width.toString().replace('px', ''));
  height = Number(height.toString().replace('px', ''));
  resizedWidth = Number(resizedWidth.toString().replace('px', ''));

  const ratio = width / resizedWidth;
  const newWidth = resizedWidth.toString();
  const newHeight = Math.round(height / ratio).toString();
  return [newWidth, newHeight];
}

/**
 * Downcasts `caption` model to `data-caption` attribute with its content
 * downcasted to plain HTML.
 *
 * This is needed because CKEditor 5 uses the `<caption>` element internally in
 * various places, which differs from Backdrop which uses an attribute. For now
 * to support that we have to manually repeat work done in the
 * DowncastDispatcher's private methods.
 *
 * @param {module:core/editor/editor~Editor} editor
 *  The editor instance to use.
 *
 * @return {function}
 *  Callback that binds an event to its parameter.
 *
 * @private
 */
function viewCaptionToCaptionAttribute(editor) {
  /**
   * Callback for the insert:caption event.
   */
  function converter(event, data, conversionApi) {
    const { consumable, writer, mapper } = conversionApi;
    const imageUtils = editor.plugins.get('ImageUtils');

    if (
      !imageUtils.isImage(data.item.parent) ||
      !consumable.consume(data.item, 'insert')
    ) {
      return;
    }

    const range = editor.model.createRangeIn(data.item);
    const viewDocumentFragment = writer.createDocumentFragment();

    // Bind caption model element to the detached view document fragment so
    // all content of the caption will be downcasted into that document
    // fragment.
    mapper.bindElements(data.item, viewDocumentFragment);

    // eslint-disable-next-line no-restricted-syntax
    for (const { item } of Array.from(range)) {
      const itemData = {
        item,
        range: editor.model.createRangeOn(item),
      };

      // The following lines are extracted from
      // DowncastDispatcher._convertInsertWithAttributes().
      const eventName = `insert:${item.name || '$text'}`;

      editor.data.downcastDispatcher.fire(
        eventName,
        itemData,
        conversionApi,
      );

      // eslint-disable-next-line no-restricted-syntax
      for (const key of item.getAttributeKeys()) {
        Object.assign(itemData, {
          attributeKey: key,
          attributeOldValue: null,
          attributeNewValue: itemData.item.getAttribute(key),
        });

        editor.data.downcastDispatcher.fire(
          `attribute:${key}`,
          itemData,
          conversionApi,
        );
      }
    }

    // Unbind all the view elements that were downcast to the document fragment.
    // eslint-disable-next-line no-restricted-syntax
    for (const child of writer
      .createRangeIn(viewDocumentFragment)
      .getItems()) {
      mapper.unbindViewElement(child);
    }

    mapper.unbindViewElement(viewDocumentFragment);

    // Stringify view document fragment to HTML string.
    const captionText = editor.data.processor.toData(viewDocumentFragment);

    if (captionText) {
      const imageViewElement = mapper.toViewElement(data.item.parent);

      writer.setAttribute('data-caption', captionText, imageViewElement);
    }
  }

  return (dispatcher) => {
    dispatcher.on('insert:caption', converter, { priority: 'high' });
  };
}

/**
 * Generates a callback that saves data-align attribute on data downcast.
 *
 * @return {function}
 *  Callback that binds an event to its parameter.
 *
 * @private
 */
function modelImageStyleToDataAttribute(editor) {
  /**
   * Callback for the attribute:imageStyle event.
   *
   * Saves the alignment value to the data-align attribute.
   */
  function converter(event, data, conversionApi) {
    const { item } = data;
    const { consumable, writer } = conversionApi;
    const alignAttribute = _getDataAttributeFromModelImageStyle(data.attributeNewValue);

    // Consume only for the values that can be converted into data-align.
    if (!alignAttribute || !consumable.consume(item, event.name)) {
      return;
    }

    const imageUtils = editor.plugins.get('ImageUtils');
    const viewElement = conversionApi.mapper.toViewElement(item);
    const img = imageUtils.findViewImgElement(viewElement);

    writer.setAttribute(
      'data-align',
      alignAttribute,
      img,
    );
  }

  return (dispatcher) => {
    dispatcher.on('attribute:imageStyle', converter, { priority: 'high' });
  };
}

/**
 * Generates a callback that handles the data downcast for img width and height.
 *
 * @return {function}
 *  Callback that binds an event to its parameter.
 *
 * @private
 */
function modelImageWidthAndHeightToAttributes(editor) {
  /**
   * Callback for the downcast event.
   *
   * This gets called once for every width, height, or resizedWidth attribute
   * that needs to be saved.
   */
  function converter(event, data, conversionApi) {
    const { item } = data;
    const { consumable, writer } = conversionApi;

    if (!consumable.consume(item, event.name)) {
      return;
    }
    const imageUtils = editor.plugins.get('ImageUtils');
    const viewElement = conversionApi.mapper.toViewElement(item);
    const img = imageUtils.findViewImgElement(viewElement);

    // For width and height, update the saved value, removing 'px' value.
    if (data.attributeKey === 'width' || data.attributeKey === 'height') {
      writer.setAttribute(
        data.attributeKey,
        data.attributeNewValue.toString().replace('px', ''),
        img,
      );
    }

    // For a resized image, only the width event fires and the height derives
    // from the width.
    if (data.attributeKey === 'resizedWidth') {
      const resizedWidth = data.attributeNewValue;
      const originalWidth = item.getAttribute('width');
      const originalHeight = item.getAttribute('height');
      const [newWidth, newHeight] = _getResizedWidthHeight(originalWidth, originalHeight, resizedWidth);
      writer.removeAttribute('resizedWidth', img);
      writer.setAttribute('width', newWidth, img);
      writer.setAttribute('height', newHeight, img);
    }
  }

  return (dispatcher) => {
    const listenerAttributes = [
      'width',
      'height',
      'resizedWidth',
      // CKEditor only fires resizedWidth changes when using resizing handles.
      //'resizedHeight',
    ];
    listenerAttributes.forEach((attributeName) => {
      dispatcher.on('attribute:' + attributeName + ':imageInline', converter, {
        priority: 'high',
      });
      dispatcher.on('attribute:' + attributeName + ':imageBlock', converter, {
        priority: 'high',
      });
    });
  };
}

/**
 * Generates a callback that handles the data upcast for the img element.
 *
 * @return {function}
 *  Callback that binds an event to its parameter.
 *
 * @private
 */
function viewImageToModelImage(editor) {
  /**
   * Callback for the element:img event.
   *
   * Handles the Backdrop specific attributes.
   */
  function converter(event, data, conversionApi) {
    const { viewItem } = data;
    const { writer, consumable, safeInsert, updateConversionResult, schema } =
      conversionApi;
    const attributesToConsume = [];

    let image;

    // Not only check if a given `img` view element has been consumed, but also
    // verify it has `src` attribute present.
    if (!consumable.test(viewItem, { name: true, attributes: 'src' })) {
      return;
    }

    const hasDataCaption = consumable.test(viewItem, {
      name: true,
      attributes: 'data-caption',
    });

    // Create image that's allowed in the given context. If the image has a
    // caption, the image must be created as a block image to ensure the caption
    // is not lost on conversion. This is based on the assumption that
    // preserving the image caption is more important to the content creator
    // than preserving the wrapping element that doesn't allow block images.
    if (schema.checkChild(data.modelCursor, 'imageInline') && !hasDataCaption) {
      image = writer.createElement('imageInline', {
        src: viewItem.getAttribute('src'),
      });
    } else {
      image = writer.createElement('imageBlock', {
        src: viewItem.getAttribute('src'),
      });
    }

    // If the view element has a `data-align` attribute, convert that to a
    // CKEditor 5 Image Style. Note these are not related to Backdrop Image
    // Styles provided by Image module.
    // See https://ckeditor.com/docs/ckeditor5/latest/features/images/images-styles.html
    if (
      editor.plugins.has('ImageStyleEditing') &&
      consumable.test(viewItem, { name: true, attributes: 'data-align' })
    ) {
      const dataAlign = viewItem.getAttribute('data-align');
      const imageStyle = _getModelImageStyleFromDataAttribute(dataAlign);

      if (imageStyle) {
        writer.setAttribute('imageStyle', imageStyle, image);

        // Make sure the attribute can be consumed after successful `safeInsert`
        // operation.
        attributesToConsume.push('data-align');
      }
    }

    // Check if the view element has still unconsumed `data-caption` attribute.
    if (hasDataCaption) {
      // Create `caption` model element. Thanks to that element the rest of the
      // `ckeditor5-plugin` converters can recognize this image as a block image
      // with a caption.
      const caption = writer.createElement('caption');

      // Parse HTML from data-caption attribute and upcast it to model fragment.
      const viewFragment = editor.data.processor.toView(
        viewItem.getAttribute('data-caption'),
      );

      // Consumable must know about those newly parsed view elements.
      conversionApi.consumable.constructor.createFrom(
        viewFragment,
        conversionApi.consumable,
      );
      conversionApi.convertChildren(viewFragment, caption);

      // Insert the caption element into image, as a last child.
      writer.append(caption, image);

      // Make sure the attribute can be consumed after successful `safeInsert`
      // operation.
      attributesToConsume.push('data-caption');
    }

    // Save the file ID to the model.
    if (
      consumable.test(viewItem, { name: true, attributes: 'data-file-id' })
    ) {
      writer.setAttribute(
        'dataFileId',
        viewItem.getAttribute('data-file-id'),
        image,
      );
      attributesToConsume.push('data-file-id');
    }

    // If the view src is updated, also update the model.
    if (consumable.test(viewItem, { name: true, attributes: 'src' })) {
      writer.setAttribute(
        'src',
        viewItem.getAttribute('src'),
        image,
      );
      attributesToConsume.push('src');
    }

    // Try to place the image in the allowed position.
    if (!safeInsert(image, data.modelCursor)) {
      return;
    }

    // Mark given element as consumed. Now other converters will not process it
    // anymore.
    consumable.consume(viewItem, {
      name: true,
      attributes: attributesToConsume,
    });

    // Make sure `modelRange` and `modelCursor` is up-to-date after inserting
    // new nodes into the model.
    updateConversionResult(image, data);
  }

  return (dispatcher) => {
    dispatcher.on('element:img', converter, { priority: 'high' });
  };
}

/**
 * General HTML Support integration for attributes on links wrapping images.
 *
 * This plugin needs to integrate with GHS manually because upstream image link
 * plugin GHS integration assumes that the `<a>` element is inside the
 * `<imageBlock>` which is not true in the case of Backdrop.
 *
 * @param {module:html-support/datafilter~DataFilter} dataFilter
 *   The General HTML support data filter.
 *
 * @return {function}
 *   Callback that binds an event to its parameter.
 */
function upcastImageBlockLinkGhsAttributes(dataFilter) {
  /**
   * Callback for the element:img upcast event.
   *
   * @type {converterHandler}
   */
  function converter(event, data, conversionApi) {
    if (!data.modelRange) {
      return;
    }

    const viewImageElement = data.viewItem;
    const viewContainerElement = viewImageElement.parent;

    if (!viewContainerElement.is('element', 'a')) {
      return;
    }
    if (!data.modelRange.getContainedElement().is('element', 'imageBlock')) {
      return;
    }

    const viewAttributes = dataFilter.processViewAttributes(
      viewContainerElement,
      conversionApi,
    );

    if (viewAttributes) {
      conversionApi.writer.setAttribute(
        'htmlLinkAttributes',
        viewAttributes,
        data.modelRange,
      );
    }
  }

  return (dispatcher) => {
    dispatcher.on('element:img', converter, {
      priority: 'high',
    });
  };
}

/**
 * Modified implementation of the linkimageediting.js downcastImageLink().
 *
 * @return {function}
 *  Callback that binds an event to its parameter.
 *
 * @private
 */
function downcastBlockImageLink() {
  /**
   * Callback for the attribute:linkHref event.
   */
  function converter(event, data, conversionApi) {
    if (!conversionApi.consumable.consume(data.item, event.name)) {
      return;
    }

    // The image will be already converted - so it will be present in the view.
    const image = conversionApi.mapper.toViewElement(data.item);
    const writer = conversionApi.writer;

    // 1. Create an empty link element.
    const linkElement = writer.createContainerElement('a', {
      href: data.attributeNewValue,
    });
    // 2. Insert link before the associated image.
    writer.insert(writer.createPositionBefore(image), linkElement);
    // 3. Move the image into the link.
    writer.move(
      writer.createRangeOn(image),
      writer.createPositionAt(linkElement, 0),
    );

    // Modified alternative implementation of Generic HTML Support's
    // addBlockImageLinkAttributeConversion(). This is happening here as well to
    // avoid a race condition with the link element not yet existing.
    if (conversionApi.consumable.consume(data.item, 'attribute:htmlLinkAttributes:imageBlock')) {
      setViewAttributes(
        conversionApi.writer,
        data.item.getAttribute('htmlLinkAttributes'),
        linkElement,
      );
    }
  }

  return (dispatcher) => {
    dispatcher.on('attribute:linkHref:imageBlock', converter, {
      // downcastImageLink specifies 'high', so we need to go higher.
      priority: 'highest',
    });
  };
}

/**
 * CKEditor command that opens the Backdrop image editing dialog.
 */
class BackdropImageCommand extends CKEditor5.core.Command {
  /**
   * @inheritdoc
   */
  refresh() {
    const editor = this.editor;
    const imageUtils = editor.plugins.get('ImageUtils');
    const element = imageUtils.getClosestSelectedImageElement(this.editor.model.document.selection);
    this.isEnabled = true;
    this.value = !!element;
  }

  /**
   * Executes the command.
   */
  execute() {
    const editor = this.editor;
    const config = editor.config.get('backdropImage');
    const imageUtils = editor.plugins.get('ImageUtils');
    const ImageCaptionUtils = editor.plugins.get('ImageCaptionUtils');
    const model = editor.model;
    const imageElement = imageUtils.getClosestSelectedImageElement(model.document.selection);

    // Convert attributes to map for easier looping.
    const extraAttributes = new Map(Object.entries(config.extraAttributes));

    const uploadsEnabled = true; // @todo Set dynamically.
    let existingValues = {};

    if (imageElement) {
      // Most attributes can be directly mapped from the model.
      extraAttributes.forEach((attributeName, modelName) => {
        existingValues[attributeName] = imageElement.getAttribute(modelName);
      });

      // Convert the 'resizedWidth' internal attribute to be the 'width'
      // attribute. Also update 'height' to match.
      const resizedWidth = imageElement.getAttribute('resizedWidth');
      if (existingValues['width'] && existingValues['height'] && resizedWidth) {
        const [newWidth, newHeight] = _getResizedWidthHeight(existingValues['width'], existingValues['height'], resizedWidth);
        existingValues['width'] = newWidth;
        existingValues['height'] = newHeight;
      }

      // Alignment is stored as a CKEditor Image Style.
      const imageStyle = imageElement.getAttribute('imageStyle');
      const alignAttribute = _getDataAttributeFromModelImageStyle(imageStyle);
      existingValues['data-align'] = alignAttribute;

      // The image caption is stored outside the imageElement model and must
      // be retrieved to get its value.
      const imageCaption = ImageCaptionUtils.getCaptionFromImageModelElement(imageElement);
      existingValues['data-has-caption'] = !!imageCaption;
      if (imageCaption && imageCaption.childCount) {
        const captionValue = editor.data.processor.toData(imageCaption.getChild(0));
        existingValues['data-caption'] = captionValue;
      }
    }

    const saveCallback = (dialogValues) => {
      // Map the submitted form values to the CKEditor image model.
      let imageAttributes = {};
      extraAttributes.forEach((attributeName, modelName) => {
        if (dialogValues.attributes[attributeName] !== undefined) {
          imageAttributes[modelName] = dialogValues.attributes[attributeName];
        }
      });

      // Set CKEditor Image Style from the data-align attribute as imageStyle.
      const imageStyle = _getModelImageStyleFromDataAttribute(dialogValues.attributes['data-align']);
      imageAttributes['imageStyle'] = imageStyle;
      if (imageAttributes.hasOwnProperty('dataAlign')) {
        delete imageAttributes['dataAlign'];
      }

      // For updating an existing element:
      if (imageElement) {
        model.change((writer) => {
          writer.removeAttribute('resizedWidth', imageElement);
          writer.setAttributes(imageAttributes, imageElement);
        });

        // If width/height are empty, set to their natural values.
        if (imageAttributes['width'] === '' && imageAttributes['height'] === '') {
          imageUtils.setImageNaturalSizeAttributes(imageElement);
        }

        const imageCaption = ImageCaptionUtils.getCaptionFromImageModelElement(imageElement);
        // Remove an existing caption if disabled.
        if (imageCaption && !dialogValues.attributes['data-has-caption']) {
          editor.execute('toggleImageCaption');
        }
        // Add a caption if enabled and none yet exists.
        if (!imageCaption && dialogValues.attributes['data-has-caption']) {
          editor.execute('toggleImageCaption', { focusCaptionOnShow: true });
        }
      }
      // Inserting a new element:
      else {
        // An imageStyle key (even if undefined) on image insert will cause
        // conflicts in the Image Style plugin, so remove the attribute entirely
        // from the object.
        if (imageAttributes.hasOwnProperty('imageStyle') && !imageAttributes['imageStyle']) {
          delete imageAttributes['imageStyle'];
        }

        // Inserting an image has an unusual way of passing the attributes.
        // See https://ckeditor.com/docs/ckeditor5/latest/api/module_image_image_insertimagecommand-InsertImageCommand.html
        editor.execute('insertImage', { source: [imageAttributes] });

        // Toggle showing the caption after the image is inserted.
        if (dialogValues.attributes['data-has-caption']) {
          editor.execute('toggleImageCaption', { focusCaptionOnShow: true });
        }
      }
    };

    const dialogSettings = {
      title: config.insertLabel || 'Insert Image',
      uploads: uploadsEnabled,
      dialogClass: 'editor-image-dialog'
    };
    Backdrop.ckeditor5.openDialog(editor, config.dialogUrl, existingValues, saveCallback, dialogSettings);
  }
}

/**
 * CKEditor upload adapter that sends a request to Backdrop on file upload.
 *
 * Adapted from @ckeditor5/ckeditor5-upload/src/adapters/simpleuploadadapter
 *
 * @private
 * @implements module:upload/filerepository~UploadAdapter
 */
class BackdropImageUploadAdapter {
  /**
   * Creates a new adapter instance.
   *
   * @param {module:upload/filerepository~FileLoader} loader
   *   The file loader.
   * @param {module:core/editor/editor~Editor} editor
   *  The editor instance.
   * @param {module:upload/adapters/simpleuploadadapter~SimpleUploadConfig} options
   *   The upload options.
   */
  constructor(loader, editor, options) {
    /**
     * FileLoader instance to use during the upload.
     *
     * @member {module:upload/filerepository~FileLoader} #loader
     */
    this.loader = loader;

    /**
     * The configuration of the adapter.
     *
     * @member {module:core/editor/editor~Editor} #editor
     */
    this.editor = editor;

    /**
     * The configuration of the adapter.
     *
     * @member {module:upload/adapters/simpleuploadadapter~SimpleUploadConfig} #options
     */
    this.options = options;
  }

  /**
   * Starts the upload process.
   *
   * @see module:upload/filerepository~UploadAdapter#upload
   * @return {Promise}
   *   Promise that the upload will be processed.
   */
  upload() {
    return this.loader.file.then(
      (file) =>
        new Promise((resolve, reject) => {
          this._initRequest();
          this._initListeners(resolve, reject, file);
          this._sendRequest(file);
        }),
    );
  }

  /**
   * Aborts the upload process.
   *
   * @see module:upload/filerepository~UploadAdapter#abort
   */
  abort() {
    if (this.xhr) {
      this.xhr.abort();
    }
  }

  /**
   * Initializes the `XMLHttpRequest` object using the URL specified as
   *
   * {@link module:upload/adapters/simpleuploadadapter~SimpleUploadConfig#uploadUrl `simpleUpload.uploadUrl`} in the editor's
   * configuration.
   */
  _initRequest() {
    this.xhr = new XMLHttpRequest();

    this.xhr.open('POST', this.options.uploadUrl, true);
    this.xhr.responseType = 'json';
  }

  /**
   * Initializes XMLHttpRequest listeners
   *
   * @private
   *
   * @param {Function} resolve
   *  Callback function to be called when the request is successful.
   * @param {Function} reject
   *  Callback function to be called when the request cannot be completed.
   * @param {File} file
   *  Native File object.
   */
  _initListeners(resolve, reject, file) {
    const xhr = this.xhr;
    const loader = this.loader;
    const editor = this.editor;
    const genericErrorText = `Couldn't upload file: ${file.name}.`;
    const notification = editor.plugins.get('Notification');

    xhr.addEventListener('error', () => reject(genericErrorText));
    xhr.addEventListener('abort', () => reject());
    xhr.addEventListener('load', () => {
      const response = xhr.response;

      if (!response || !response.uploaded) {
        return reject(
          response && response.error && response.error.message
            ? response.error.message
            : genericErrorText,
        );
      }
      // There still may be notifications on upload, like resized images.
      if (response.error && response.error.message) {
        // The warning from Backdrop may contain <em> tags for placeholders.
        const markup = new DOMParser().parseFromString(response.error.message, 'text/html');

        // Set a warning for remaining notifications. This currently renders
        // as a window.alert() call, but may change to be better presented in
        // future versions of CKEditor 5.
        // See https://github.com/ckeditor/ckeditor5/issues/8934
        notification.showWarning(markup.body.textContent, {
          namespace: 'upload:image',
        });
      }

      // Resolve with the `urls` property and pass the response
      // to allow customizing the behavior of features relying on the upload
      // adapters.
      resolve({
        response,
        urls: { default: response.url },
      });
    });

    // Upload progress when it is supported.
    if (xhr.upload) {
      xhr.upload.addEventListener('progress', (evt) => {
        if (evt.lengthComputable) {
          loader.uploadTotal = evt.total;
          loader.uploaded = evt.loaded;
        }
      });
    }
  }

  /**
   * Prepares the data and sends the request.
   *
   * @param {File} file
   *   File instance to be uploaded.
   */
  _sendRequest(file) {
    // Set headers if specified.
    const headers = this.options.headers || {};

    // Use the withCredentials flag if specified.
    const withCredentials = this.options.withCredentials || false;

    Object.keys(headers).forEach((headerName) => {
      this.xhr.setRequestHeader(headerName, headers[headerName]);
    });

    this.xhr.withCredentials = withCredentials;

    // Prepare the form data.
    const data = new FormData();

    data.append('upload', file);

    // Send the request.
    this.xhr.send(data);
  }
}

})(CKEditor5);
