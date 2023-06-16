# Updating CKEditor 5

This directory contains a custom DLL build of CKEditor 5. Normal builds of
CKEditor, such as the one created through the CKEditor Online Builder Tool, must
be compiled with all possible plugins at the time it is compiled. Because
Backdrop needs to allow plugins to be loaded by individual modules, using the
normal builder tool is not suitable.

A DLL (Dynamically Linked Library) build on the other hand allows plugins to
be loaded as separate individual files. When updating CKEditor 5, a new DLL
build must be created.

See https://ckeditor.com/docs/ckeditor5/latest/installation/advanced/alternative-setups/dll-builds.html#creating-a-dll-build

## Steps to update

1. Install npm if you do not have it on your system. The recommended way to get
   npm running is to install nvm (Node Version Manager), which in turn installs
   npm and allows you to switch between multiple different versions.
2. Make a temporary working directory anywhere on your system. e.g.
   ```
   mkdir ~/ckeditor5-temp
   cd ~/ckeditor5-temp
   ```
3. Install CKEditor and plugins through npm.
   ```
   npm install --save ckeditor5 \
     @ckeditor/ckeditor5-alignment \
     @ckeditor/ckeditor5-autoformat \
     @ckeditor/ckeditor5-basic-styles \
     @ckeditor/ckeditor5-block-quote \
     @ckeditor/ckeditor5-code-block \
     @ckeditor/ckeditor5-editor-classic \
     @ckeditor/ckeditor5-essentials \
     @ckeditor/ckeditor5-heading \
     @ckeditor/ckeditor5-horizontal-line \
     @ckeditor/ckeditor5-html-support \
     @ckeditor/ckeditor5-image \
     @ckeditor/ckeditor5-indent \
     @ckeditor/ckeditor5-language \
     @ckeditor/ckeditor5-link \
     @ckeditor/ckeditor5-list \
     @ckeditor/ckeditor5-paste-from-office \
     @ckeditor/ckeditor5-remove-format \
     @ckeditor/ckeditor5-source-editing \
     @ckeditor/ckeditor5-special-characters \
     @ckeditor/ckeditor5-style \
     @ckeditor/ckeditor5-table
   ```
4. Pull out each package's build and concatenate.

   This step is tedious and needs to be scripted. See this CKEditor core issue:
   https://github.com/ckeditor/ckeditor5/issues/10142

   Open the node_modules directory. Inside are the downloaded packages. Many of
   the CKEditor packages ship with a "build" directory. All these build
   directories need to be pulled out and reorganized to be used by Backdrop.

   For now, all the downloaded packages are concatenated together. Go through
   the `node_modules/ckeditor5` and `node_modules/@ckeditor` directories and find
   every `build` directory. Inside of it, copy the contents of the plugin .js
   file into a single text file. Start with the
   `node_modules/ckeditor5/build/ckeditor5-dll.js` file. Then add the rest of
   the build files from `node_modules/@ckeditor` in alphabetical order.

   Save the final file as `ckeditor.js`

6. Run webpack
   ```
   ./node_modules/webpack-cli/bin/cli.js
   ```

## Plugin List

Individual plugins must be selected when using the online builder. First, clear
out the entire plugin list, and then search and select the following plugins:


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

## Update version number

The downloaded archive from CKEditor.com includes the version number. For
example the archive may be named "ckeditor-38.0.1-kq1nhgi33tr8". In this case,
"38.0.1" is the version number. Edit ckeditor5.module and update the constant
`CKEDITOR5_VERSION` at the top of the file.

## Testing

The Backdrop-specific integration points are the most likely things to encounter
problems when upgrading. Specifically test image uploading, image modification,
and linking functionality.
