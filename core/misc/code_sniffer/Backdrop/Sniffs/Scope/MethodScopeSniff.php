<?php
/**
 * Verifies that class/interface/trait methods have scope modifiers.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Scope;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\AbstractScopeSniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Verifies that class/interface/trait methods have scope modifiers.
 *
 * Laregely copied from
 * \PHP_CodeSniffer\Standards\Squiz\Sniffs\Scope\MethodScopeSniff to work on
 * traits and have a fixer.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class MethodScopeSniff extends AbstractScopeSniff
{


    /**
     * Constructs a
     * \PHP_CodeSniffer\Standards\Squiz\Sniffs\Scope\MethodScopeSniff.
     */
    public function __construct()
    {
        parent::__construct([T_CLASS, T_INTERFACE, T_TRAIT], [T_FUNCTION]);

    }//end __construct()


    /**
     * Processes the function tokens within the class.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file where this token was found.
     * @param int                         $stackPtr  The position where the token was found.
     * @param int                         $currScope The current scope opener token.
     *
     * @return void
     */
    protected function processTokenWithinScope(File $phpcsFile, $stackPtr, $currScope)
    {
        $tokens = $phpcsFile->getTokens();

        $methodName = $phpcsFile->getDeclarationName($stackPtr);
        if ($methodName === null) {
            // Ignore closures.
            return;
        }

        if ($phpcsFile->hasCondition($stackPtr, T_FUNCTION) === true) {
            // Ignore nested functions.
            return;
        }

        $modifier = null;
        for ($i = ($stackPtr - 1); $i > 0; $i--) {
            if ($tokens[$i]['line'] < $tokens[$stackPtr]['line']) {
                break;
            } else if (isset(Tokens::$scopeModifiers[$tokens[$i]['code']]) === true) {
                $modifier = $i;
                break;
            }
        }

        if ($modifier === null) {
            $error = 'Visibility must be declared on method "%s"';
            $data  = [$methodName];
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'Missing', $data);

            if ($fix === true) {
                // No scope modifier means the method is public in PHP, fix that
                // to be explicitly public.
                $phpcsFile->fixer->addContentBefore($stackPtr, 'public ');
            }
        }

    }//end processTokenWithinScope()


    /**
     * Processes a token that is found outside the scope that this test is
     * listening to.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file where this token was found.
     * @param int                         $stackPtr  The position in the stack where this
     *                                               token was found.
     *
     * @return void
     */
    protected function processTokenOutsideScope(File $phpcsFile, $stackPtr)
    {

    }//end processTokenOutsideScope()


}//end class
