<?php

/**
 * \Backdrop\Sniffs\Commenting\InlineVariableCommentSniff
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Commenting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Checks for the correct usage of inline variable type declarations.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class InlineVariableCommentSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [
            T_COMMENT,
            T_DOC_COMMENT_TAG,
        ];

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
        $ignore = [
            T_CLASS,
            T_INTERFACE,
            T_TRAIT,
            T_FUNCTION,
            T_CLOSURE,
            T_PUBLIC,
            T_PRIVATE,
            T_PROTECTED,
            T_FINAL,
            T_STATIC,
            T_ABSTRACT,
            T_CONST,
            T_PROPERTY,
            T_INCLUDE,
            T_INCLUDE_ONCE,
            T_REQUIRE,
            T_REQUIRE_ONCE,
            T_VAR,
        ];

        // If this is a function/class/interface doc block comment, skip it.
        $nextToken = $phpcsFile->findNext(Tokens::$emptyTokens, ($stackPtr + 1), null, true);
        if (in_array($tokens[$nextToken]['code'], $ignore, true) === true) {
            return;
        }

        if ($tokens[$stackPtr]['code'] === T_COMMENT) {
            if (strpos($tokens[$stackPtr]['content'], '@var') !== false) {
                $warning = 'Inline @var declarations should use the /** */ delimiters';

                if (strpos($tokens[$stackPtr]['content'], '#') === 0 || strpos($tokens[$stackPtr]['content'], '//') === 0) {
                    // If this comment contains '*/' then the developer is mixing
                    // inline comment styles. This could be commented out code,
                    // so leave this line alone completely.
                    if (strpos($tokens[$stackPtr]['content'], '*/') !== false) {
                        return;
                    }

                    if ($phpcsFile->addFixableWarning($warning, $stackPtr, 'VarInline') === true) {
                        // Hashtag and slash based comments contain a trailing
                        // new line.
                        $varContent = rtrim($tokens[$stackPtr]['content']);

                        // Remove all leading hashtags and slashes.
                        $varContent = ltrim($varContent, '/# ');

                        $phpcsFile->fixer->replaceToken($stackPtr, ('/** '.$varContent." */\n"));
                    }
                } else {
                    if ($phpcsFile->addFixableWarning($warning, $stackPtr, 'VarInline') === true) {
                        $phpcsFile->fixer->replaceToken($stackPtr, substr_replace($tokens[$stackPtr]['content'], '/**', 0, 2));
                    }
                }//end if
            }//end if

            return;
        }//end if

        // Skip if it's not a variable declaration.
        if ($tokens[$stackPtr]['content'] !== '@var') {
            return;
        }

        // Get the content of the @var tag to determine the order.
        $varContent    = '';
        $varContentPtr = $phpcsFile->findNext(T_DOC_COMMENT_STRING, ($stackPtr + 1));
        if ($varContentPtr !== false) {
            $varContent = $tokens[$varContentPtr]['content'];
        }

        if (strpos($varContent, '$') === 0) {
            $warning = 'The variable name should be defined after the type';

            $parts = explode(' ', $varContent, 3);
            if (isset($parts[1]) === true) {
                if ($phpcsFile->addFixableWarning($warning, $varContentPtr, 'VarInlineOrder') === true) {
                    // Switch type and variable name.
                    $replace = [
                        $parts[1],
                        $parts[0],
                    ];
                    if (isset($parts[2]) === true) {
                        $replace[] = $parts[2];
                    }

                    $phpcsFile->fixer->replaceToken($varContentPtr, implode(' ', $replace));
                }
            } else {
                $phpcsFile->addWarning($warning, $varContentPtr, 'VarInlineOrder');
            }
        }//end if

    }//end process()


}//end class
