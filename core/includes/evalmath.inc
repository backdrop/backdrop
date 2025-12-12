<?php
/**
 * @file
 * ============================================================================
 * EvalMath - PHP Class to safely evaluate math expressions
 * Copyright (C) 2005 Miles Kaufmann <http://www.twmagic.com/>
 * ============================================================================
 *
 * NAME
 *   EvalMath - safely evaluate math expressions
 *
 * SYNOPSIS
 *   @code
 *   include('evalmath.class.php');
 *   $m = new EvalMath;
 *   // Basic evaluation:
 *   $result = $m->evaluate('2+2');
 *   // Supports: order of operation; parentheses; negation; built-in functions
 *   $result = $m->evaluate('-8(5/2)^2*(1-sqrt(4))-8');
 *   // Create your own variables
 *   $m->evaluate('a = e^(ln(pi))');
 *   // Or functions
 *   $m->evaluate('f(x,y) = x^2 + y^2 - 2x*y + 1');
 *   // And then use them
 *   $result = $m->evaluate('3*f(42,a)');
 *   @endcode
 *
 * DESCRIPTION
 *   Use the EvalMath class when you want to evaluate mathematical expressions
 *   from untrusted sources.  You can define your own variables and functions,
 *   which are stored in the object.  Try it, it's fun!
 *
 * METHODS
 *   @code
 *   $m->evaluate($expr)
 *      Evaluates the expression and returns the result.  If an error occurs,
 *      prints a warning and returns FALSE.  If $expr is a function assignment,
 *      returns TRUE on success.
 *
 *   $m->e($expr)
 *      A synonym for $m->evaluate().
 *
 *   $m->vars()
 *      Returns an associative array of all user-defined variables and values.
 *
 *   $m->funcs()
 *      Returns an array of all user-defined functions.
 *   @endcode
 *
 * PARAMETERS
 *   @code
 *   $m->suppress_errors
 *      Set to TRUE to turn off warnings when evaluating expressions

 *   $m->last_error
 *      If the last evaluation failed, contains a string describing the error.
 *      (Useful when suppress_errors is on).
 *   @endcode
 *
 * AUTHOR INFORMATION
 *  Copyright 2005, Miles Kaufmann.
 *
 *LICENSE
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:

 *  1   Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *  2.  Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *  3.  The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Safely evaluate math expressions.
 */
class EvalMath {
  public $suppress_errors = FALSE;
  public $last_error = NULL;

  /**
   * Publiciables (and constants)
   *
   * @var array
   */
  public $v = array('e' => 2.71, 'pi' => 3.14);

  /**
   * User-defined functions
   *
   * @var array
   */
  public $f = array();

  /**
   * Constants
   *
   * @var array
   */
  public $vb = array('e', 'pi');

  /**
   * Built-in functions
   *
   * @var array
   */
  public $fb = array(
    'sin', 'sinh', 'arcsin', 'asin', 'arcsinh', 'asinh',
    'cos', 'cosh', 'arccos', 'acos', 'arccosh', 'acosh',
    'tan', 'tanh', 'arctan', 'atan', 'arctanh', 'atanh',
    'pow', 'exp',
    'sqrt', 'abs', 'ln', 'log',
    'time', 'ceil', 'floor', 'min', 'max', 'round');

  /**
   * Construct.
   */
  public function __construct() {
    // Make the variables a little more accurate.
    $this->v['pi'] = pi();
    $this->v['e'] = exp(1);
  }

  /**
   * @param string $expr
   *   Expression.
   *
   * @return bool|null|void
   */
  public function e($expr) {
    return $this->evaluate($expr);
  }

  /**
   * @param string $expr
   *   Expression.
   *
   * @return bool|null|void
   */
  public function evaluate($expr) {
    $this->last_error = NULL;
    $expr = trim($expr);
    // Strip semicolons at the end.
    if (substr($expr, -1, 1) == ';') $expr = substr($expr, 0, strlen($expr) - 1);
    // ===============
    // Is it a variable assignment?
    if (preg_match('/^\s*([a-z]\w*)\s*=\s*(.+)$/', $expr, $matches)) {
      // Make sure we're not assigning to a constant.
      if (in_array($matches[1], $this->vb)) {
        return $this->trigger("cannot assign to constant '$matches[1]'");
      }
      // Get the result and make sure it's good.
      if (($tmp = $this->pfx($this->nfx($matches[2]))) === FALSE) return FALSE;
      // If so, stick it in the variable array.
      $this->v[$matches[1]] = $tmp;
      // And return the resulting value.
      return $this->v[$matches[1]];
      // ===============
    }
    // Is it a function assignment?
    elseif (preg_match('/^\s*([a-z]\w*)\s*\(\s*([a-z]\w*(?:\s*,\s*[a-z]\w*)*)\s*\)\s*=\s*(.+)$/', $expr, $matches)) {
      // Get the function name.
      $fnn = $matches[1];
      // Make sure it isn't built in.
      if (in_array($matches[1], $this->fb)) {
        return $this->trigger("cannot redefine built-in function '$matches[1]()'");
      }
      // Get the arguments.
      $args = explode(",", preg_replace("/\s+/", "", $matches[2]));
      // See if it can be converted to postfix.
      if (($stack = $this->nfx($matches[3])) === FALSE) return FALSE;
      // Freeze the state of the non-argument variables.
      for ($i = 0; $i < count($stack); $i++) {
        $token = $stack[$i];
        if (preg_match('/^[a-z]\w*$/', $token) && !in_array($token, $args)) {
          if (array_key_exists($token, $this->v)) {
            $stack[$i] = $this->v[$token];
          } else {
            return $this->trigger("undefined variable '$token' in function definition");
          }
        }
      }
      $this->f[$fnn] = array('args' => $args, 'func' => $stack);
      return TRUE;
      // ===============
    } else {
      // Straight up evaluation, woo.
      return $this->pfx($this->nfx($expr));
    }
  }

  /**
   * @return array
   */
  public function vars() {
    $output = $this->v;
    unset($output['pi']);
    unset($output['e']);
    return $output;
  }

  /**
   * @return array
   */
  public function funcs() {
    $output = array();
    foreach ($this->f as $fnn => $dat)
      $output[] = $fnn . '(' . implode(',', $dat['args']) . ')';
    return $output;
  }

  /**
   * Convert infix to postfix notation
   *
   * @param string $expr
   *   Expression.
   *
   * @return array|bool|void
   */
  private function nfx($expr) {

    $index = 0;
    $stack = new EvalMathStack;
    // Postfix form of expression, to be passed to pfx().
    $output = array();
    $expr = trim(strtolower($expr));

    $ops   = array('+', '-', '*', '/', '^', '_');
    // Right-associative operator?
    $ops_r = array('+' => 0, '-' => 0, '*' => 0, '/' => 0, '^' => 1);
    // Operator precedence.
    $ops_p = array('+' => 0, '-' => 0, '*' => 1, '/' => 1, '_' => 1, '^' => 2);

    // We use this in syntax-checking the expression and determining when a -
    // is a negation.
    $expecting_op = FALSE;

    // Make sure the characters are all good.
    if (preg_match("/[^\w\s+*^\/()\.,-]/", $expr, $matches)) {
      return $this->trigger("illegal character '{$matches[0]}'");
    }

    // 1 Infinite Loop ;)
    while (1) {
      // Get the first character at the current index.
      $op = substr($expr, $index, 1);
      // Find out if we're currently at the beginning of a number/variable/
      // function/parenthesis/operand.
      $ex = preg_match('/^([a-z]\w*\(?|\d+(?:\.\d*)?|\.\d+|\()/', substr($expr, $index), $match);
      // ===============
      // Is it a negation instead of a minus?
      if ($op == '-' && !$expecting_op) {
        // Put a negation on the stack.
        $stack->push('_');
        $index++;
      }
      // We have to explicitly deny this, because it's legal on the stack.
      elseif ($op == '_') {
        // But not in the input expression.
        return $this->trigger("illegal character '_'");
        // ===============
      }
      // Are we putting an operator on the stack?
      elseif ((in_array($op, $ops) || $ex) && $expecting_op) {
        if ($ex) {
          // Are we expecting an operator but have a number/variable/function/
          // opening parenthesis?
          // It's an implicit multiplication.
          $op = '*'; $index--;
        }
        // Heart of the algorithm:
        while ($stack->count > 0 && ($o2 = $stack->last()) && in_array($o2, $ops) && ($ops_r[$op] ? $ops_p[$op] < $ops_p[$o2] : $ops_p[$op] <= $ops_p[$o2])) {
          // Pop stuff off the stack into the output.
          $output[] = $stack->pop();
        }
        // Many thanks: http://en.wikipedia.org/wiki/Reverse_Polish_notation#The_algorithm_in_detail
        // Finally put OUR operator onto the stack.
        $stack->push($op);
        $index++;
        $expecting_op = FALSE;
        // ===============
      }
      // Ready to close a parenthesis?
      elseif ($op == ')' && $expecting_op) {
        // Pop off the stack back to the last (.
        while (($o2 = $stack->pop()) != '(') {
          if (is_null($o2)) return $this->trigger("unexpected ')'");
          else $output[] = $o2;
        }
        // Did we just close a function?
        if (preg_match("/^([a-z]\w*)\($/", $stack->last(2), $matches)) {
          // Get the function name.
          $fnn = $matches[1];
          // See how many arguments there were (cleverly stored on the stack,
          // thank you).
          $arg_count = $stack->pop();
          // Pop the function and push onto the output.
          $output[] = $stack->pop();
          if (in_array($fnn, $this->fb)) {
            // Check the argument count.
            if ($arg_count > 1)
              return $this->trigger("too many arguments ($arg_count given, 1 expected)");
          } elseif (array_key_exists($fnn, $this->f)) {
            if ($arg_count != count($this->f[$fnn]['args']))
              return $this->trigger("wrong number of arguments ($arg_count given, " . count($this->f[$fnn]['args']) . " expected)");
          } else {
            // Did we somehow push a non-function on the stack? this should
            // never happen.
            return $this->trigger("internal error");
          }
        }
        $index++;
        // ===============
      } elseif ($op == ',' && $expecting_op) {
        // Did we just finish a function argument?
        while (($o2 = $stack->pop()) != '(') {
          if (is_null($o2)) return $this->trigger("unexpected ','");
          // Oops, never had a ( pop the argument expression stuff and push onto
          // the output.
          else $output[] = $o2;
        }
        // Make sure there was a function.
        if (!preg_match("/^([a-z]\w*)\($/", $stack->last(2), $matches))
          return $this->trigger("unexpected ','");
        // Increment the argument count.
        $stack->push($stack->pop() + 1);
        // Put the ( back on, we'll need to pop back to it again.
        $stack->push('(');
        $index++;
        $expecting_op = FALSE;
        // ===============
      } elseif ($op == '(' && !$expecting_op) {
        // That was easy.
        $stack->push('(');
        $index++;
        // ===============
      } elseif ($ex && !$expecting_op) {
        // Do we now have a function/variable/number?
        $expecting_op = TRUE;
        $val = $match[1];
        if (preg_match("/^([a-z]\w*)\($/", $val, $matches)) {
          // May be func, or variable w/ implicit multiplication against
          // parentheses...
          if (in_array($matches[1], $this->fb) || array_key_exists($matches[1], $this->f)) {
            // It's a func.
            $stack->push($val);
            $stack->push(1);
            $stack->push('(');
            $expecting_op = FALSE;
          } else {
            // It's a var w/ implicit multiplication.
            $val = $matches[1];
            $output[] = $val;
          }
        } else {
          // It's a plain old var or num.
          $output[] = $val;
        }
        $index += strlen($val);
        // ===============
      } elseif ($op == ')') {
        // Miscellaneous error checking.
        return $this->trigger("unexpected ')'");
      } elseif (in_array($op, $ops) && !$expecting_op) {
        return $this->trigger("unexpected operator '$op'");
      } else {
        // I don't even want to know what you did to get here.
        return $this->trigger("an unexpected error occurred");
      }
      if ($index == strlen($expr)) {
        if (in_array($op, $ops)) {
          // Did we end with an operator? Bad.
          return $this->trigger("operator '$op' lacks operand");
        } else {
          break;
        }
      }
      while (substr($expr, $index, 1) == ' ') {
        // Step the index past whitespace (pretty much turns whitespace into
        // implicit multiplication if no operator is there).
        $index++;
      }

    }
    // Pop everything off the stack and push onto output.
    while (!is_null($op = $stack->pop())) {
      // If there are (s on the stack, ()s were unbalanced.
      if ($op == '(') return $this->trigger("expecting ')'");
      $output[] = $op;
    }
    return $output;
  }

  /**
   * Evaluate postfix notation.
   *
   * @param array $tokens
   *   Tokens list.
   * @param array $vars
   *   Variables list.
   */
  private function pfx(array $tokens, array $vars = array()) {

    if ($tokens == FALSE) return FALSE;

    $stack = new EvalMathStack;

    // Nice and easy.
    foreach ($tokens as $token) {
      // If the token is a binary operator, pop two values off the stack, do the
      // operation, and push the result back on.
      if (in_array($token, array('+', '-', '*', '/', '^'))) {
        if (is_null($op2 = $stack->pop())) return $this->trigger("internal error");
        if (is_null($op1 = $stack->pop())) return $this->trigger("internal error");
        switch ($token) {
          case '+':
            $stack->push($op1 + $op2);
            break;
          case '-':
            $stack->push($op1 - $op2);
            break;
          case '*':
            $stack->push($op1 * $op2);
            break;
          case '/':
            if ($op2 == 0) return $this->trigger("division by zero");
            $stack->push($op1 / $op2);
            break;
          case '^':
            $stack->push(pow($op1, $op2));
            break;
        }
        // If the token is a unary operator, pop one value off the stack, do the operation, and push it back on.
      } elseif ($token == "_") {
        $stack->push(-1 * $stack->pop());
        // If the token is a function, pop arguments off the stack, hand them to
        // the function, and push the result back on.
      } elseif (preg_match("/^([a-z]\w*)\($/", $token, $matches)) {
        // It's a function!
        $fnn = $matches[1];
        if (in_array($fnn, $this->fb)) {
          // Built-in function:
          if (is_null($op1 = $stack->pop())) return $this->trigger("internal error");
          // For the 'arc' trig synonyms.
          $fnn = preg_replace("/^arc/", "a", $fnn);
          if ($fnn == 'ln') $fnn = 'log';
          // Perfectly safe eval().
          eval('$stack->push(' . $fnn . '($op1));');
        } elseif (array_key_exists($fnn, $this->f)) { // User function.
          // Get args.
          $args = array();
          for ($i = count($this->f[$fnn]['args']) - 1; $i >= 0; $i--) {
            if (is_null($args[$this->f[$fnn]['args'][$i]] = $stack->pop())) return $this->trigger("internal error");
          }
          // Yay... recursion!!!!
          $stack->push($this->pfx($this->f[$fnn]['func'], $args));
        }
        // If the token is a number or variable, push it on the stack.
      } else {
        if (is_numeric($token)) {
          $stack->push($token);
        } elseif (array_key_exists($token, $this->v)) {
          $stack->push($this->v[$token]);
        } elseif (array_key_exists($token, $vars)) {
          $stack->push($vars[$token]);
        } else {
          return $this->trigger("undefined variable '$token'");
        }
      }
    }
    // When we're out of tokens, the stack should have a single element, the
    // final result.
    if ($stack->count != 1) return $this->trigger("internal error");
    return $stack->pop();
  }

  /**
   * Trigger an error, but nicely, if need be.
   *
   * @param string $msg
   *   Message.
   *
   * @return false|void
   */
  private function trigger($msg) {
    $this->last_error = $msg;
    if (!$this->suppress_errors) trigger_error($msg, E_USER_WARNING);
    return FALSE;
  }
}

/**
 * For internal use.
 */
class EvalMathStack {

  /**
   * @var array
   */
  public $stack = array();

  /**
   * @var int
   */
  public $count = 0;

  /**
   * Push
   */
  public function push($val) {
    $this->stack[$this->count] = $val;
    $this->count++;
  }

  /**
   * Pop
   */
  public function pop() {
    if ($this->count > 0) {
      $this->count--;
      return $this->stack[$this->count];
    }
    return NULL;
  }

  /**
   * Last
   */
  public function last($n = 1) {
    return !empty($this->stack[$this->count - $n]) ? $this->stack[$this->count - $n] : NULL;
  }
}
