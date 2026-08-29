<?php
/**
 * \Backdrop\Sniffs\Semantics\TInHookMenuSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Checks that t() is not used in hook_menu().
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class TInHookMenuSniff extends FunctionDefinition
{


    /**
     * Process this function definition.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile   The file being scanned.
     * @param int                         $stackPtr    The position of the function name in the stack.
     *                                                 name in the stack.
     * @param int                         $functionPtr The position of the function keyword in the stack.
     *                                                 keyword in the stack.
     *
     * @return void
     */
    public function processFunction(File $phpcsFile, $stackPtr, $functionPtr)
    {
        $fileExtension = strtolower(substr($phpcsFile->getFilename(), -6));
        // Only check in *.module files.
        if ($fileExtension !== 'module') {
            return;
        }

        $fileName = substr(basename($phpcsFile->getFilename()), 0, -7);
        $tokens   = $phpcsFile->getTokens();
        if ($tokens[$stackPtr]['content'] !== ($fileName.'_menu')) {
            return;
        }

        // Search in the function body for t() calls.
        $string = $phpcsFile->findNext(
            T_STRING,
            $tokens[$functionPtr]['scope_opener'],
            $tokens[$functionPtr]['scope_closer']
        );
        while ($string !== false) {
            if ($tokens[$string]['content'] === 't') {
                $opener = $phpcsFile->findNext(
                    Tokens::$emptyTokens,
                    ($string + 1),
                    null,
                    true
                );
                if ($opener !== false
                    && $tokens[$opener]['code'] === T_OPEN_PARENTHESIS
                ) {
                    $error = 'Do not use t() in hook_menu()';
                    $phpcsFile->addError($error, $string, 'TFound');
                }
            }

            $string = $phpcsFile->findNext(
                T_STRING,
                ($string + 1),
                $tokens[$functionPtr]['scope_closer']
            );
        }//end while

    }//end processFunction()


}//end class
