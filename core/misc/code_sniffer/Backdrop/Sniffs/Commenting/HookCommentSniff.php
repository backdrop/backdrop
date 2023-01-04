<?php
/**
 * Ensures hook comments on function are correct.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Commenting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Ensures hook comments on function are correct.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class HookCommentSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_FUNCTION];

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

        // We are only interested in the most outer scope, ignore methods in classes for example.
        if (empty($tokens[$stackPtr]['conditions']) === false) {
            return;
        }

        $commentEnd = $phpcsFile->findPrevious(T_WHITESPACE, ($stackPtr - 1), null, true);
        if ($tokens[$commentEnd]['code'] !== T_DOC_COMMENT_CLOSE_TAG) {
            return;
        }

        $commentStart = $tokens[$commentEnd]['comment_opener'];

        $empty = [
            T_DOC_COMMENT_WHITESPACE,
            T_DOC_COMMENT_STAR,
        ];

        $short = $phpcsFile->findNext($empty, ($commentStart + 1), $commentEnd, true);
        if ($short === false) {
            // No content at all.
            return;
        }

        // Account for the fact that a short description might cover
        // multiple lines.
        $shortContent = $tokens[$short]['content'];
        $shortEnd     = $short;
        for ($i = ($short + 1); $i < $commentEnd; $i++) {
            if ($tokens[$i]['code'] === T_DOC_COMMENT_STRING) {
                if ($tokens[$i]['line'] === ($tokens[$shortEnd]['line'] + 1)) {
                    $shortContent .= $tokens[$i]['content'];
                    $shortEnd      = $i;
                } else {
                    break;
                }
            }
        }

        // Check if a hook implementation doc block is formatted correctly.
        if (preg_match('/^[\s]*Implement[^\n]+?hook_[^\n]+/i', $shortContent, $matches) === 1) {
            if (strstr($matches[0], 'Implements ') === false || strstr($matches[0], 'Implements of') !== false
                || preg_match('/ (drush_)?hook_[a-zA-Z0-9_]+\(\)( for .+)?\.$/', $matches[0]) !== 1
            ) {
                $phpcsFile->addWarning('Format should be "* Implements hook_foo().", "* Implements hook_foo_BAR_ID_bar() for xyz_bar().",, "* Implements hook_foo_BAR_ID_bar() for xyz-bar.html.twig.", "* Implements hook_foo_BAR_ID_bar() for xyz-bar.tpl.php.", or "* Implements hook_foo_BAR_ID_bar() for block templates."', $short, 'HookCommentFormat');
            } else {
                // Check that a hook implementation does not duplicate @param and
                // @return documentation.
                foreach ($tokens[$commentStart]['comment_tags'] as $pos => $tag) {
                    if ($tokens[$tag]['content'] === '@param') {
                        $warn = 'Hook implementations should not duplicate @param documentation';
                        $phpcsFile->addWarning($warn, $tag, 'HookParamDoc');
                    }

                    if ($tokens[$tag]['content'] === '@return') {
                        $warn = 'Hook implementations should not duplicate @return documentation';
                        $phpcsFile->addWarning($warn, $tag, 'HookReturnDoc');
                    }
                }
            }//end if

            return;
        }//end if

        // Check if the doc block just repeats the function name with
        // "Implements example_hook_name()".
        $functionName = $phpcsFile->getDeclarationName($stackPtr);
        if ($functionName !== null && preg_match("/^[\s]*Implements $functionName\(\)\.$/i", $shortContent) === 1) {
            $error = 'Hook implementations must be documented with "Implements hook_example()."';
            $fix   = $phpcsFile->addFixableError($error, $short, 'HookRepeat');
            if ($fix === true) {
                $newComment = preg_replace('/Implements [^_]+/', 'Implements hook', $shortContent);
                $phpcsFile->fixer->replaceToken($short, $newComment);
            }
        }

    }//end process()


}//end class
