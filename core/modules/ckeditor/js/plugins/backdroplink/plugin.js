/**
 * @file
 * Backdrop CKEditor 4 Link plugin.
 */

(function ($, Backdrop, CKEDITOR) {

"use strict";

function parseAttributes(editor, element) {
  var parsedAttributes = {};

  var domElement = element.$;
  var attribute;
  var attributeName;
  for (var attrIndex = 0; attrIndex < domElement.attributes.length; attrIndex++) {
    attribute = domElement.attributes.item(attrIndex);
    attributeName = attribute.nodeName.toLowerCase();
    // Ignore data-cke-* attributes; they're CKEditor internals.
    if (attributeName.indexOf('data-cke-') === 0) {
      continue;
    }
    // Store the value for this attribute, unless there's a data-cke-saved-
    // alternative for it, which will contain the quirk-free, original value.
    parsedAttributes[attributeName] = element.data('cke-saved-' + attributeName) || attribute.nodeValue;
  }

  // Remove any cke_* classes.
  if (parsedAttributes.class) {
    parsedAttributes.class = CKEDITOR.tools.trim(parsedAttributes.class.replace(/cke_\S+/, ''));
  }

  return parsedAttributes;
}

function getAttributes(editor, data) {
  var set = {};
  for (var attributeName in data) {
    if (data.hasOwnProperty(attributeName)) {
      set[attributeName] = data[attributeName];
    }
  }

  // CKEditor tracks the *actual* saved href in a data-cke-saved-* attribute
  // to work around browser quirks. We need to update it.
  set['data-cke-saved-href'] = set.href;

  // Remove all attributes which are not currently set.
  var removed = {};
  for (var s in set) {
    if (set.hasOwnProperty(s)) {
      delete removed[s];
    }
  }

  return {
    set: set,
    removed: CKEDITOR.tools.objectKeys(removed)
  };
}

CKEDITOR.plugins.add('backdroplink', {
  init: function (editor) {
    // Add the commands for link and unlink.
    editor.addCommand('backdroplink', {
      allowedContent: 'a[!href,target]',
      requiredContent: 'a[href]',
      modes: {wysiwyg: 1},
      canUndo: true,
      exec: function (editor) {
        var backdropImageUtils = CKEDITOR.plugins.backdropimage;
        var focusedImageWidget = backdropImageUtils && backdropImageUtils.getFocusedWidget(editor);
        // Get currently selected text.
        var selectedText = editor.getSelection().getSelectedText();
        // Get a currently selected link as CKEDITOR.dom.element
        var linkElement = getSelectedLink(editor);
        if (linkElement && (selectedText == "")) {
          // Get the text if cursor is somewhere in an existing link.
          selectedText = linkElement.$.text;
        }

        // Set existing attribute values based on selected element.
        var existingValues = {};
        if (linkElement && linkElement.$) {
          existingValues = parseAttributes(editor, linkElement);

          // Update the displayed link text
          existingValues.text = selectedText;
        }
        // Or, if an image widget is focused, we're editing a link wrapping
        // an image widget.
        else if (focusedImageWidget && focusedImageWidget.data.link) {
          existingValues = CKEDITOR.tools.clone(focusedImageWidget.data.link);
        }
        // Or, if selected element is not an existing link or an image.
        else {
          existingValues.text = selectedText;
        }

        // Prepare a save callback to be used upon saving the dialog.
        var saveCallback = function (returnValues) {
          editor.fire('saveSnapshot');
          // Ignore a disabled target attribute.
          if (returnValues.attributes.target === 0) {
            delete returnValues.attributes.target;
          }
          // If an image widget is focused, we're not editing an independent
          // link, but we're wrapping an image widget in a link.
          if (focusedImageWidget) {
            // Remove attributes that are inappropriate.
            delete returnValues.attributes['data-file-id'];
            delete returnValues.attributes.text;
            focusedImageWidget.setData('link', CKEDITOR.tools.extend(returnValues.attributes, focusedImageWidget.data.link));
            editor.fire('saveSnapshot');
            return;
          }
          // If not an image, replace text of link with new text.
          else {
            // Get the current selection.
            var selection = editor.getSelection();
            // Get the text of the current selection.
            var oldtext = selection.getSelectedText();
            // And the replacement text.
            var newtext = returnValues.attributes.text;
            // Get the range of the selection.
            var range = selection.getRanges(1)[0];

            // If the selection is a link, replace the text
            var element = selection.getStartElement();
            this.element = element;
            if (element.is('a')) {
              this.element.setText(newtext);
            }
          }

          // Create a new link element if needed.
          if (!linkElement && returnValues.attributes.href) {
            // Use link URL as text with a collapsed cursor.
            if (range.collapsed) {
              var text;
              if (newtext) {
                text = new CKEDITOR.dom.text(newtext);
              }
              else {
                // Use href as link text
                // Shorten mailto URLs to just the email address.
                text = new CKEDITOR.dom.text(returnValues.attributes.href.replace(/^mailto:/, ''), editor.document);
              }
              range.insertNode(text);
              range.selectNodeContents(text);
            }

            // Create the new link by applying a style to the new text.
            // Remove attributes that are not required.
            if (!returnValues.attributes['data-file-id']) {
              delete returnValues.attributes['data-file-id'];
            }
            delete returnValues.attributes.text;
            var style = new CKEDITOR.style({element: 'a', attributes: returnValues.attributes});
            style.type = CKEDITOR.STYLE_INLINE;
            style.applyToRange(range);
            range.select();

            // Set the link so individual properties may be set below.
            linkElement = getSelectedLink(editor);

            // Move the selection to after the link so the user may continue
            // typing outside of the link element itself.
            range.setStartAfter(linkElement);
            range.setEndAfter(linkElement);
            range.select();
          }
          // Update the link properties and remove redundant items.
          else if (linkElement) {
            for (var attrName in returnValues.attributes) {
              if (returnValues.attributes.hasOwnProperty(attrName)) {
                var value = returnValues.attributes[attrName];
                // Remove attribute data-file-id if 0 or null.
                if ((attrName == "data-file-id") && !(value > 0)){
                  linkElement.removeAttribute(attrName);
                }
                // Update the property if a value is specified.
                else if ((returnValues.attributes[attrName].length > 0) && (attrName !== "text")) {
                  linkElement.data('cke-saved-' + attrName, value);
                  linkElement.setAttribute(attrName, value);
                }
                // Delete the property if set to an empty string.
                else {
                  linkElement.removeAttribute(attrName);
                }
              }
            }
          }

          // Save snapshot for undo support.
          editor.fire('saveSnapshot');
        };
        // Backdrop.t() will not work inside CKEditor plugins because CKEditor
        // loads the JavaScript file instead of Backdrop. Pull translated strings
        // from the plugin settings that are translated server-side.
        var dialogSettings = {
          dialogClass: 'editor-link-dialog'
        };

        // Open the dialog for the edit form.
        Backdrop.ckeditor.openDialog(editor, editor.config.backdrop.linkDialogUrl, existingValues, saveCallback, dialogSettings);
      }
    });
    editor.addCommand('backdropunlink', {
      contextSensitive: 1,
      startDisabled: 1,
      allowedContent: 'a[!href]',
      requiredContent: 'a[href]',
      exec: function (editor) {
        var style = new CKEDITOR.style({element: 'a', type: CKEDITOR.STYLE_INLINE, alwaysRemoveElement: 1});
        editor.removeStyle(style);
      },
      refresh: function (editor, path) {
        var element = path.lastElement && path.lastElement.getAscendant('a', true);
        if (element && element.getName() === 'a' && element.getAttribute('href') && element.getChildCount()) {
          this.setState(CKEDITOR.TRISTATE_OFF);
        }
        else {
          this.setState(CKEDITOR.TRISTATE_DISABLED);
        }
      }
    });

    // CTRL + K.
    editor.setKeystroke(CKEDITOR.CTRL + 75, 'backdroplink');

    // Add buttons for link and unlink.
    if (editor.ui.addButton) {
      editor.ui.addButton('BackdropLink', {
        label: Backdrop.t('Link'),
        command: 'backdroplink',
        icon: this.path + '/link.png'
      });
      editor.ui.addButton('BackdropUnlink', {
        label: Backdrop.t('Unlink'),
        command: 'backdropunlink',
        icon: this.path + '/unlink.png'
      });
    }

    editor.on('doubleclick', function (evt) {
      var element = getSelectedLink(editor) || evt.data.element;

      if (!element.isReadOnly()) {
        if (element.is('a')) {
          editor.getSelection().selectElement(element);
          editor.getCommand('backdroplink').exec();
        }
      }
    });

    // If the "menu" plugin is loaded, register the menu items.
    if (editor.addMenuItems) {
      editor.addMenuItems({
        link: {
          label: Backdrop.t('Edit Link'),
          command: 'backdroplink',
          group: 'link',
          order: 1
        },

        unlink: {
          label: Backdrop.t('Unlink'),
          command: 'backdropunlink',
          group: 'link',
          order: 5
        }
      });
    }

    // If the "contextmenu" plugin is loaded, register the listeners.
    if (editor.contextMenu) {
      editor.contextMenu.addListener(function (element, selection) {
        if (!element || element.isReadOnly()) {
          return null;
        }
        var anchor = getSelectedLink(editor);
        if (!anchor) {
          return null;
        }

        var menu = {};
        if (anchor.getAttribute('href') && anchor.getChildCount()) {
          menu = {link: CKEDITOR.TRISTATE_OFF, unlink: CKEDITOR.TRISTATE_OFF};
        }
        return menu;
      });
    }
  }
});

/**
 * Get the surrounding link element of current selection.
 *
 * The following selection will all return the link element.
 *
 *  <a href="#">li^nk</a>
 *  <a href="#">[link]</a>
 *  text[<a href="#">link]</a>
 *  <a href="#">li[nk</a>]
 *  [<b><a href="#">li]nk</a></b>]
 *  [<a href="#"><b>li]nk</b></a>
 *
 * @param {CKEDITOR.editor} editor
 */
function getSelectedLink(editor) {
  // Get the current selection
  var selection = editor.getSelection();
  // Get the currently selected element.
  var selectedElement = selection.getSelectedElement();
  // If a selected element exists and is 'a'
  if (selectedElement && selectedElement.is('a')) {
    return selectedElement;
  }
  // Get the range of the current selection
  var range = selection.getRanges(true)[0];
  // If it has a value, decreases the range to make sure that boundaries
  // always anchor beside text nodes or the innermost element.
  if (range) {
    range.shrink(CKEDITOR.SHRINK_TEXT);
    // Find the node which fully contains the range.
    // Return an element path for the selection in the editor.
    // elementPath.contains = Search the path elements that meets the specified criteria.
    // getCommonAncestor = Find the node which fully contains the range.
    return editor.elementPath(range.getCommonAncestor()).contains('a', 1);
  }
  return null;
}

// Expose an API for other plugins to interact with backdroplink widgets.
// (Compatible with the official CKEditor link plugin's API:
// http://dev.ckeditor.com/ticket/13885.)
CKEDITOR.plugins.backdroplink = {
  parseLinkAttributes: parseAttributes,
  getLinkAttributes: getAttributes
};

})(jQuery, Backdrop, CKEDITOR);
