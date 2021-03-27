Entity Reference
================

Provides a field type that can reference arbitrary entities.

Installation
------------

* Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules

Configuration
-------------
Go to fields settings for an entity type such as for the page content type:
`admin/structure/types/manage/page/fields`. Add or edit an entity reference field.

Too many entities
-----------------
Note that when using a select widget, Entity reference loads all the
entities in that list in order to get the entity's label. If there are
too many loaded entities that site might reach its memory limit and crash
(also known as WSOD). In such a case you are advised to change the widget
to "autocomplete". If you get a WSOD when trying to edit the field
settings, you can reach the widget settings directly by navigation to

  `admin/structure/types/manage/[ENTITY-TYPE]/fields/[FIELD-NAME]/widget-type`

Replace ENTITY-TYPE and FIELD_NAME with the correct values.

Issues
------

To submit bug reports and feature suggestions, or to track changes:
  https://github.com/backdrop-contrib/entityreference/issues

Current Maintainers
-------------------

- [Herb v/d Dool](https://github.com/herbdool/)
- Seeking co-maintainers.

Credits
-------

- Ported to Backdrop by [Herb v/d Dool](https://github.com/herbdool/)
  and [Docwilmot](https://github.com/docwilmot).
- Originally developed for Drupal by [amitaibu](https://www.drupal.org/u/amitaibu) 
  and [Damien Tournoud](https://www.drupal.org/u/damien-tournoud).

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
