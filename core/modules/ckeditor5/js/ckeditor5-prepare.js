/**
 * @file
 * Backwards compatibility of UMD build to integration via DLL builds.
 */
(function (CKEDITOR) {
  "use strict";

  /**
   * Prepare CKEditor 5 namespace variables for backwards-compatibility.
   *
   * CKEditor 5 versions 45 and higher stopped supporting the loading mechanism
   * Backdrop used previously (DLL builds) and changed the namespace used by
   * the editor from "CKEditor5" to "CKEDITOR".
   *
   * This mimics some of the DLL structure by adding aliases for commonly
   * extended classes. This does not mimic the entire DLL structure, only the
   * ones most likely to affect contributed modules. This must happen before any
   * custom plugin JS files load.
   */
  function prepareCKEditorNamespaces() {
    /**
     * @deprecated "CKEDITOR.core" does not exist in UMD builds.
     */
    CKEDITOR.core = {};
    CKEDITOR.core.Plugin = CKEDITOR.Plugin;
    CKEDITOR.core.Command = CKEDITOR.Command;

    /**
     * @deprecated "CKEDITOR.ui" does not exist in UMD builds.
     */
    CKEDITOR.ui = {};
    CKEDITOR.ui.ButtonView = CKEDITOR.ButtonView;

    // Use a different name to expose to global space - the one we used before.
    window.CKEditor5 = CKEDITOR;

    // Prevent conflict with CKEditor 4. Note this renaming depends on the
    // loading order, v4 has to run after this file.
    delete window.CKEDITOR;
  }

  prepareCKEditorNamespaces();

})(window.CKEDITOR);
