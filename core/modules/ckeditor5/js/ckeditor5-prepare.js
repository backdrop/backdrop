/**
 * @file
 * Backwards compatibility of UMD build to integration via DLL builds.
 */
(function () {
  "use strict";

  // Mimic some of the DLL structure by adding some aliases for commonly
  // extended classes. It won't be possible to mimic the entire DLL structure.
  // This must happen before any custom plugin js files load.
  CKEDITOR.core = {};// Does not exist in UMD.
  CKEDITOR.core.Plugin = CKEDITOR.Plugin;
  CKEDITOR.core.Command = CKEDITOR.Command;

  CKEDITOR.ui = {};// Does not exist in UMD.
  CKEDITOR.ui.ButtonView = CKEDITOR.ButtonView;

  // Use a different name to expose to global space - the one we used before.
  window.CKEditor5 = CKEDITOR;
  // Prevent conflict with CKE4, depends on order, v4 has to run after this.
  delete window.CKEDITOR;

})();
