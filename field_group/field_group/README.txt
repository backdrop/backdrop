
DONE
- Fix run-time error, max time exceeded. see drupal.org
- Make configuration work 
- refactor code
- Design and introduce hooks for field group format wrappers
- create javascript processors
- Create basic implementations for 
  * div 
  * fieldset 
  * vertical tabs 
  * accordion 
  * horizontal tabs
- make sure the group is not rendered is elements in it are empty
- Create an edit label thing
- Create delete field_group 
- Fix the menu system for fieldgroup

TODO
- add simpletests
- write patch for vertical tabs to work without form.inc (not needed. I got it to work in
  php form elements)
- make the vertical tabs work completely. Bug in default tabs.
