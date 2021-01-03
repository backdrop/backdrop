<?php
/**
 * \Backdrop\Sniffs\WhiteSpace\CommaSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\WhiteSpace;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * \Backdrop\Sniffs\WhiteSpace\CommaSniff.
 *
 * Checks that there is one space after a comma.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class CommaSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_COMMA];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     *
     * @return void
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        if (isset($tokens[($stackPtr + 1)]) === false) {
            return;
        }

        if ($tokens[($stackPtr + 1)]['code'] !== T_WHITESPACE
            && $tokens[($stackPtr + 1)]['code'] !== T_COMMA
            && $tokens[($stackPtr + 1)]['code'] !== T_CLOSE_PARENTHESIS
        ) {
            $error = 'Expected one space after the comma, 0 found';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'NoSpace');
            if ($fix === true) {
                $phpcsFile->fixer->addContent($stackPtr, ' ');
            }

            return;
        }

        if ($tokens[($stackPtr + 1)]['code'] === T_WHITESPACE
            && isset($tokens[($stackPtr + 2)]) === true
            && $tokens[($stackPtr + 2)]['line'] === $tokens[($stackPtr + 1)]['line']
            && $tokens[($stackPtr + 1)]['content'] !== ' '
        ) {
            $error = 'Expected one space after the comma, %s found';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'TooManySpaces', [strlen($tokens[($stackPtr + 1)]['content'])]);
            if ($fix === true) {
                $phpcsFile->fixer->replaceToken(($stackPtr + 1), ' ');
            }
        }

    }//end process()


}//end class
