/**
 * @license Copyright (c) 2003-2025, CKSource Holding sp. z o.o. All rights reserved.
 * For licensing, see LICENSE.md or https://ckeditor.com/legal/ckeditor-licensing-options
 */

( e => {
const { [ 'translation' ]: { dictionary, getPluralForm } } = {"translation":{"dictionary":{},"getPluralForm":null}};
e[ 'translation' ] ||= { dictionary: {}, getPluralForm: null };
e[ 'translation' ].dictionary = Object.assign( e[ 'translation' ].dictionary, dictionary );
e[ 'translation' ].getPluralForm = getPluralForm;
} )( window.CKEDITOR_TRANSLATIONS ||= {} );
