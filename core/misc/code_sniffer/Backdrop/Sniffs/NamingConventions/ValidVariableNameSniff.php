<?php
/**
 * \Backdrop\Sniffs\NamingConventions\ValidVariableNameSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\NamingConventions;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\AbstractVariableSniff;

/**
 * \Backdrop\Sniffs\NamingConventions\ValidVariableNameSniff.
 *
 * Checks the naming of member variables.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class ValidVariableNameSniff extends AbstractVariableSniff
{


    /**
     * Processes class member variables.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     *
     * @return void
     */
    protected function processMemberVar(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        $memberProps = $phpcsFile->getMemberProperties($stackPtr);
        if (empty($memberProps) === true) {
            return;
        }

        // Check if the class extends another class and get the name of the class
        // that is extended.
        if (empty($tokens[$stackPtr]['conditions']) === false) {
            $classPtr    = key($tokens[$stackPtr]['conditions']);
            $extendsName = $phpcsFile->findExtendedClassName($classPtr);

            // Special case config entities: those are allowed to have underscores in
            // their class property names. If a class extends something like
            // ConfigEntityBase then we consider it a config entity class and allow
            // underscores.
            if ($extendsName !== false && strpos($extendsName, 'ConfigEntity') !== false) {
                return;
            }

            // Plugin annotations may have underscores in class properties.
            // For example, see \Backdrop\Core\Field\Annotation\FieldFormatter.
            // The only class named "Plugin" in Backdrop core is
            // \Backdrop\Component\Annotation\Plugin while many Views plugins
            // extend \Backdrop\views\Annotation\ViewsPluginAnnotationBase.
            if ($extendsName !== false && in_array(
                $extendsName,
                [
                    'Plugin',
                    'ViewsPluginAnnotationBase',
                ]
            ) !== false
            ) {
                return;
            }

            $implementsNames = $phpcsFile->findImplementedInterfaceNames($classPtr);
            if ($implementsNames !== false && in_array('AnnotationInterface', $implementsNames) !== false) {
                return;
            }
        }//end if

        // The name of a property must start with a lowercase letter, properties
        // with underscores are not allowed, except the cases handled above.
        $memberName = ltrim($tokens[$stackPtr]['content'], '$');
        if (preg_match('/^[a-z]/', $memberName) === 1 && strpos($memberName, '_') === false) {
            return;
        }

        $error = 'Class property %s should use lowerCamel naming without underscores';
        $data  = [$tokens[$stackPtr]['content']];
        $phpcsFile->addError($error, $stackPtr, 'LowerCamelName', $data);

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
        $tokens = $phpcsFile->getTokens();

        $varName = ltrim($tokens[$stackPtr]['content'], '$');

        $phpReservedVars = [
            '_SERVER',
            '_GET',
            '_POST',
            '_REQUEST',
            '_SESSION',
            '_ENV',
            '_COOKIE',
            '_FILES',
            'GLOBALS',
        ];

        // If it's a php reserved var, then its ok.
        if (in_array($varName, $phpReservedVars) === true) {
            return;
        }

        // If it is a static public variable of a class, then its ok.
        if ($tokens[($stackPtr - 1)]['code'] === T_DOUBLE_COLON) {
            return;
        }

        if (preg_match('/^[A-Z]/', $varName) === 1) {
            $error = "Variable \"$varName\" starts with a capital letter, but only \$lowerCamelCase or \$snake_case is allowed";
            $phpcsFile->addError($error, $stackPtr, 'LowerStart');
        }

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
        // We don't care about variables in strings.
        return;

    }//end processVariableInString()


}//end class
