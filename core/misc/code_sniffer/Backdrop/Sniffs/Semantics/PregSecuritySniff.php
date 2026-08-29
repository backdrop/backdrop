<?php
/**
 * \Backdrop\Sniffs\Semantics\PregSecuritySniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;

/**
 * Check the usage of the preg functions to ensure the insecure /e flag isn't
 * used: https://www.backdrop.org/node/750148
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class PregSecuritySniff extends FunctionCall
{


    /**
     * Returns an array of function names this test wants to listen for.
     *
     * @return array<string>
     */
    public function registerFunctionNames()
    {
        return [
            'preg_filter',
            'preg_grep',
            'preg_match',
            'preg_match_all',
            'preg_replace',
            'preg_replace_callback',
            'preg_split',
        ];

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
        $tokens   = $phpcsFile->getTokens();
        $argument = $this->getArgument(1);

        if ($argument === false) {
            return;
        }

        if ($tokens[$argument['start']]['code'] !== T_CONSTANT_ENCAPSED_STRING) {
            // Not a string literal.
            // @TODO: Extend code to recognize patterns in variables.
            return;
        }

        $pattern = $tokens[$argument['start']]['content'];
        $quote   = substr($pattern, 0, 1);
        // Check that the pattern is a string.
        if ($quote === '"' || $quote === "'") {
            // Get the delimiter - first char after the enclosing quotes.
            $delimiter = preg_quote(substr($pattern, 1, 1), '/');
            // Check if there is the evil e flag.
            if (preg_match('/'.$delimiter.'[\w]{0,}e[\w]{0,}$/', substr($pattern, 0, -1)) === 1) {
                $warn = 'Using the e flag in %s is a possible security risk. For details see https://www.backdrop.org/node/750148';
                $phpcsFile->addError(
                    $warn,
                    $argument['start'],
                    'PregEFlag',
                    [$tokens[$stackPtr]['content']]
                );
                return;
            }
        }

    }//end processFunctionCall()


}//end class
