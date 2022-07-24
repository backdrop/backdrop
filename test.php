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

/**
 * None of the above still causes phpcs to nag, we have to be sloppier, but nagging still happens. Not with comments, though.
 */
function bar($one, $two = 'what', $three) {
    $test = ''; 
  switch ($two) {
    case 'a':
      $test= 'space';
    case 'b':
      $test ='nobreak';
  }
  $newvar = isset($three)?$three : null;
}