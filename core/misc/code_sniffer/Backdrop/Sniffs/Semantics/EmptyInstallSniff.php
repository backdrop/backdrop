<?php
/**
 * \Backdrop\Sniffs\Semantics\EmptyInstallSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Throws an error if hook_install() or hook_uninstall() definitions are empty.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class EmptyInstallSniff extends FunctionDefinition
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
        $fileExtension = strtolower(substr($phpcsFile->getFilename(), -7));
        // Only check in *.install files.
        if ($fileExtension !== 'install') {
            return;
        }

        $fileName = substr(basename($phpcsFile->getFilename()), 0, -8);
        $tokens   = $phpcsFile->getTokens();
        if ($tokens[$stackPtr]['content'] === ($fileName.'_install')
            || $tokens[$stackPtr]['content'] === ($fileName.'_uninstall')
        ) {
            // Check if there is a function body.
            $bodyPtr = $phpcsFile->findNext(
                Tokens::$emptyTokens,
                ($tokens[$functionPtr]['scope_opener'] + 1),
                $tokens[$functionPtr]['scope_closer'],
                true
            );
            if ($bodyPtr === false) {
                $error = 'Empty installation hooks are not necessary';
                $phpcsFile->addError($error, $stackPtr, 'EmptyInstall');
            }
        }

    }//end processFunction()


}//end class
