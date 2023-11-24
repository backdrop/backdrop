CKEditor 5
==========
This module integrates CKEditor 5 with Backdrop CMS, providing a modern rich
text editor. This module is being developed in contrib as a candidate to be
included in a future version of Backdrop core.

Intended to be a full replacement for the existing CKEditor module (which was
based on CKEditor 4), this module provides matching functionality, including:

* Support for Backdrop link and image dialogs (in progress)
* Uploading images through copy/paste.
* Uploading files through the link dialog.
* Drag and drop toolbar configuration.
* Automatic configuration of text format HTML tags based on toolbar.

There is not yetany kind of upgrade path from CKEditor 4 to 5, other than
reconfiguring an editor to match the previous CKEditor 4 configuration.

Requirements
------------
Backdrop core. You can enable CKEditor 4 and 5 modules on the same site,
as long as they are configured on different text formats.

CKEditor 5 does not support Internet Explorer. An up-to-date browser is
required. See
[CKEditor Browser Compatibility](https://ckeditor.com/docs/ckeditor5/latest/support/browser-compatibility.html).


Installation
------------
- Install this module using the official Backdrop CMS instructions at
  https://docs.backdropcms.org/documentation/extend-with-modules.

- Visit the Text Editors and Formats page under Administration > Configuration >
  Content authoring (admin/config/content/formats).
- Create or edit a format. Under the "Editor" dropdown, select "CKEditor 5".
- Drag and drop toolbar items to configure a toolbar for the editor.
- Additional configuration settings for file and image uploading, style and
  heading lists are in a vertical tab set below the toolbar.

Issues
------
Bugs and Feature Requests should be reported in the Issue Queue:
https://github.com/backdrop-contrib/ckeditor5/issues.


Current Maintainers
-------------------
- [Nate Lampton](https://github.com/quicksketch)
- [Indigoxela](https://github.com/indigoxela)

Credits
-------
- Written for Backdrop CMS by [Nate Lampton](https://github.com/quicksketch).
- Most PHP code adapted from CKEditor (4) Backdrop CMS module.
- Image plugin adapted from Drupal 10 CKEditor 5 module.
- Link plugin adapted from [Drupal Advanced Editor Link 2.x](https://www.drupal.org/project/editor_advanced_link)


License
-------
This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
