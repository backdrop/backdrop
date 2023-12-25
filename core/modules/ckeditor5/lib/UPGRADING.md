# Updating CKEditor 5

This directory contains a custom DLL build of CKEditor 5. Normal builds of
CKEditor, such as the one created through the CKEditor Online Builder Tool, must
be configured with desired plugins at the time it is compiled. Because Backdrop
needs to allow plugins to be loaded by individual modules, using the normal
builder tool is not suitable.

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
3. Copy the script from lib/scripts there and make it executable:
   ```
   cp [original/location/]concat-build.sh concat-build.sh
   chmod +x concat-build.sh
   ```
4. Run the script. This will download the DLL build via npm and concat the core
   and plugin builds into one file.
   ```
   ./concat-build.sh
   ```
5. Copy the resulting "ckeditor5-dll.js" file and the "translations" directory
   into the module directory lib/ckeditor5/build/
6. Update the CKEDITOR5_VERSION constant in ckeditor5.module

## Determine version number

When downloading via npm, the directory you run concat-build.sh in, will then
contain a package.json file, where you can find the version number.

## Testing

The Backdrop-specific integration points are the most likely things to encounter
problems when upgrading. Specifically test image uploading, image modification,
and linking functionality.
