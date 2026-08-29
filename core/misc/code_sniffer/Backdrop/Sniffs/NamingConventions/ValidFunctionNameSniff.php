<?php
/**
 * \Backdrop\Sniffs\NamingConventions\ValidFunctionNameSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\NamingConventions;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Standards\Generic\Sniffs\NamingConventions\CamelCapsFunctionNameSniff;
use PHP_CodeSniffer\Util\Common;

/**
 * \Backdrop\Sniffs\NamingConventions\ValidFunctionNameSniff.
 *
 * Extends
 * \PHP_CodeSniffer\Standards\Generic\Sniffs\NamingConventions\CamelCapsFunctionNameSniff
 * to also check global function names outside the scope of classes and to not
 * allow methods beginning with an underscore.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class ValidFunctionNameSniff extends CamelCapsFunctionNameSniff
{


    /**
     * Processes the tokens within the scope.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being processed.
     * @param int                         $stackPtr  The position where this token was
     *                                               found.
     * @param int                         $currScope The position of the current scope.
     *
     * @return void
     */
    protected function processTokenWithinScope(File $phpcsFile, $stackPtr, $currScope)
    {
        $methodName = $phpcsFile->getDeclarationName($stackPtr);
        if ($methodName === null) {
            // Ignore closures.
            return;
        }

        $className = $phpcsFile->getDeclarationName($currScope);
        $errorData = [$className.'::'.$methodName];

        // Is this a magic method. i.e., is prefixed with "__" ?
        if (preg_match('|^__|', $methodName) !== 0) {
            $magicPart = strtolower(substr($methodName, 2));
            if (isset($this->magicMethods[$magicPart]) === false
                && isset($this->methodsDoubleUnderscore[$magicPart]) === false
            ) {
                $error = 'Method name "%s" is invalid; only PHP magic methods should be prefixed with a double underscore';
                $phpcsFile->addError($error, $stackPtr, 'MethodDoubleUnderscore', $errorData);
            }

            return;
        }

        $methodProps = $phpcsFile->getMethodProperties($stackPtr);
        if (Common::isCamelCaps($methodName, false, true, $this->strict) === false) {
            if ($methodProps['scope_specified'] === true) {
                $error = '%s method name "%s" is not in lowerCamel format';
                $data  = [
                    ucfirst($methodProps['scope']),
                    $errorData[0],
                ];
                $phpcsFile->addError($error, $stackPtr, 'ScopeNotCamelCaps', $data);
            } else {
                $error = 'Method name "%s" is not in lowerCamel format';
                $phpcsFile->addError($error, $stackPtr, 'NotCamelCaps', $errorData);
            }

            $phpcsFile->recordMetric($stackPtr, 'CamelCase method name', 'no');
            return;
        } else {
            $phpcsFile->recordMetric($stackPtr, 'CamelCase method name', 'yes');
        }

    }//end processTokenWithinScope()


    /**
     * Processes the tokens outside the scope.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being processed.
     * @param int                         $stackPtr  The position where this token was
     *                                               found.
     *
     * @return void
     */
    protected function processTokenOutsideScope(File $phpcsFile, $stackPtr)
    {
        $functionName = $phpcsFile->getDeclarationName($stackPtr);
        if ($functionName === null) {
            // Ignore closures.
            return;
        }

        $isApiFile     = substr($phpcsFile->getFilename(), -8) === '.api.php';
        $isHookExample = substr($functionName, 0, 5) === 'hook_';
        if ($isApiFile === true && $isHookExample === true) {
            // Ignore for examaple hook_ENTITY_TYPE_insert() functions in .api.php
            // files.
            return;
        }

        if ($functionName !== strtolower($functionName)) {
            $expected = strtolower(preg_replace('/([^_])([A-Z])/', '$1_$2', $functionName));
            $error    = 'Invalid function name, expected %s but found %s';
            $data     = [
                $expected,
                $functionName,
            ];
            $phpcsFile->addError($error, $stackPtr, 'InvalidName', $data);
        }

    }//end processTokenOutsideScope()


}//end class
