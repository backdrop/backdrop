/**
 * @file
 * Prevent conflicts with CKEditor 4.
 *
 * Disable automatically attaching to all contenteditable elements. This
 * option can conflict with CKEditor 5, which also uses contenteditable.
 *
 * See https://ckeditor.com/docs/ckeditor4/latest/guide/dev_inline.html
 */
if (CKEDITOR) {
  CKEDITOR.disableAutoInline = false;
}
