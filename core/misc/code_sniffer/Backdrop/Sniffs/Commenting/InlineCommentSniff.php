<?php
/**
 * \Backdrop\Sniffs\Commenting\InlineCommentSniff.
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
 * \Backdrop\Sniffs\Commenting\InlineCommentSniff.
 *
 * Checks that no perl-style comments are used. Checks that inline comments ("//")
 * have a space after //, start capitalized and end with proper punctuation.
 * Largely copied from
 * \PHP_CodeSniffer\Standards\Squiz\Sniffs\Commenting\InlineCommentSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class InlineCommentSniff implements Sniff
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
            T_DOC_COMMENT_OPEN_TAG,
        ];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token in the
     *                                               stack passed in $tokens.
     *
     * @return int|void
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();

        // If this is a function/class/interface doc block comment, skip it.
        // We are only interested in inline doc block comments, which are
        // not allowed.
        if ($tokens[$stackPtr]['code'] === T_DOC_COMMENT_OPEN_TAG) {
            $nextToken = $phpcsFile->findNext(
                Tokens::$emptyTokens,
                ($stackPtr + 1),
                null,
                true
            );

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

            // Also ignore all doc blocks defined in the outer scope (no scope
            // conditions are set).
            if (in_array($tokens[$nextToken]['code'], $ignore, true) === true
                || empty($tokens[$stackPtr]['conditions']) === true
            ) {
                return;
            }

            if ($phpcsFile->tokenizerType === 'JS') {
                // We allow block comments if a function or object
                // is being assigned to a variable.
                $ignore    = Tokens::$emptyTokens;
                $ignore[]  = T_EQUAL;
                $ignore[]  = T_STRING;
                $ignore[]  = T_OBJECT_OPERATOR;
                $nextToken = $phpcsFile->findNext($ignore, ($nextToken + 1), null, true);
                if ($tokens[$nextToken]['code'] === T_FUNCTION
                    || $tokens[$nextToken]['code'] === T_CLOSURE
                    || $tokens[$nextToken]['code'] === T_OBJECT
                    || $tokens[$nextToken]['code'] === T_PROTOTYPE
                ) {
                    return;
                }
            }

            $prevToken = $phpcsFile->findPrevious(
                Tokens::$emptyTokens,
                ($stackPtr - 1),
                null,
                true
            );

            if ($tokens[$prevToken]['code'] === T_OPEN_TAG) {
                return;
            }

            // Inline doc blocks are allowed in JSDoc.
            if ($tokens[$stackPtr]['content'] === '/**' && $phpcsFile->tokenizerType !== 'JS') {
                // The only exception to inline doc blocks is the /** @var */
                // declaration. Allow that in any form.
                $varTag = $phpcsFile->findNext([T_DOC_COMMENT_TAG], ($stackPtr + 1), $tokens[$stackPtr]['comment_closer'], false, '@var');
                if ($varTag === false) {
                    $error = 'Inline doc block comments are not allowed; use "/* Comment */" or "// Comment" instead';
                    $phpcsFile->addError($error, $stackPtr, 'DocBlock');
                }
            }
        }//end if

        if ($tokens[$stackPtr]['content'][0] === '#') {
            $error = 'Perl-style comments are not allowed; use "// Comment" instead';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'WrongStyle');
            if ($fix === true) {
                $comment = ltrim($tokens[$stackPtr]['content'], "# \t");
                $phpcsFile->fixer->replaceToken($stackPtr, "// $comment");
            }
        }

        // We don't want end of block comments. Check if the last token before the
        // comment is a closing curly brace.
        $previousContent = $phpcsFile->findPrevious(T_WHITESPACE, ($stackPtr - 1), null, true);
        if ($tokens[$previousContent]['line'] === $tokens[$stackPtr]['line']) {
            if ($tokens[$previousContent]['code'] === T_CLOSE_CURLY_BRACKET) {
                return;
            }

            // Special case for JS files.
            if ($tokens[$previousContent]['code'] === T_COMMA
                || $tokens[$previousContent]['code'] === T_SEMICOLON
            ) {
                $lastContent = $phpcsFile->findPrevious(T_WHITESPACE, ($previousContent - 1), null, true);
                if ($tokens[$lastContent]['code'] === T_CLOSE_CURLY_BRACKET) {
                    return;
                }
            }
        }

        // Only want inline comments.
        if (substr($tokens[$stackPtr]['content'], 0, 2) !== '//') {
            return;
        }

        // Ignore code example lines.
        if ($this->isInCodeExample($phpcsFile, $stackPtr) === true) {
            return;
        }

        $commentTokens = [$stackPtr];

        $nextComment = $stackPtr;
        $lastComment = $stackPtr;
        while (($nextComment = $phpcsFile->findNext(T_COMMENT, ($nextComment + 1), null, false)) !== false) {
            if ($tokens[$nextComment]['line'] !== ($tokens[$lastComment]['line'] + 1)) {
                break;
            }

            // Only want inline comments.
            if (substr($tokens[$nextComment]['content'], 0, 2) !== '//') {
                break;
            }

            // There is a comment on the very next line. If there is
            // no code between the comments, they are part of the same
            // comment block.
            $prevNonWhitespace = $phpcsFile->findPrevious(T_WHITESPACE, ($nextComment - 1), $lastComment, true);
            if ($prevNonWhitespace !== $lastComment) {
                break;
            }

            // A comment starting with "@" means a new comment section.
            if (preg_match('|^//[\s]*@|', $tokens[$nextComment]['content']) === 1) {
                break;
            }

            $commentTokens[] = $nextComment;
            $lastComment     = $nextComment;
        }//end while

        $commentText      = '';
        $lastCommentToken = $stackPtr;
        foreach ($commentTokens as $lastCommentToken) {
            $comment = rtrim($tokens[$lastCommentToken]['content']);

            if (trim(substr($comment, 2)) === '') {
                continue;
            }

            $spaceCount = 0;
            $tabFound   = false;

            $commentLength = strlen($comment);
            for ($i = 2; $i < $commentLength; $i++) {
                if ($comment[$i] === "\t") {
                    $tabFound = true;
                    break;
                }

                if ($comment[$i] !== ' ') {
                    break;
                }

                $spaceCount++;
            }

            $fix = false;
            if ($tabFound === true) {
                $error = 'Tab found before comment text; expected "// %s" but found "%s"';
                $data  = [
                    ltrim(substr($comment, 2)),
                    $comment,
                ];
                $fix   = $phpcsFile->addFixableError($error, $lastCommentToken, 'TabBefore', $data);
            } else if ($spaceCount === 0) {
                $error = 'No space found before comment text; expected "// %s" but found "%s"';
                $data  = [
                    substr($comment, 2),
                    $comment,
                ];
                $fix   = $phpcsFile->addFixableError($error, $lastCommentToken, 'NoSpaceBefore', $data);
            }//end if

            if ($fix === true) {
                $newComment = '// '.ltrim($tokens[$lastCommentToken]['content'], "/\t ");
                $phpcsFile->fixer->replaceToken($lastCommentToken, $newComment);
            }

            if ($spaceCount > 1) {
                // Check if there is a comment on the previous line that justifies the
                // indentation.
                $prevComment = $phpcsFile->findPrevious([T_COMMENT], ($lastCommentToken - 1), null, false);
                if (($prevComment !== false) && (($tokens[$prevComment]['line']) === ($tokens[$lastCommentToken]['line'] - 1))) {
                    $prevCommentText = rtrim($tokens[$prevComment]['content']);
                    $prevSpaceCount  = 0;
                    for ($i = 2; $i < strlen($prevCommentText); $i++) {
                        if ($prevCommentText[$i] !== ' ') {
                            break;
                        }

                        $prevSpaceCount++;
                    }

                    if ($spaceCount > $prevSpaceCount && $prevSpaceCount > 0) {
                        // A previous comment could be a list item or @todo.
                        $indentationStarters = [
                            '-',
                            '@todo',
                        ];
                        $words        = preg_split('/\s+/', $prevCommentText);
                        $numberedList = (bool) preg_match('/^[0-9]+\./', $words[1]);
                        if (in_array($words[1], $indentationStarters) === true) {
                            if ($spaceCount !== ($prevSpaceCount + 2)) {
                                $error = 'Comment indentation error after %s element, expected %s spaces';
                                $fix   = $phpcsFile->addFixableError($error, $lastCommentToken, 'SpacingBefore', [$words[1], ($prevSpaceCount + 2)]);
                                if ($fix === true) {
                                    $newComment = '//'.str_repeat(' ', ($prevSpaceCount + 2)).ltrim($tokens[$lastCommentToken]['content'], "/\t ");
                                    $phpcsFile->fixer->replaceToken($lastCommentToken, $newComment);
                                }
                            }
                        } else if ($numberedList === true) {
                            $expectedSpaceCount = ($prevSpaceCount + strlen($words[1]) + 1);
                            if ($spaceCount !== $expectedSpaceCount) {
                                $error = 'Comment indentation error, expected %s spaces';
                                $fix   = $phpcsFile->addFixableError($error, $lastCommentToken, 'SpacingBefore', [$expectedSpaceCount]);
                                if ($fix === true) {
                                    $newComment = '//'.str_repeat(' ', $expectedSpaceCount).ltrim($tokens[$lastCommentToken]['content'], "/\t ");
                                    $phpcsFile->fixer->replaceToken($lastCommentToken, $newComment);
                                }
                            }
                        } else {
                            $error = 'Comment indentation error, expected only %s spaces';
                            $phpcsFile->addError($error, $lastCommentToken, 'SpacingBefore', [$prevSpaceCount]);
                        }//end if
                    }//end if
                } else {
                    $error = '%s spaces found before inline comment; expected "// %s" but found "%s"';
                    $data  = [
                        $spaceCount,
                        substr($comment, (2 + $spaceCount)),
                        $comment,
                    ];
                    $fix   = $phpcsFile->addFixableError($error, $lastCommentToken, 'SpacingBefore', $data);
                    if ($fix === true) {
                        $phpcsFile->fixer->replaceToken($lastCommentToken, '// '.substr($comment, (2 + $spaceCount)).$phpcsFile->eolChar);
                    }
                }//end if
            }//end if

            $commentText .= trim(substr($tokens[$lastCommentToken]['content'], 2));
        }//end foreach

        if ($commentText === '') {
            $error = 'Blank comments are not allowed';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'Empty');
            if ($fix === true) {
                $phpcsFile->fixer->replaceToken($stackPtr, '');
            }

            return ($lastCommentToken + 1);
        }

        $words = preg_split('/\s+/', $commentText);
        if (preg_match('/^\p{Ll}/u', $commentText) === 1) {
            // Allow special lower cased words that contain non-alpha characters
            // (function references, machine names with underscores etc.).
            $matches = [];
            preg_match('/[a-z]+/', $words[0], $matches);
            if (isset($matches[0]) === true && $matches[0] === $words[0]) {
                $error = 'Inline comments must start with a capital letter';
                $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'NotCapital');
                if ($fix === true) {
                    $newComment = preg_replace("/$words[0]/", ucfirst($words[0]), $tokens[$stackPtr]['content'], 1);
                    $phpcsFile->fixer->replaceToken($stackPtr, $newComment);
                }
            }
        }

        // Only check the end of comment character if the start of the comment
        // is a letter, indicating that the comment is just standard text.
        if (preg_match('/^\p{L}/u', $commentText) === 1) {
            $commentCloser   = $commentText[(strlen($commentText) - 1)];
            $acceptedClosers = [
                'full-stops'             => '.',
                'exclamation marks'      => '!',
                'question marks'         => '?',
                'colons'                 => ':',
                'or closing parentheses' => ')',
            ];

            // Allow special last words like URLs or function references
            // without punctuation.
            $lastWord = $words[(count($words) - 1)];
            $matches  = [];
            preg_match('/https?:\/\/.+/', $lastWord, $matches);
            $isUrl = isset($matches[0]) === true;
            preg_match('/[$a-zA-Z_]+\([$a-zA-Z_]*\)/', $lastWord, $matches);
            $isFunction = isset($matches[0]) === true;

            // Also allow closing tags like @endlink or @endcode.
            $isEndTag = $lastWord[0] === '@';

            if (in_array($commentCloser, $acceptedClosers, true) === false
                && $isUrl === false && $isFunction === false && $isEndTag === false
            ) {
                $error = 'Inline comments must end in %s';
                $ender = '';
                foreach ($acceptedClosers as $closerName => $symbol) {
                    $ender .= ' '.$closerName.',';
                }

                $ender = trim($ender, ' ,');
                $data  = [$ender];
                $fix   = $phpcsFile->addFixableError($error, $lastCommentToken, 'InvalidEndChar', $data);
                if ($fix === true) {
                    $newContent = preg_replace('/(\s+)$/', '.$1', $tokens[$lastCommentToken]['content']);
                    $phpcsFile->fixer->replaceToken($lastCommentToken, $newContent);
                }
            }
        }//end if

        // Finally, the line below the last comment cannot be empty if this inline
        // comment is on a line by itself.
        if ($tokens[$previousContent]['line'] < $tokens[$stackPtr]['line']) {
            $next = $phpcsFile->findNext(T_WHITESPACE, ($lastCommentToken + 1), null, true);
            if ($next === false) {
                // Ignore if the comment is the last non-whitespace token in a file.
                return ($lastCommentToken + 1);
            }

            if ($tokens[$next]['code'] === T_DOC_COMMENT_OPEN_TAG) {
                // If this inline comment is followed by a docblock,
                // ignore spacing as docblock/function etc spacing rules
                // are likely to conflict with our rules.
                return ($lastCommentToken + 1);
            }

            $errorCode = 'SpacingAfter';

            if (isset($tokens[$stackPtr]['conditions']) === true) {
                $conditions   = $tokens[$stackPtr]['conditions'];
                $type         = end($conditions);
                $conditionPtr = key($conditions);

                if (($type === T_FUNCTION || $type === T_CLOSURE)
                    && $tokens[$conditionPtr]['scope_closer'] === $next
                ) {
                    $errorCode = 'SpacingAfterAtFunctionEnd';
                }
            }

            for ($i = ($lastCommentToken + 1); $i < $phpcsFile->numTokens; $i++) {
                if ($tokens[$i]['line'] === ($tokens[$lastCommentToken]['line'] + 1)) {
                    if ($tokens[$i]['code'] !== T_WHITESPACE) {
                        return ($lastCommentToken + 1);
                    }
                } else if ($tokens[$i]['line'] > ($tokens[$lastCommentToken]['line'] + 1)) {
                    break;
                }
            }

            $error = 'There must be no blank line following an inline comment';
            $fix   = $phpcsFile->addFixableWarning($error, $lastCommentToken, $errorCode);
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                for ($i = ($lastCommentToken + 1); $i < $next; $i++) {
                    if ($tokens[$i]['line'] === $tokens[$next]['line']) {
                        break;
                    }

                    $phpcsFile->fixer->replaceToken($i, '');
                }

                $phpcsFile->fixer->endChangeset();
            }
        }//end if

        return ($lastCommentToken + 1);

    }//end process()


    /**
     * Determines if a comment line is part of an @code/@endcode example.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     *
     * @return boolean Returns true if the comment line is within a @code block,
     *                 false otherwise.
     */
    protected function isInCodeExample(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();
        if ($tokens[$stackPtr]['content'] === '// @code'.$phpcsFile->eolChar) {
            return true;
        }

        $prevComment = $stackPtr;
        $lastComment = $stackPtr;
        while (($prevComment = $phpcsFile->findPrevious([T_COMMENT], ($lastComment - 1), null, false)) !== false) {
            if ($tokens[$prevComment]['line'] !== ($tokens[$lastComment]['line'] - 1)) {
                return false;
            }

            if ($tokens[$prevComment]['content'] === '// @code'.$phpcsFile->eolChar) {
                return true;
            }

            if ($tokens[$prevComment]['content'] === '// @endcode'.$phpcsFile->eolChar) {
                return false;
            }

            $lastComment = $prevComment;
        }

        return false;

    }//end isInCodeExample()


}//end class
