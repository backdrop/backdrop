Field Group
===========

This module will group a set of fields with different HTML wrappers on the
content type form and/or view. You can add field groups in several types with
their own format settings.

Fields can be dragged into groups with unlimited nesting. Each Field group
format comes with a configuration form, specific for that format type. Note that
field_group will only group fields, it can not be used to hide certain fields
since this a permission matter.

Some formats come in pairs. These types have an HTML wrapper to nest its field
group children. For example, place vertical tabs into the vertical tab group.
There is one exception to this rule: you can use a vertical tab without a
wrapper when the additional settings tabs are available, such as with content
type forms.

Available group types:

* Fieldset
* [HTML Details](https://developer.mozilla.org/en-US/docs/Web/HTML/Element/details)
* Vertical tabs
* Div
* HTML element

Field Group has API functions to add your own formatter and rendering for it.

Update
------

Some group types have been moved to a separate module [Field Group Extra](https://backdropcms.org/project/field_group_extra).
Install that module if you require: accordions, horizontal tabs or multipage
group types.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Maintainers
-----------

[Herb v/d Dool](https://github.com/herbdool)

Credit
------

Drupal 7 version:

* [stalski](http://drupal.org/user/322618)
* [swentel](http://drupal.org/user/107403)
* [zuuperman](http://drupal.org/user/361625)

Ported to Backdrop CMS by [Herb v/d Dool](https://github.com/herbdool).
