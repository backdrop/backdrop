/**
 * @file
 * Prevent ckeditor.js conflict.
 *
 * When both libraries are loaded, the basePath determination of CKEditor4
 * fails regularly. This variable is only used by v4 and prevents breaking init,
 * when the attached method has not run yet. For example, when both editor
 * versions are available in "Formatting options" and v5 is the default.
 *
 * This workaround does not prevent all problems with CKEDITOR's
 * resourceManager.load, but the editor will successfully initialize after
 * getting its settings.
 */
var CKEDITOR_BASEPATH = Backdrop.settings.basePath + 'core/misc/ckeditor/';
