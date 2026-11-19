<?php
/**
 * \Backdrop\Sniffs\Classes\UnusedUseStatementSniff.
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
 * Checks for "use" statements that are not needed in a file.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class UnusedUseStatementSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_USE];

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

        // Only check use statements in the global scope.
        if (empty($tokens[$stackPtr]['conditions']) === false) {
            return;
        }

        // Seek to the end of the statement and get the string before the semi colon.
        $semiColon = $phpcsFile->findEndOfStatement($stackPtr);
        if ($tokens[$semiColon]['code'] !== T_SEMICOLON) {
            return;
        }

        $classPtr = $phpcsFile->findPrevious(
            Tokens::$emptyTokens,
            ($semiColon - 1),
            null,
            true
        );

        if ($tokens[$classPtr]['code'] !== T_STRING) {
            return;
        }

        // Search where the class name is used. PHP treats class names case
        // insensitive, that's why we cannot search for the exact class name string
        // and need to iterate over all T_STRING tokens in the file.
        $classUsed      = $phpcsFile->findNext(T_STRING, ($classPtr + 1));
        $lowerClassName = strtolower($tokens[$classPtr]['content']);

        // Check if the referenced class is in the same namespace as the current
        // file. If it is then the use statement is not necessary.
        $namespacePtr = $phpcsFile->findPrevious([T_NAMESPACE], $stackPtr);
        // Check if the use statement does aliasing with the "as" keyword. Aliasing
        // is allowed even in the same namespace.
        $aliasUsed = $phpcsFile->findPrevious(T_AS, ($classPtr - 1), $stackPtr);

        if ($namespacePtr !== false && $aliasUsed === false) {
            $nsEnd     = $phpcsFile->findNext(
                [
                    T_NS_SEPARATOR,
                    T_STRING,
                    T_WHITESPACE,
                ],
                ($namespacePtr + 1),
                null,
                true
            );
            $namespace = trim($phpcsFile->getTokensAsString(($namespacePtr + 1), ($nsEnd - $namespacePtr - 1)));

            $useNamespacePtr = $phpcsFile->findNext([T_STRING], ($stackPtr + 1));
            $useNamespaceEnd = $phpcsFile->findNext(
                [
                    T_NS_SEPARATOR,
                    T_STRING,
                ],
                ($useNamespacePtr + 1),
                null,
                true
            );
            $useNamespace    = rtrim($phpcsFile->getTokensAsString($useNamespacePtr, ($useNamespaceEnd - $useNamespacePtr - 1)), '\\');

            if (strcasecmp($namespace, $useNamespace) === 0) {
                $classUsed = false;
            }
        }//end if

        while ($classUsed !== false) {
            if (strtolower($tokens[$classUsed]['content']) === $lowerClassName) {
                // If the name is used in a PHP 7 function return type declaration
                // stop.
                if ($tokens[$classUsed]['code'] === T_RETURN_TYPE) {
                    return;
                }

                $beforeUsage = $phpcsFile->findPrevious(
                    Tokens::$emptyTokens,
                    ($classUsed - 1),
                    null,
                    true
                );
                // If a backslash is used before the class name then this is some other
                // use statement.
                if (in_array(
                    $tokens[$beforeUsage]['code'],
                    [
                        T_USE,
                        T_NS_SEPARATOR,
                    // If an object operator is used then this is a method call
                    // with the same name as the class name. Which means this is
                    // not referring to the class.
                        T_OBJECT_OPERATOR,
                    // Function definition, not class invocation.
                        T_FUNCTION,
                    // Static method call, not class invocation.
                        T_DOUBLE_COLON,
                    ]
                ) === false
                ) {
                    return;
                }

                // Trait use statement within a class.
                if ($tokens[$beforeUsage]['code'] === T_USE && empty($tokens[$beforeUsage]['conditions']) === false) {
                    return;
                }
            }//end if

            $classUsed = $phpcsFile->findNext([T_STRING, T_RETURN_TYPE], ($classUsed + 1));
        }//end while

        $warning = 'Unused use statement';
        $fix     = $phpcsFile->addFixableWarning($warning, $stackPtr, 'UnusedUse');
        if ($fix === true) {
            // Remove the whole use statement line.
            $phpcsFile->fixer->beginChangeset();
            for ($i = $stackPtr; $i <= $semiColon; $i++) {
                $phpcsFile->fixer->replaceToken($i, '');
            }

            // Also remove whitespace after the semicolon (new lines).
            while (isset($tokens[$i]) === true && $tokens[$i]['code'] === T_WHITESPACE) {
                $phpcsFile->fixer->replaceToken($i, '');
                if (strpos($tokens[$i]['content'], $phpcsFile->eolChar) !== false) {
                    break;
                }

                $i++;
            }

            // Replace @var data types in doc comments with the fully qualified class
            // name.
            $useNamespacePtr = $phpcsFile->findNext([T_STRING], ($stackPtr + 1));
            $useNamespaceEnd = $phpcsFile->findNext(
                [
                    T_NS_SEPARATOR,
                    T_STRING,
                ],
                ($useNamespacePtr + 1),
                null,
                true
            );
            $fullNamespace   = $phpcsFile->getTokensAsString($useNamespacePtr, ($useNamespaceEnd - $useNamespacePtr));

            $tag = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($stackPtr + 1));

            while ($tag !== false) {
                if (($tokens[$tag]['content'] === '@var' || $tokens[$tag]['content'] === '@return')
                    && isset($tokens[($tag + 1)]) === true
                    && $tokens[($tag + 1)]['code'] === T_DOC_COMMENT_WHITESPACE
                    && isset($tokens[($tag + 2)]) === true
                    && $tokens[($tag + 2)]['code'] === T_DOC_COMMENT_STRING
                    && strpos($tokens[($tag + 2)]['content'], $tokens[$classPtr]['content']) === 0
                ) {
                    $replacement = '\\'.$fullNamespace.substr($tokens[($tag + 2)]['content'], strlen($tokens[$classPtr]['content']));
                    $phpcsFile->fixer->replaceToken(($tag + 2), $replacement);
                }

                $tag = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($tag + 1));
            }

            $phpcsFile->fixer->endChangeset();
        }//end if

    }//end process()


}//end class
