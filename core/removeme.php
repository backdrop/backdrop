<?php
/**
 * @file
 * Test phpstan output on a file added with this PR with some silly lines of
 * code.
 */

class BaseClass {
  public function doSomething() {
    return 'something done';
  }
}

class SubClass extends BaseClass {
  /**
   * @param bool $surprise
   * @return string
   */
  public function doSomething($surprise = TRUE) {
    if ($surprise) {
      $this->outofnothing = 'where does this come from?';
      return 'ouch';
    }
  }
}

$foo = new SubClass();
var_dump($foo->doSomething());
var_dump($foo->outofnothing);
