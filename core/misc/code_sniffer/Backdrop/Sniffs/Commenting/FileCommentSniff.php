<?php
/**
 * Parses and verifies the doc comments for files.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Commenting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Parses and verifies the doc comments for files.
 *
 * Verifies that :
 * <ul>
 *  <li>A doc comment exists.</li>
 *  <li>There is a blank newline after the @file statement.</li>
 * </ul>
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

class FileCommentSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_OPEN_TAG];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     *
     * @return int
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        $tokens       = $phpcsFile->getTokens();
        $commentStart = $phpcsFile->findNext(T_WHITESPACE, ($stackPtr + 1), null, true);

        // Files containing exactly one class, interface or trait are allowed to
        // ommit a file doc block. If a namespace is used then the file comment must
        // be omitted.
        $oopKeyword = $phpcsFile->findNext([T_CLASS, T_INTERFACE, T_TRAIT], $stackPtr);
        if ($oopKeyword !== false) {
            $namespace = $phpcsFile->findNext(T_NAMESPACE, $stackPtr);
            // Check if the file contains multiple classes/interfaces/traits - then a
            // file doc block is allowed.
            $secondOopKeyword = $phpcsFile->findNext([T_CLASS, T_INTERFACE, T_TRAIT], ($oopKeyword + 1));
            // Namespaced classes, interfaces and traits should not have an @file doc
            // block.
            if (($tokens[$commentStart]['code'] === T_DOC_COMMENT_OPEN_TAG
                || $tokens[$commentStart]['code'] === T_COMMENT)
                && $secondOopKeyword === false
                && $namespace !== false
            ) {
                if ($tokens[$commentStart]['code'] === T_COMMENT) {
                    $phpcsFile->addError('Namespaced classes, interfaces and traits should not begin with a file doc comment', $commentStart, 'NamespaceNoFileDoc');
                } else {
                    $fix = $phpcsFile->addFixableError('Namespaced classes, interfaces and traits should not begin with a file doc comment', $commentStart, 'NamespaceNoFileDoc');
                    if ($fix === true) {
                        $phpcsFile->fixer->beginChangeset();

                        for ($i = $commentStart; $i <= ($tokens[$commentStart]['comment_closer'] + 1); $i++) {
                            $phpcsFile->fixer->replaceToken($i, '');
                        }

                        // If, after removing the comment, there are two new lines
                        // remove them.
                        if ($tokens[($commentStart - 1)]['content'] === "\n" && $tokens[$i]['content'] === "\n") {
                            $phpcsFile->fixer->replaceToken($i, '');
                        }

                        $phpcsFile->fixer->endChangeset();
                    }
                }
            }//end if

            if ($namespace !== false) {
                return ($phpcsFile->numTokens + 1);
            }

            // Search for global functions before and after the class.
            $function = $phpcsFile->findPrevious(T_FUNCTION, ($oopKeyword - 1));
            if ($function === false) {
                $function = $phpcsFile->findNext(T_FUNCTION, ($tokens[$oopKeyword]['scope_closer'] + 1));
            }

            $fileTag = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($commentStart + 1), null, false, '@file');

            // No other classes, no other global functions and no explicit @file tag
            // anywhere means it is ok to skip the file comment.
            if ($secondOopKeyword === false && $function === false && $fileTag === false) {
                return ($phpcsFile->numTokens + 1);
            }
        }//end if

        if ($tokens[$commentStart]['code'] === T_COMMENT) {
            $fix = $phpcsFile->addFixableError('You must use "/**" style comments for a file comment', $commentStart, 'WrongStyle');
            if ($fix === true) {
                $content = $tokens[$commentStart]['content'];

                // If the comment starts with something like "/**" then we just
                // insert a space after the stars.
                if (strpos($content, '/**') === 0) {
                    $phpcsFile->fixer->replaceToken($commentStart, str_replace('/**', '/** ', $content));
                } else if (strpos($content, '/*') === 0) {
                    // Just turn the /* ... */ style comment into a /** ... */ style
                    // comment.
                    $phpcsFile->fixer->replaceToken($commentStart, str_replace('/*', '/**', $content));
                } else {
                    $content = trim(ltrim($tokens[$commentStart]['content'], '/# '));
                    $phpcsFile->fixer->replaceToken($commentStart, "/**\n * @file\n * $content\n */\n");
                }
            }

            return ($phpcsFile->numTokens + 1);
        } else if ($commentStart === false || $tokens[$commentStart]['code'] !== T_DOC_COMMENT_OPEN_TAG) {
            $fix = $phpcsFile->addFixableError('Missing file doc comment', 0, 'Missing');
            if ($fix === true) {
                // Only PHP has a real opening tag, additional newline at the
                // beginning here.
                if ($phpcsFile->tokenizerType === 'PHP') {
                    // In templates add the file doc block to the very beginning of
                    // the file.
                    if ($tokens[0]['code'] === T_INLINE_HTML) {
                        $phpcsFile->fixer->addContentBefore(0, "<?php\n\n/**\n * @file\n */\n?>\n");
                    } else {
                        $phpcsFile->fixer->addContent($stackPtr, "\n/**\n * @file\n */\n");
                    }
                } else {
                    $phpcsFile->fixer->addContent($stackPtr, "/**\n * @file\n */\n");
                }
            }

            return ($phpcsFile->numTokens + 1);
        }//end if

        $commentEnd = $tokens[$commentStart]['comment_closer'];
        $fileTag    = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($commentStart + 1), $commentEnd, false, '@file');
        $next       = $phpcsFile->findNext(T_WHITESPACE, ($commentEnd + 1), null, true);

        // If there is no @file tag and the next line is a function or class
        // definition then the file docblock is mising.
        if ($tokens[$next]['line'] === ($tokens[$commentEnd]['line'] + 1)
            && $tokens[$next]['code'] === T_FUNCTION
        ) {
            if ($fileTag === false) {
                $fix = $phpcsFile->addFixableError('Missing file doc comment', $stackPtr, 'Missing');
                if ($fix === true) {
                    // Only PHP has a real opening tag, additional newline at the
                    // beginning here.
                    if ($phpcsFile->tokenizerType === 'PHP') {
                        $phpcsFile->fixer->addContent($stackPtr, "\n/**\n * @file\n */\n");
                    } else {
                        $phpcsFile->fixer->addContent($stackPtr, "/**\n * @file\n */\n");
                    }
                }

                return ($phpcsFile->numTokens + 1);
            }
        }//end if

        if ($fileTag === false || $tokens[$fileTag]['line'] !== ($tokens[$commentStart]['line'] + 1)) {
            $secondLine = $phpcsFile->findNext([T_DOC_COMMENT_STAR, T_DOC_COMMENT_CLOSE_TAG], ($commentStart + 1), $commentEnd);
            $fix        = $phpcsFile->addFixableError('The second line in the file doc comment must be "@file"', $secondLine, 'FileTag');
            if ($fix === true) {
                if ($fileTag === false) {
                    $phpcsFile->fixer->addContent($commentStart, "\n * @file");
                } else {
                    // Delete the @file tag at its current position and insert one
                    // after the beginning of the comment.
                    $phpcsFile->fixer->beginChangeset();
                    $phpcsFile->fixer->addContent($commentStart, "\n * @file");
                    $phpcsFile->fixer->replaceToken($fileTag, '');
                    $phpcsFile->fixer->endChangeset();
                }
            }

            return ($phpcsFile->numTokens + 1);
        }

        // Exactly one blank line after the file comment.
        if ($tokens[$next]['line'] !== ($tokens[$commentEnd]['line'] + 2)
            && $next !== false && $tokens[$next]['code'] !== T_CLOSE_TAG
        ) {
            $error = 'There must be exactly one blank line after the file comment';
            $fix   = $phpcsFile->addFixableError($error, $commentEnd, 'SpacingAfterComment');
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                $uselessLine = ($commentEnd + 1);
                while ($uselessLine < $next) {
                    $phpcsFile->fixer->replaceToken($uselessLine, '');
                    $uselessLine++;
                }

                $phpcsFile->fixer->addContent($commentEnd, "\n\n");
                $phpcsFile->fixer->endChangeset();
            }

            return ($phpcsFile->numTokens + 1);
        }

        // Template file: no blank line after the file comment.
        if ($tokens[$next]['line'] !== ($tokens[$commentEnd]['line'] + 1)
            && $tokens[$next]['line'] > $tokens[$commentEnd]['line']
            && $tokens[$next]['code'] === T_CLOSE_TAG
        ) {
            $error = 'There must be no blank line after the file comment in a template';
            $fix   = $phpcsFile->addFixableError($error, $commentEnd, 'TeamplateSpacingAfterComment');
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                $uselessLine = ($commentEnd + 1);
                while ($uselessLine < $next) {
                    $phpcsFile->fixer->replaceToken($uselessLine, '');
                    $uselessLine++;
                }

                $phpcsFile->fixer->addContent($commentEnd, "\n");
                $phpcsFile->fixer->endChangeset();
            }
        }

        // Ignore the rest of the file.
        return ($phpcsFile->numTokens + 1);

    }//end process()


}//end class
