<?php
/**
 * \Backdrop\Sniffs\Semantics\FunctionWatchdogSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;

/**
 * Checks that the second argument to watchdog() is not enclosed with t().
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class FunctionWatchdogSniff extends FunctionCall
{


    /**
     * Returns an array of function names this test wants to listen for.
     *
     * @return array<string>
     */
    public function registerFunctionNames()
    {
        return ['watchdog'];

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
        // Get the second argument passed to watchdog().
        $argument = $this->getArgument(2);
        if ($argument === false) {
            $error = 'The second argument to watchdog() is missing';
            $phpcsFile->addError($error, $stackPtr, 'WatchdogArgument');
            return;
        }

        if ($tokens[$argument['start']]['code'] === T_STRING
            && $tokens[$argument['start']]['content'] === 't'
        ) {
            $error = 'The second argument to watchdog() should not be enclosed with t()';
            $phpcsFile->addError($error, $argument['start'], 'WatchdogT');
        }

        $concatFound = $phpcsFile->findNext(T_STRING_CONCAT, $argument['start'], $argument['end']);
        if ($concatFound !== false) {
            $error = 'Concatenating translatable strings is not allowed, use placeholders instead and only one string literal';
            $phpcsFile->addError($error, $concatFound, 'Concat');
        }

    }//end processFunctionCall()


}//end class
