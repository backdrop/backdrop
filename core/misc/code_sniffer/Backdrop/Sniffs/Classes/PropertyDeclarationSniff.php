<?php
/**
 * Verifies that properties are declared correctly.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Classes;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\AbstractVariableSniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Laregely copied from
 * \PHP_CodeSniffer\Standards\PSR2\Sniffs\Classes\PropertyDeclarationSniff to have a fixer
 * for the var keyword.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class PropertyDeclarationSniff extends AbstractVariableSniff
{


    /**
     * Processes the function tokens within the class.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file where this token was found.
     * @param int                         $stackPtr  The position where the token was found.
     *
     * @return void
     */
    protected function processMemberVar(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        if ($tokens[$stackPtr]['content'][1] === '_') {
            $error = 'Property name "%s" should not be prefixed with an underscore to indicate visibility';
            $data  = [$tokens[$stackPtr]['content']];
            $phpcsFile->addWarning($error, $stackPtr, 'Underscore', $data);
        }

        // Detect multiple properties defined at the same time. Throw an error
        // for this, but also only process the first property in the list so we don't
        // repeat errors.
        $find = Tokens::$scopeModifiers;
        $find = array_merge($find, [T_VARIABLE, T_VAR, T_SEMICOLON]);
        $prev = $phpcsFile->findPrevious($find, ($stackPtr - 1));
        if ($tokens[$prev]['code'] === T_VARIABLE) {
            return;
        }

        if ($tokens[$prev]['code'] === T_VAR) {
            $error = 'The var keyword must not be used to declare a property';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'VarUsed');
            if ($fix === true) {
                $phpcsFile->fixer->replaceToken($prev, 'public');
            }
        }

        $next = $phpcsFile->findNext([T_VARIABLE, T_SEMICOLON], ($stackPtr + 1));
        if ($tokens[$next]['code'] === T_VARIABLE) {
            $error = 'There must not be more than one property declared per statement';
            $phpcsFile->addError($error, $stackPtr, 'Multiple');
        }

        $modifier = $phpcsFile->findPrevious(Tokens::$scopeModifiers, $stackPtr);
        if (($modifier === false) || ($tokens[$modifier]['line'] !== $tokens[$stackPtr]['line'])) {
            $error = 'Visibility must be declared on property "%s"';
            $data  = [$tokens[$stackPtr]['content']];
            $phpcsFile->addError($error, $stackPtr, 'ScopeMissing', $data);
        }

    }//end processMemberVar()


    /**
     * Processes normal variables.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file where this token was found.
     * @param int                         $stackPtr  The position where the token was found.
     *
     * @return void
     */
    protected function processVariable(File $phpcsFile, $stackPtr)
    {
        /*
            We don't care about normal variables.
        */

    }//end processVariable()


    /**
     * Processes variables in double quoted strings.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file where this token was found.
     * @param int                         $stackPtr  The position where the token was found.
     *
     * @return void
     */
    protected function processVariableInString(File $phpcsFile, $stackPtr)
    {
        /*
            We don't care about normal variables.
        */

    }//end processVariableInString()


}//end class
