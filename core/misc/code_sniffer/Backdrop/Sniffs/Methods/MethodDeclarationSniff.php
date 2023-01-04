<?php
/**
 * \Backdrop\Sniffs\Methods\MethodDeclarationSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Methods;

use PHP_CodeSniffer\Sniffs\AbstractScopeSniff;
use PHP_CodeSniffer\Standards\PSR2\Sniffs\Methods\MethodDeclarationSniff as PSR2MethodDeclarationSniff;

/**
 * Checks that the method declaration is correct.
 *
 * Extending
 * \PHP_CodeSniffer\Standards\PSR2\Sniffs\Methods\MethodDeclarationSniff
 * to also support traits.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class MethodDeclarationSniff extends PSR2MethodDeclarationSniff
{


    /**
     * Constructor.
     */
    public function __construct()
    {
        AbstractScopeSniff::__construct([T_CLASS, T_INTERFACE, T_TRAIT], [T_FUNCTION]);

    }//end __construct()


}//end class
