Entity Reference
================

Provides a field type that can reference arbitrary entities. For example, there 
could be "team" and "player" content types, and each player could have a field 
where authors could select which team they belong to.

Installation
------------

* Install this module using the official Backdrop CMS instructions at
  <https://backdropcms.org/guide/modules>

Configuration
-------------

2. To add an entity reference field to a node, navigate to Administration >
       Structure > Content types > [content type to edit] > Manage fields and
       add a new field. Add a title and select the entity reference field type.
       Select a widget: Select list, Autocomplete (Tags style), Autocomplete, or
       Check boxes/radio buttons. Save.
    3. From the Field Setting tab select the entity reference "Target type":
       Node, Comment, File, User, Taxonomy term, or Taxonomy vocabulary.
    4. From the Field Setting tab select select the Entity Selection Mode -
       Simple (with optional filter by bundle) is the default. Select how to
       Sort: Don't sort, A property of the base table of the entity, or A field
       attached to this entity. Select the sort field or property and the sort
       direction. Save.
Now when authoring content there is an option to make reference to another
entity.

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
