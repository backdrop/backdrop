/**
 * This is a Backdrop-optimized build of CKEditor.
 *
 * You may re-use it at any time at http://ckeditor.com/builder to build
 * CKEditor again. Alternatively, use the "build.sh" script to build it locally.
 * If you do so, be sure to pass it the "-s" flag. So: "sh build.sh -s".
 *
 * NOTE:
 *    This file is not used by CKEditor, you may remove it.
 *    Changing this file will not change your CKEditor configuration.
 */

/**
 * @license Copyright (c) 2003-2016, CKSource - Frederico Knabben. All rights reserved.
 * For licensing, see LICENSE.md or http://ckeditor.com/license
 */

/**
 * This file was added automatically by CKEditor builder.
 * You may re-use it at any time to build CKEditor again.
 *
 * If you would like to build CKEditor online again
 * (for example to upgrade), visit one the following links:
 *
 * (1) http://ckeditor.com/builder
 *     Visit online builder to build CKEditor from scratch.
 *
 * (2) http://ckeditor.com/builder/11819a2c5c3edb0be90cc07e1cc6a055
 *     Visit online builder to build CKEditor, starting with the same setup as before.
 *
 * (3) http://ckeditor.com/builder/download/11819a2c5c3edb0be90cc07e1cc6a055
 *     Straight download link to the latest version of CKEditor (Optimized) with the same setup as before.
 *
 * NOTE:
 *    This file is not used by CKEditor, you may remove it.
 *    Changing this file will not change your CKEditor configuration.
 */

var CKBUILDER_CONFIG = {
	skin: 'moono',
	preset: 'standard',
	ignore: [
		'.bender',
		'bender.js',
		'bender-err.log',
		'bender-out.log',
		'dev',
		'.DS_Store',
		'.editorconfig',
		'.gitattributes',
		'.gitignore',
		'gruntfile.js',
		'.idea',
		'.jscsrc',
		'.jshintignore',
		'.jshintrc',
		'less',
		'.mailmap',
		'node_modules',
		'package.json',
		'README.md',
		'tests',
		// Parts of CKEditor that we consciously don't ship with Backdrop.
		'adapters',
		'config.js',
		'contents.css',
		'Gruntfile.js',
		'styles.js',
		'samples',
		'skins/moono/readme.md'
	],
	plugins : {
		'a11yhelp' : 1,
		'about' : 1,
		'basicstyles' : 1,
		'blockquote' : 1,
		'clipboard' : 1,
		'contextmenu' : 1,
		'elementspath' : 1,
		'enterkey' : 1,
		'entities' : 1,
		'filebrowser' : 1,
		'floatingspace' : 1,
		'format' : 1,
		'horizontalrule' : 1,
		'htmlwriter' : 1,
		'image2' : 1,
		'indent' : 1,
		'indentlist' : 1,
		'justify' : 1,
		'list' : 1,
		'magicline' : 1,
		'maximize' : 1,
		'pastefromword' : 1,
		'pastetext' : 1,
		'removeformat' : 1,
		'resize' : 1,
		'sharedspace' : 1,
		'showblocks' : 1,
		'showborders' : 1,
		'sourcearea' : 1,
		'sourcedialog' : 1,
		'specialchar' : 1,
		'stylescombo' : 1,
		'tab' : 1,
		'table' : 1,
		'tableresize' : 1,
		'tabletools' : 1,
		'toolbar' : 1,
		'undo' : 1,
		'widget' : 1,
		'wysiwygarea' : 1
	},
	languages : {
		'en' : 1
	}
};
