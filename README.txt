CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Requirements
 * Installation
 * Recommended Modules
 * Configuration
 * Maintainers


INTRODUCTION
------------

The Entity Reference module provides a field that can reference other entities.

 * For a full description of the module visit
   https://www.drupal.org/project/entityreference

 * To submit bug reports and feature suggestions, or to track changes visit
   https://www.drupal.org/project/issues/entityreference


REQUIREMENTS
------------

This module requires the following modules:

 * Entity API - https://www.drupal.org/project/entity
 * Chaos tool suite (ctools) - https://www.drupal.org/project/ctools


RECOMMENDED MODULES
-------------------

Modules extending Entity reference functionality:

 * Entity Reference View Widget -
   https://www.drupal.org/project/entityreference_view_widget
 * Entityreference Prepopulate -
   https://www.drupal.org/project/entityreference_prepopulate
 * Inline Entity Form - https://www.drupal.org/project/inline_entity_form


INSTALLATION
------------

 * Install the Entity Reference module as you would normally install a
   contributed Drupal module. Visit https://www.drupal.org/node/895232 for
   further information.


CONFIGURATION
--------------

    1. Navigate to Administration > Modules and enable the Entity Reference
       module and its dependencies.
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


Note that when using a select widget, Entity reference loads all the entities
in that list in order to get the entity's label. If there are too many loaded
entities that site might reach its memory limit and crash(also known as WSOD).
In such a case you are advised to change the widget to "autocomplete". If you
get a WSOD when trying to edit the field settings, you can reach the widget
settings directly by navigation to:

 * admin/structure/types/manage/[ENTITY-TYPE]/fields/[FIELD-NAME]/widget-type

Replace ENTITY-TYPE and FIELD_NAME with the correct values.

MAINTAINERS
-----------

 * Mathew Winstone (minorOffense) - https://www.drupal.org/u/minoroffense
 * Bojan Živanović (bojanz) - https://www.drupal.org/u/bojanz
 * David Pascoe-Deslauriers (spotzero) - https://www.drupal.org/u/spotzero
 * Amitai Burstein (amitaibu) - https://www.drupal.org/u/amitaibu

Supporting organizations:

 * Coldfront Labs Inc. - https://www.drupal.org/coldfront-labs-inc
