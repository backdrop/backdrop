<?php
/**
 * \Backdrop\Sniffs\WhiteSpace\ObjectOperatorSpacingSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\WhiteSpace;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Ensure that there are no white spaces before and after the object operator.
 *
 * Largely copied from
 * \PHP_CodeSniffer\Standards\Squiz\Sniffs\WhiteSpace\ObjectOperatorSpacingSniff
 * but modified to not throw errors on multi line statements.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class ObjectOperatorSpacingSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_OBJECT_OPERATOR];

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
        if ($tokens[($stackPtr - 1)]['code'] !== T_WHITESPACE) {
            $before = 0;
        } else {
            if ($tokens[($stackPtr - 2)]['line'] !== $tokens[$stackPtr]['line']) {
                $before = 'newline';
            } else {
                $before = $tokens[($stackPtr - 1)]['length'];
            }
        }

        if ($tokens[($stackPtr + 1)]['code'] !== T_WHITESPACE) {
            $after = 0;
        } else {
            if ($tokens[($stackPtr + 2)]['line'] !== $tokens[$stackPtr]['line']) {
                $after = 'newline';
            } else {
                $after = $tokens[($stackPtr + 1)]['length'];
            }
        }

        $phpcsFile->recordMetric($stackPtr, 'Spacing before object operator', $before);
        $phpcsFile->recordMetric($stackPtr, 'Spacing after object operator', $after);

        $prevToken = $phpcsFile->findPrevious(Tokens::$emptyTokens, ($stackPtr - 1), null, true);
        // Line breaks are allowed before an object operator.
        if ($before !== 0 && $tokens[$stackPtr]['line'] === $tokens[$prevToken]['line']) {
            $error = 'Space found before object operator';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'Before');
            if ($fix === true) {
                $phpcsFile->fixer->replaceToken(($stackPtr - 1), '');
            }
        }

        if ($after !== 0) {
            $error = 'Space found after object operator';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'After');
            if ($fix === true) {
                if ($after === 'newline') {
                    // Delete the operator on this line and insert it before the
                    // token on the next line.
                    $phpcsFile->fixer->beginChangeset();
                    $phpcsFile->fixer->replaceToken($stackPtr, '');
                    $phpcsFile->fixer->addContentBefore(($stackPtr + 2), '->');
                    $phpcsFile->fixer->endChangeset();
                } else {
                    $phpcsFile->fixer->replaceToken(($stackPtr + 1), '');
                }
            }
        }

    }//end process()


}//end class
