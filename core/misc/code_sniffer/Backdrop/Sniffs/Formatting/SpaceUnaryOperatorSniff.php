<?php
/**
 * \Backdrop\Sniffs\Formatting\SpaceUnaryOperatorSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Formatting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * \PHP_CodeSniffer\Standards\Generic\Sniffs\Formatting\SpaceUnaryOperatorSniff.
 *
 * Ensures there are no spaces on increment / decrement statements or on +/- sign
 * operators or "!" boolean negators.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class SpaceUnaryOperatorSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
         return [
             T_DEC,
             T_INC,
             T_MINUS,
             T_PLUS,
             T_BOOLEAN_NOT,
         ];

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

        // Check decrement / increment.
        if ($tokens[$stackPtr]['code'] === T_DEC || $tokens[$stackPtr]['code'] === T_INC) {
            $previous   = $phpcsFile->findPrevious(Tokens::$emptyTokens, ($stackPtr - 1), null, true);
            $modifyLeft = in_array(
                $tokens[$previous]['code'],
                [
                    T_VARIABLE,
                    T_CLOSE_SQUARE_BRACKET,
                    T_CLOSE_PARENTHESIS,
                    T_STRING,
                ]
            );

            if ($modifyLeft === true && $tokens[($stackPtr - 1)]['code'] === T_WHITESPACE) {
                $error = 'There must not be a single space before a unary operator statement';
                $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'IncDecLeft');
                if ($fix === true) {
                    $phpcsFile->fixer->replaceToken(($stackPtr - 1), '');
                }

                return;
            }

            if ($modifyLeft === false && $tokens[($stackPtr + 1)]['code'] === T_WHITESPACE) {
                $error = 'A unary operator statement must not be followed by a single space';
                $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'IncDecRight');
                if ($fix === true) {
                    $phpcsFile->fixer->replaceToken(($stackPtr + 1), '');
                }

                return;
            }
        }//end if

        // Check "!" operator.
        if ($tokens[$stackPtr]['code'] === T_BOOLEAN_NOT && $tokens[($stackPtr + 1)]['code'] === T_WHITESPACE) {
            $error = 'A unary operator statement must not be followed by a space';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'BooleanNot');
            if ($fix === true) {
                $phpcsFile->fixer->replaceToken(($stackPtr + 1), '');
            }

            return;
        }

        // Find the last syntax item to determine if this is an unary operator.
        $lastSyntaxItem        = $phpcsFile->findPrevious(
            [T_WHITESPACE],
            ($stackPtr - 1),
            (($tokens[$stackPtr]['column']) * -1),
            true,
            null,
            true
        );
        $operatorSuffixAllowed = in_array(
            $tokens[$lastSyntaxItem]['code'],
            [
                T_LNUMBER,
                T_DNUMBER,
                T_CLOSE_PARENTHESIS,
                T_CLOSE_CURLY_BRACKET,
                T_CLOSE_SQUARE_BRACKET,
                T_CLOSE_SHORT_ARRAY,
                T_VARIABLE,
                T_STRING,
            ]
        );

        // Check plus / minus value assignments or comparisons.
        if ($tokens[$stackPtr]['code'] === T_MINUS || $tokens[$stackPtr]['code'] === T_PLUS) {
            if ($operatorSuffixAllowed === false
                && $tokens[($stackPtr + 1)]['code'] === T_WHITESPACE
            ) {
                $error = 'A unary operator statement must not be followed by a space';
                $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'PlusMinus');
                if ($fix === true) {
                    $phpcsFile->fixer->replaceToken(($stackPtr + 1), '');
                }
            }
        }

    }//end process()


}//end class
