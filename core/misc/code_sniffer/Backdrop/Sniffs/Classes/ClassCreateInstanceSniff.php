<?php
/**
 * Class create instance Test.
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
 * Class create instance Test.
 *
 * Checks the declaration of the class is correct.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class ClassCreateInstanceSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_NEW];

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

        $commaOrColon = $phpcsFile->findNext([T_SEMICOLON, T_COLON, T_COMMA], ($stackPtr + 1));
        if ($commaOrColon === false) {
            // Syntax error, nothing we can do.
            return;
        }

        // Search for an opening parenthesis in the current statement until the
        // next semicolon or comma.
        $nextParenthesis = $phpcsFile->findNext(T_OPEN_PARENTHESIS, ($stackPtr + 1), $commaOrColon);
        if ($nextParenthesis === false) {
            $error       = 'Calling class constructors must always include parentheses';
            $constructor = $phpcsFile->findNext(Tokens::$emptyTokens, ($stackPtr + 1), null, true, null, true);
            // We can invoke the fixer if we know this is a static constructor
            // function call or constructor calls with namespaces, example
            // "new \DOMDocument;" or constructor with class names in variables
            // "new $controller;".
            if ($tokens[$constructor]['code'] === T_STRING
                || $tokens[$constructor]['code'] === T_NS_SEPARATOR
                || ($tokens[$constructor]['code'] === T_VARIABLE
                && $tokens[($constructor + 1)]['code'] === T_SEMICOLON)
            ) {
                // Scan to the end of possible string\namespace parts.
                $nextConstructorPart = $constructor;
                while (true) {
                    $nextConstructorPart = $phpcsFile->findNext(
                        Tokens::$emptyTokens,
                        ($nextConstructorPart + 1),
                        null,
                        true,
                        null,
                        true
                    );
                    if ($nextConstructorPart === false
                        || ($tokens[$nextConstructorPart]['code'] !== T_STRING
                        && $tokens[$nextConstructorPart]['code'] !== T_NS_SEPARATOR)
                    ) {
                        break;
                    }

                    $constructor = $nextConstructorPart;
                }

                $fix = $phpcsFile->addFixableError($error, $constructor, 'ParenthesisMissing');
                if ($fix === true) {
                    $phpcsFile->fixer->addContent($constructor, '()');
                }

                // We can invoke the fixer if we know this is a
                // constructor call with class names in an array
                // example "new $controller[$i];".
            } else if ($tokens[$constructor]['code'] === T_VARIABLE
                && $tokens[($constructor + 1)]['code'] === T_OPEN_SQUARE_BRACKET
            ) {
                // Scan to the end of possible multilevel arrays.
                $nextConstructorPart = $constructor;
                do {
                    $nextConstructorPart = $tokens[($nextConstructorPart + 1)]['bracket_closer'];
                } while ($tokens[($nextConstructorPart + 1)]['code'] === T_OPEN_SQUARE_BRACKET);

                $fix = $phpcsFile->addFixableError($error, $nextConstructorPart, 'ParenthesisMissing');
                if ($fix === true) {
                    $phpcsFile->fixer->addContent($nextConstructorPart, '()');
                }
            } else {
                $phpcsFile->addError($error, $stackPtr, 'ParenthesisMissing');
            }//end if
        }//end if

    }//end process()


}//end class
