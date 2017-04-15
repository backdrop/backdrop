DESCRIPTION
===========
Provides a field type that can reference arbitrary entities.

SITE BUILDERS
=============
Note that when using a select widget, Entity reference loads all the
entities in that list in order to get the entity's label. If there are
too many loaded entities that site might reach its memory limit and crash
(also known as WSOD). In such a case you are advised to change the widget
to "autocomplete". If you get a WSOD when trying to edit the field
settings, you can reach the widget settings directly by navigation to

  admin/structure/types/manage/[ENTITY-TYPE]/fields/[FIELD-NAME]/widget-type

Replace ENTITY-TYPE and FIELD_NAME with the correct values.
