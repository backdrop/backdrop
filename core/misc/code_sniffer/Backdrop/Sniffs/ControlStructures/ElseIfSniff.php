<?php
/**
 * Verifies that control statements conform to their coding standards.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\ControlStructures;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Checks that "elseif" is used instead of "else if".
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class ElseIfSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_ELSE];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token in the
     *                                               stack passed in $tokens.
     *
     * @return void
     */
    public function process(File $phpcsFile, $stackPtr)
    {

        $tokens = $phpcsFile->getTokens();

        $nextNonWhiteSpace = $phpcsFile->findNext(
            T_WHITESPACE,
            ($stackPtr + 1),
            null,
            true,
            null,
            true
        );

        if ($tokens[$nextNonWhiteSpace]['code'] === T_IF) {
            $fix = $phpcsFile->addFixableError('Use "elseif" in place of "else if"', $nextNonWhiteSpace, 'ElseIfDeclaration');
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                $phpcsFile->fixer->replaceToken($stackPtr, 'elseif');
                for ($i = ($stackPtr + 1); $i < $nextNonWhiteSpace; $i++) {
                    if ($tokens[$i]['code'] === T_WHITESPACE) {
                        $phpcsFile->fixer->replaceToken($i, '');
                    }
                }

                $phpcsFile->fixer->replaceToken($nextNonWhiteSpace, '');
                $phpcsFile->fixer->endChangeset();
            }
        }

    }//end process()


}//end class
