<?php
/**
 * \Backdrop\Sniffs\Semantics\LStringTranslatableSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;

/**
 * Checks that string literals passed to l() are translatable.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class LStringTranslatableSniff extends FunctionCall
{


    /**
     * Returns an array of function names this test wants to listen for.
     *
     * @return array<string>
     */
    public function registerFunctionNames()
    {
        return ['l'];

    }//end registerFunctionNames()


    /**
     * Processes this function call.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile    The file being scanned.
     * @param int                         $stackPtr     The position of the function call in
     *                                                  the stack.
     * @param int                         $openBracket  The position of the opening
     *                                                  parenthesis in the stack.
     * @param int                         $closeBracket The position of the closing
     *                                                  parenthesis in the stack.
     *
     * @return void
     */
    public function processFunctionCall(
        File $phpcsFile,
        $stackPtr,
        $openBracket,
        $closeBracket
    ) {
        $tokens = $phpcsFile->getTokens();
        // Get the first argument passed to l().
        $argument = $this->getArgument(1);
        if ($tokens[$argument['start']]['code'] === T_CONSTANT_ENCAPSED_STRING
            // If the string starts with a HTML tag we don't complain.
            && $tokens[$argument['start']]['content'][1] !== '<'
        ) {
            $error = 'The $text argument to l() should be enclosed within t() so that it is translatable';
            $phpcsFile->addError($error, $stackPtr, 'LArg');
        }

    }//end processFunctionCall()


}//end class
