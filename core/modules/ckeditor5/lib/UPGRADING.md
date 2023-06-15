# Updating CKEditor 5

This directory contains a custom build of CKEditor 5. To update the library, a
new custom build must be downloaded from the
[online builder tool](https://ckeditor.com/ckeditor-5/online-builder/)

## Plugin List

Individual plugins must be selected when using the online builder. First, clear
out the entire plugin list, and then search and select the following plugins:

* Autoformat
* Block quote
* Bold
* Link
* Image
* Image upload
* Heading
* Image caption
* Image style
* Image toolbar
* Indent
* Italic
* List
* Media embed
* Paste from Office
* Table
* Table toolbar
* Text transformation
* Alignment
* Code
* Code blocks
* Horizontal lin
* HTML comment
* General HTML Support
* Remove format
* Source editing
* Special characters
* Special character essentials
* Style

## Remaining Build Steps

Ignore the configuration of the toolbar, that is controlled by Backdrop
separately.

Leave English as the default language. All other languages are included even
when not selected.

Download the final zip file containing the build.

## Replacing files

Extract the contents of the build zip file. Then copy it into the ckeditor5
module `lib/ckeditor5` subdirectory.

Delete the `sample` directory.

## Testing

The Backdrop-specific integration points are the most likely things to encounter
problems when upgrading. Specifically test image uploading, image modification,
and linking functionality.
