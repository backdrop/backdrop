/**
 * @file
 * Prevent conflicts with CKEditor 4.
 *
 * Disable automatically attaching to all contenteditable elements. This
 * option can conflict with CKEditor 5, which also uses contenteditable.
 *
 * See https://ckeditor.com/docs/ckeditor4/latest/guide/dev_inline.html
 */
if (CKEDITOR && CKEDITOR.config) {
  // Disable looking for the default config.js file.
  CKEDITOR.config.customConfig = false;
  // Disable looking for the default styles.css file.
  CKEDITOR.config.contentsCss = false;
  // Disable looking for the default styles.js file.
  CKEDITOR.config.stylesSet = false;
  // Disable automatic attachment to contenteditable elements.
  CKEDITOR.disableAutoInline = true;
}
