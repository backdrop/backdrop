# Updating CKEditor 5

This directory contains CKEditor 5 UMD build.

## Steps to update

1. Install npm if you do not have it on your system. The recommended way to get
   npm running is to install nvm (Node Version Manager), which in turn installs
   npm and allows you to switch between multiple different versions.
2. Make a temporary working directory anywhere on your system. e.g.
   ```
   mkdir ~/ckeditor5-temp
   cd ~/ckeditor5-temp
   ```
3. Download the latest version
   ```
   npm install --save ckeditor5
   ```
4. Copy the translations directory recursively to your dev branch
   and plugin builds into one file.
   ```
   cp -R ... @todo
   ```
5. Copy the relevant files to the dist folder of your dev branch
   ```
   cp node_modules/ckeditor5/dist/browser/ckeditor5.umd.js ... @todo
   cp node_modules/ckeditor5... .css @todo
   ```
6. Update the CKEDITOR5_VERSION constant in ckeditor5.module

## Determine version number

When downloading via npm, the directory you run concat-build.sh in, will then
contain a package.json file, where you can find the version number.


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
