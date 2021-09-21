<?php
/**
 * @file
 * Some file to test.
 */

/**
 * Implements foo.
 */
function foo() {
  $foo = array('foo', 'bar', 'baz');
  $bar = array();
  foreach($foo as $ouch){
    // another ouch in a comment for several reasons
    $bar[] = $ouch;
  }
  // Another failure.
  $fooMyVariable = 'camelcase';
}
