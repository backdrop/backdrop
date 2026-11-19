<?php
/**
 * \Backdrop\Sniffs\Classes\UseGlobalClassSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Classes;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Checks non-namespaced classes are referenced by FQN, not imported.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class UseGlobalClassSniff implements Sniff
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
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The PHP_CodeSniffer file where the
     *                                               token was found.
     * @param int                         $stackPtr  The position in the PHP_CodeSniffer
     *                                               file's token stack where the token
     *                                               was found.
     *
     * @return void|int Optionally returns a stack pointer. The sniff will not be
     *                  called again on the current file until the returned stack
     *                  pointer is reached. Return $phpcsFile->numTokens + 1 to skip
     *                  the rest of the file.
     */
    public function process(File $phpcsFile, $stackPtr)
    {

        $tokens = $phpcsFile->getTokens();

        // Find the first declaration, marking the end of the use statements.
        $bodyStart = $phpcsFile->findNext([T_CLASS, T_INTERFACE, T_TRAIT, T_FUNCTION], 0);

        // Ensure we are in the global scope, to exclude trait use statements.
        if (empty($tokens[$stackPtr]['conditions']) === false) {
            return;
        }

        // End of the full statement.
        $stmtEnd = $phpcsFile->findNext(T_SEMICOLON, $stackPtr);

        $lineStart = $stackPtr;
        // Iterate through a potential multiline use statement.
        while (false !== $lineEnd = $phpcsFile->findNext([T_SEMICOLON, T_COMMA], ($lineStart + 1), ($stmtEnd + 1))) {
            // We are only interested in imports that contain no backslash,
            // which means this is a class without a namespace.
            // Also skip function imports.
            if ($phpcsFile->findNext(T_NS_SEPARATOR, $lineStart, $lineEnd) !== false
                || $phpcsFile->findNext(T_STRING, $lineStart, $lineEnd, false, 'function') !== false
            ) {
                $lineStart = $lineEnd;
                continue;
            }

            // The first string token is the class name.
            $class     = $phpcsFile->findNext(T_STRING, $lineStart, $lineEnd);
            $className = $tokens[$class]['content'];
            // If there is more than one string token, the last one is the alias.
            $alias     = $phpcsFile->findPrevious(T_STRING, $lineEnd, $stackPtr);
            $aliasName = $tokens[$alias]['content'];

            $error = 'Non-namespaced classes/interfaces/traits should not be referenced with use statements';
            $fix   = $phpcsFile->addFixableError($error, $class, 'RedundantUseStatement');

            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();

                // Remove the entire line by default.
                $start = $lineStart;
                $end   = $lineEnd;
                $next  = $phpcsFile->findNext(T_WHITESPACE, ($end + 1), null, true);

                if ($tokens[$lineStart]['code'] === T_COMMA) {
                    // If there are lines before this one,
                    // then leave the ending delimiter in place.
                    $end = ($lineEnd - 1);
                } else if ($tokens[$lineEnd]['code'] === T_COMMA) {
                    // If there are lines after, but not before,
                    // then leave the use keyword.
                    $start = $class;
                } else if ($tokens[$next]['code'] === T_USE) {
                    // If the whole statement is removed, and there is one after it,
                    // then also remove the linebreaks.
                    $end = ($next - 1);
                }

                for ($i = $start; $i <= $end; $i++) {
                    $phpcsFile->fixer->replaceToken($i, '');
                }

                // Find all usages of the class, and add a leading backslash.
                // Only start looking after the end of the use statement block.
                $i = $bodyStart;
                while (false !== $i = $phpcsFile->findNext(T_STRING, ($i + 1), null, false, $aliasName)) {
                    if ($tokens[($i - 1)]['code'] !== T_NS_SEPARATOR) {
                        $phpcsFile->fixer->replaceToken($i, '\\'.$className);
                    }
                }

                $phpcsFile->fixer->endChangeset();
            }//end if

            $lineStart = $lineEnd;
        }//end while

    }//end process()


}//end class
