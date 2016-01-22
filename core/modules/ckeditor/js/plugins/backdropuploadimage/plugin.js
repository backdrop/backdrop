/**
 * @file
 * Allow direct upload by drag and dropping local images into CKEditor.
 */

'use strict';

(function ($) {

  CKEDITOR.plugins.add('backdropuploadimage', {
    requires: 'uploadwidget',

    onLoad: function () {
      CKEDITOR.addCss(
        '.cke_upload_uploading img{' +
        'opacity: 0.3' +
        '}'
      );
    },

    init: function (editor) {
      // Do not execute this paste listener if it will not be possible to upload file.
      if (!CKEDITOR.plugins.clipboard.isFileApiSupported) {
        return;
      }

      var entityType = (editor.element.$.attributes['data-dnd-context']) ? 'node' : 'file';
      var format = editor.config.backdrop.format;

      var fileTools = CKEDITOR.fileTools,
      //uploadUrl = fileTools.getUploadUrl( editor.config, 'image' );
        uploadUrl = Backdrop.settings.basePath + 'filter/uploadimage/' + entityType + '/' + format;

      // Handle images which are available in the dataTransfer.
      fileTools.addUploadWidget(editor, 'uploadimage', {
        supportedTypes: /image\/(jpeg|png|gif)/,

        uploadUrl: uploadUrl,

        fileToElement: function () {
          var img = new CKEDITOR.dom.element('img');
          img.setAttribute('src', loadingImage);
          return img;
        },

        parts: {
          img: 'img'
        },

        onUploading: function (upload) {
          // Show the image during the upload.
          this.parts.img.setAttribute('src', upload.data);
        },

        onUploaded: function (upload) {
          var range = editor.createRange();
          range.setStartBefore(this.wrapper);
          range.setEndAfter(this.wrapper);
          if (entityType == 'node') {
            var
              sid = upload.url,
              context = editor.element.$.attributes['data-dnd-context'].value;
            Backdrop.dnd.refreshLibraries();
            Backdrop.dnd.fetchNode(context, sid, function() {
              Backdrop.dnd.Nodes[sid].meta.entityType = entityType;
              Backdrop.dnd.Nodes[sid].meta.type = 'image';
              Backdrop.dnd.Nodes[sid].sas = '[node='+sid+':'+context+']';
              Backdrop.dnd.insertAtom(sid);
            });
          }
          else {
            var
              arr = upload.url.split('@'),
              fid = arr[0],
              url = arr[1],
              range = editor.getSelection().getRanges()[0],
              html = '<img alt="" src="'+url+'" data-file-id="'+fid+'">',
              element = CKEDITOR.htmlParser.fragment.fromHtml(html).children[0];
            // Turn it into a proper DOM element, and insert it.
            element = CKEDITOR.dom.element.createFromHtml(element.getOuterHtml());
            // ∆ image, not image2 !
            var widget = editor.widgets.initOn(element, "image", {'data-file-id': fid});
            editor.editable().insertElementIntoRange(widget.wrapper, range);
            widget.focus();
          }
        }
      });

      // Handle images which are not available in the dataTransfer.
      // This means that we need to read them from the <img src="data:..."> elements.
      editor.on('paste', function (evt) {
        // For performance reason do not parse data if it does not contain img tag and data attribute.
        if (!evt.data.dataValue.match(/<img[\s\S]+data:/i)) {
          return;
        }

        var data = evt.data,
        // Prevent XSS attacks.
          tempDoc = document.implementation.createHTMLDocument(''),
          temp = new CKEDITOR.dom.element(tempDoc.body),
          imgs, img, i;

        // Without this isReadOnly will not works properly.
        temp.data('cke-editable', 1);
        temp.appendHtml(data.dataValue);

        imgs = temp.find('img');
        for (i = 0; i < imgs.count(); i++) {
          img = imgs.getItem(i);

          // Image have to contain src=data:...
          var isDataInSrc = img.getAttribute('src') && img.getAttribute('src').substring(0, 5) == 'data:',
            isRealObject = img.data('cke-realelement') === null;

          // We are not uploading images in non-editable blocs and fake objects (#13003).
          if (isDataInSrc && isRealObject && !img.data('cke-upload-id') && !img.isReadOnly(1)) {
            var loader = editor.uploadRepository.create(img.getAttribute('src'));
            loader.upload(uploadUrl);
            fileTools.markElement(img, 'uploadimage', loader.id);
            fileTools.bindNotifications(editor, loader);
          }
        }

        data.dataValue = temp.getHtml();
      });
    }
  });

  // Black rectangle which is shown before image is loaded.
  var loadingImage = 'data:image/gif;base64,R0lGODlhDgAOAIAAAAAAAP///yH5BAAAAAAALAAAAAAOAA4AAAIMhI+py+0Po5y02qsKADs=';
})(jQuery);
