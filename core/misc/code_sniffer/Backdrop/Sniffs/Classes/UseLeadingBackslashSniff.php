<?php
/**
 * \Backdrop\Sniffs\Classes\UseLeadingBackslashSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Classes;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Use statements to import classes must not begin with "\".
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class UseLeadingBackslashSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_USE];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token in
     *                                               the stack passed in $tokens.
     *
     * @return void
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        // Only check use statements in the global scope.
        if (empty($tokens[$stackPtr]['conditions']) === false) {
            return;
        }

        $startPtr = $phpcsFile->findNext(
            Tokens::$emptyTokens,
            ($stackPtr + 1),
            null,
            true
        );

        if ($startPtr !== false && $tokens[$startPtr]['code'] === T_NS_SEPARATOR) {
            $error = 'When importing a class with "use", do not include a leading \\';
            $fix   = $phpcsFile->addFixableError($error, $startPtr, 'SeparatorStart');
            if ($fix === true) {
                $phpcsFile->fixer->replaceToken($startPtr, '');
            }
        }

    }//end process()


}//end class
