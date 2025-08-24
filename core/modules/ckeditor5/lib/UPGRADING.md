# Updating CKEditor 5

This directory contains CKEditor 5 UMD build, available via npm.

## Steps to update

1. Install npm if you do not have it on your system. The recommended way to get
   npm running is to install nvm (Node Version Manager), which in turn installs
   npm and allows you to switch between multiple different versions.
2. Download the latest version
   ```
   npm install --save ckeditor5
   ```
   Or, if you already have a previous version of ckeditor5 installed, update to
   the latest version:
   ```
   npm update
   ```
3. Copy relevant umd files (and only those) from translations directory:
   ```
   cp node_modules/ckeditor5/dist/translations/*umd.js PATH/TO/core/modules/ckeditor5/lib/ckeditor5/dist/translations/
   ```
4. Copy the relevant files to the dist folder of your dev branch
   ```
   cp node_modules/ckeditor5/dist/browser/ckeditor5.umd.js PATH/TO/core/modules/ckeditor5/lib/ckeditor5/dist/browser/
   cp node_modules/ckeditor5/dist/browser/ckeditor5.css PATH/TO/core/modules/ckeditor5/lib/ckeditor5/dist/browser/
   ```
5. Delete the map comment from lib/ckeditor5/dist/browser/ckeditor5.umd.js:
   That's the last line: `//# sourceMappingURL=ckeditor5.umd.js.map`
6. Update the CKEDITOR5_VERSION constant in ckeditor5.module

## Determine version number

```
npm ls
```
Or from package.json file.

## Updating Emoji version

New Emoji standards are released intermittently. The current version can be
found at https://www.unicode.org/emoji/charts/full-emoji-list.html

CKEditor by default retrieves a list of all available emoji from its CDN. But
using a remote asset may be blocked by a site's CORS configuration, so Backdrop
mirrors the list of emoji locally.

See https://ckeditor.com/docs/ckeditor5/latest/features/emoji.html#emoji-source
for information about changing the emoji source.

To update the emoji list, visit:

https://cdn.ckeditor.com/ckeditor5/data/emoji/16/en.json

Replacing "16" with the latest emoji version. Note that CKEditor only maintains
the emoji list in English currently.

Save this file into `lib/ckeditor5/build/emoji/en.json`.

## Testing

The Backdrop-specific integration points are the most likely things to encounter
problems when upgrading. Specifically test image uploading, image modification,
and linking functionality.
