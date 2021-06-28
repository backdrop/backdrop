<?php
/**
 * \Backdrop\Sniffs\WhiteSpace\EmptyLinesSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\WhiteSpace;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * \Backdrop\Sniffs\WhiteSpace\EmptyLinesSniff.
 *
 * Checks that there are not more than 2 empty lines following each other.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class EmptyLinesSniff implements Sniff
{

    /**
     * A list of tokenizers this sniff supports.
     *
     * @var array<string>
     */
    public $supportedTokenizers = [
        'PHP',
        'JS',
        'CSS',
    ];


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_WHITESPACE];

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
        if ($tokens[$stackPtr]['content'] === $phpcsFile->eolChar
            && isset($tokens[($stackPtr + 1)]) === true
            && $tokens[($stackPtr + 1)]['content'] === $phpcsFile->eolChar
            && isset($tokens[($stackPtr + 2)]) === true
            && $tokens[($stackPtr + 2)]['content'] === $phpcsFile->eolChar
            && isset($tokens[($stackPtr + 3)]) === true
            && $tokens[($stackPtr + 3)]['content'] === $phpcsFile->eolChar
        ) {
            $error = 'More than 2 empty lines are not allowed';
            $phpcsFile->addError($error, ($stackPtr + 3), 'EmptyLines');
        }

    }//end process()


}//end class
