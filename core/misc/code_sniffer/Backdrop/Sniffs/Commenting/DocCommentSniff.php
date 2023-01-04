<?php
/**
 * Ensures doc blocks follow basic formatting.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Commenting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Ensures doc blocks follow basic formatting.
 *
 * Largely copied from
 * \PHP_CodeSniffer\Standards\Generic\Sniffs\Commenting\DocCommentSniff,
 * but Backdrop @file comments are different.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class DocCommentSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_DOC_COMMENT_OPEN_TAG];

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
        $tokens       = $phpcsFile->getTokens();
        $commentEnd   = $phpcsFile->findNext(T_DOC_COMMENT_CLOSE_TAG, ($stackPtr + 1));
        $commentStart = $tokens[$commentEnd]['comment_opener'];

        $empty = [
            T_DOC_COMMENT_WHITESPACE,
            T_DOC_COMMENT_STAR,
        ];

        $short = $phpcsFile->findNext($empty, ($stackPtr + 1), $commentEnd, true);
        if ($short === false) {
            // No content at all.
            $error = 'Doc comment is empty';
            $phpcsFile->addError($error, $stackPtr, 'Empty');
            return;
        }

        // Ignore doc blocks in functions, this is handled by InlineCommentSniff.
        if (empty($tokens[$stackPtr]['conditions']) === false && in_array(T_FUNCTION, $tokens[$stackPtr]['conditions']) === true) {
            return;
        }

        // The first line of the comment should just be the /** code.
        // In JSDoc there are cases with @lends that are on the same line as code.
        if ($tokens[$short]['line'] === $tokens[$stackPtr]['line'] && $phpcsFile->tokenizerType !== 'JS') {
            $error = 'The open comment tag must be the only content on the line';
            $fix   = $phpcsFile->addFixableError($error, $stackPtr, 'ContentAfterOpen');
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                $phpcsFile->fixer->addNewline($stackPtr);
                $phpcsFile->fixer->addContentBefore($short, '* ');
                $phpcsFile->fixer->endChangeset();
            }
        }

        // The last line of the comment should just be the */ code.
        $prev = $phpcsFile->findPrevious($empty, ($commentEnd - 1), $stackPtr, true);
        if ($tokens[$commentEnd]['content'] !== '*/') {
            $error = 'Wrong function doc comment end; expected "*/", found "%s"';
            $phpcsFile->addError($error, $commentEnd, 'WrongEnd', [$tokens[$commentEnd]['content']]);
        }

        // Check for additional blank lines at the end of the comment.
        if ($tokens[$prev]['line'] < ($tokens[$commentEnd]['line'] - 1)) {
            $error = 'Additional blank lines found at end of doc comment';
            $fix   = $phpcsFile->addFixableError($error, $commentEnd, 'SpacingAfter');
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                for ($i = ($prev + 1); $i < $commentEnd; $i++) {
                    if ($tokens[($i + 1)]['line'] === $tokens[$commentEnd]['line']) {
                        break;
                    }

                    $phpcsFile->fixer->replaceToken($i, '');
                }

                $phpcsFile->fixer->endChangeset();
            }
        }

        // The short description of @file comments is one line below.
        if ($tokens[$short]['code'] === T_DOC_COMMENT_TAG && $tokens[$short]['content'] === '@file') {
            $next = $phpcsFile->findNext($empty, ($short + 1), $commentEnd, true);
            if ($next !== false) {
                $fileShort = $short;
                $short     = $next;
            }
        }

        // Do not check defgroup sections, they have no short description. Also don't
        // check PHPUnit tests doc blocks because they might not have a description.
        if (in_array($tokens[$short]['content'], ['@defgroup', '@addtogroup', '@}', '@coversDefaultClass']) === true) {
            return;
        }

        // Check for a comment description.
        if ($tokens[$short]['code'] !== T_DOC_COMMENT_STRING) {
            // JSDoc has many cases of @type declaration that don't have a
            // description.
            if ($phpcsFile->tokenizerType === 'JS') {
                return;
            }

            // PHPUnit test methods are allowed to skip the short description and
            // only provide an @covers annotation.
            if ($tokens[$short]['content'] === '@covers') {
                return;
            }

            $error = 'Missing short description in doc comment';
            $phpcsFile->addError($error, $stackPtr, 'MissingShort');
            return;
        }

        if (isset($fileShort) === true) {
            $start = $fileShort;
        } else {
            $start = $stackPtr;
        }

        // No extra newline before short description.
        if ($tokens[$short]['line'] !== ($tokens[$start]['line'] + 1)) {
            $error = 'Doc comment short description must be on the first line';
            $fix   = $phpcsFile->addFixableError($error, $short, 'SpacingBeforeShort');
            if ($fix === true) {
                // Move file comment short description to the next line.
                if (isset($fileShort) === true && $tokens[$short]['line'] === $tokens[$start]['line']) {
                    $phpcsFile->fixer->addContentBefore($fileShort, "\n *");
                } else {
                    $phpcsFile->fixer->beginChangeset();
                    for ($i = $start; $i < $short; $i++) {
                        if ($tokens[$i]['line'] === $tokens[$start]['line']) {
                            continue;
                        } else if ($tokens[$i]['line'] === $tokens[$short]['line']) {
                            break;
                        }

                        $phpcsFile->fixer->replaceToken($i, '');
                    }

                    $phpcsFile->fixer->endChangeset();
                }
            }
        }//end if

        if ($tokens[($short - 1)]['content'] !== ' '
            && strpos($tokens[($short - 1)]['content'], $phpcsFile->eolChar) === false
        ) {
            $error = 'Function comment short description must start with exactly one space';
            $fix   = $phpcsFile->addFixableError($error, $short, 'ShortStartSpace');
            if ($fix === true) {
                if ($tokens[($short - 1)]['code'] === T_DOC_COMMENT_WHITESPACE) {
                    $phpcsFile->fixer->replaceToken(($short - 1), ' ');
                } else {
                    $phpcsFile->fixer->addContent(($short - 1), ' ');
                }
            }
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

            if ($tokens[$i]['code'] === T_DOC_COMMENT_TAG) {
                break;
            }
        }

        // Remove any trailing white spaces which are detected by other sniffs.
        $shortContent = trim($shortContent);

        if (preg_match('|\p{Lu}|u', $shortContent[0]) === 0
            // Allow both variants of inheritdoc comments.
            && $shortContent !== '{@inheritdoc}'
            && $shortContent !== '{@inheritDoc}'
            // Ignore Features module export files that just use the file name as
            // comment.
            && $shortContent !== basename($phpcsFile->getFilename())
        ) {
            $error = 'Doc comment short description must start with a capital letter';
            // If we cannot capitalize the first character then we don't have a
            // fixable error.
            if ($tokens[$short]['content'] === ucfirst($tokens[$short]['content'])) {
                $phpcsFile->addError($error, $short, 'ShortNotCapital');
            } else {
                $fix = $phpcsFile->addFixableError($error, $short, 'ShortNotCapital');
                if ($fix === true) {
                    $phpcsFile->fixer->replaceToken($short, ucfirst($tokens[$short]['content']));
                }
            }
        }

        $lastChar = substr($shortContent, -1);
        // Allow these characters as valid line-ends not requiring to be fixed.
        if (in_array($lastChar, ['.', '!', '?', ')']) === false
            // Allow both variants of inheritdoc comments.
            && $shortContent !== '{@inheritdoc}'
            && $shortContent !== '{@inheritDoc}'
            // Ignore Features module export files that just use the file name as
            // comment.
            && $shortContent !== basename($phpcsFile->getFilename())
        ) {
            $error = 'Doc comment short description must end with a full stop';
            // If the last character is alphanumeric and the content is all on one line then fix it.
            if (preg_match('/[a-zA-Z0-9]/', $lastChar) === 1
                && $tokens[$short]['line'] === $tokens[$shortEnd]['line']
            ) {
                $fix = $phpcsFile->addFixableError($error, $shortEnd, 'ShortFullStop');
                if ($fix === true) {
                    $phpcsFile->fixer->addContent($shortEnd, '.');
                }
            } else {
                // The correct fix is not obvious, so report an error and leave for manual correction.
                $phpcsFile->addError($error, $shortEnd, 'ShortFullStop');
            }
        }

        if ($tokens[$short]['line'] !== $tokens[$shortEnd]['line']) {
            $error = 'Doc comment short description must be on a single line, further text should be a separate paragraph';
            $phpcsFile->addError($error, $shortEnd, 'ShortSingleLine');
        }

        $long = $phpcsFile->findNext($empty, ($shortEnd + 1), ($commentEnd - 1), true);
        if ($long === false) {
            return;
        }

        if ($tokens[$long]['code'] === T_DOC_COMMENT_STRING) {
            if ($tokens[$long]['line'] !== ($tokens[$shortEnd]['line'] + 2)) {
                $error = 'There must be exactly one blank line between descriptions in a doc comment';
                $fix   = $phpcsFile->addFixableError($error, $long, 'SpacingBetween');
                if ($fix === true) {
                    $phpcsFile->fixer->beginChangeset();
                    for ($i = ($shortEnd + 1); $i < $long; $i++) {
                        if ($tokens[$i]['line'] === $tokens[$shortEnd]['line']) {
                            continue;
                        } else if ($tokens[$i]['line'] === ($tokens[$long]['line'] - 1)) {
                            break;
                        }

                        $phpcsFile->fixer->replaceToken($i, '');
                    }

                    $phpcsFile->fixer->endChangeset();
                }
            }

            if (preg_match('|\p{Lu}|u', $tokens[$long]['content'][0]) === 0
                && $tokens[$long]['content'] !== ucfirst($tokens[$long]['content'])
            ) {
                $error = 'Doc comment long description must start with a capital letter';
                $fix   = $phpcsFile->addFixableError($error, $long, 'LongNotCapital');
                if ($fix === true) {
                    $phpcsFile->fixer->replaceToken($long, ucfirst($tokens[$long]['content']));
                }
            }

            // Account for the fact that a description might cover multiple lines.
            $longContent = $tokens[$long]['content'];
            $longEnd     = $long;
            for ($i = ($long + 1); $i < $commentEnd; $i++) {
                if ($tokens[$i]['code'] === T_DOC_COMMENT_STRING) {
                    if ($tokens[$i]['line'] <= ($tokens[$longEnd]['line'] + 1)) {
                        $longContent .= $tokens[$i]['content'];
                        $longEnd      = $i;
                    } else {
                        break;
                    }
                }

                if ($tokens[$i]['code'] === T_DOC_COMMENT_TAG) {
                    if ($tokens[$i]['line'] <= ($tokens[$longEnd]['line'] + 1)
                        // Allow link tags within the long comment itself.
                        && ($tokens[$i]['content'] === '@link' || $tokens[$i]['content'] === '@endlink')
                    ) {
                        $longContent .= $tokens[$i]['content'];
                        $longEnd      = $i;
                    } else {
                        break;
                    }
                }
            }//end for

            // Remove any trailing white spaces which are detected by other sniffs.
            $longContent = trim($longContent);

            if (preg_match('/[a-zA-Z]$/', $longContent) === 1) {
                $error = 'Doc comment long description must end with a full stop';
                $fix   = $phpcsFile->addFixableError($error, $longEnd, 'LongFullStop');
                if ($fix === true) {
                    $phpcsFile->fixer->addContent($longEnd, '.');
                }
            }
        }//end if

        if (empty($tokens[$commentStart]['comment_tags']) === true) {
            // No tags in the comment.
            return;
        }

        $firstTag = $tokens[$commentStart]['comment_tags'][0];
        $prev     = $phpcsFile->findPrevious($empty, ($firstTag - 1), $stackPtr, true);
        // This does not apply to @file, @code, @link and @endlink tags.
        if ($tokens[$firstTag]['line'] !== ($tokens[$prev]['line'] + 2)
            && isset($fileShort) === false
            && in_array($tokens[$firstTag]['content'], ['@code', '@link', '@endlink']) === false
        ) {
            $error = 'There must be exactly one blank line before the tags in a doc comment';
            $fix   = $phpcsFile->addFixableError($error, $firstTag, 'SpacingBeforeTags');
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                for ($i = ($prev + 1); $i < $firstTag; $i++) {
                    if ($tokens[$i]['line'] === $tokens[$firstTag]['line']) {
                        break;
                    }

                    $phpcsFile->fixer->replaceToken($i, '');
                }

                $indent = str_repeat(' ', $tokens[$stackPtr]['column']);
                $phpcsFile->fixer->addContent($prev, $phpcsFile->eolChar.$indent.'*'.$phpcsFile->eolChar);
                $phpcsFile->fixer->endChangeset();
            }
        }

        // Break out the tags into groups and check alignment within each.
        // A tag group is one where there are no blank lines between tags.
        // The param tag group is special as it requires all @param tags to be inside.
        $tagGroups    = [];
        $groupid      = 0;
        $paramGroupid = null;
        $currentTag   = null;
        $previousTag  = null;
        $isNewGroup   = null;
        $checkTags    = [
            '@param',
            '@return',
            '@throws',
        ];
        foreach ($tokens[$commentStart]['comment_tags'] as $pos => $tag) {
            if ($pos > 0) {
                // If this tag is not in the same column as the initial tag then
                // it must be an inline comment tag and should be ignored here.
                if ($tokens[$tag]['column'] !== $tokens[$firstTag]['column']) {
                    continue;
                }

                $prev = $phpcsFile->findPrevious(
                    T_DOC_COMMENT_STRING,
                    ($tag - 1),
                    $tokens[$commentStart]['comment_tags'][($pos - 1)]
                );

                if ($prev === false) {
                    $prev = $tokens[$commentStart]['comment_tags'][($pos - 1)];
                }

                $isNewGroup = $tokens[$prev]['line'] !== ($tokens[$tag]['line'] - 1);
                if ($isNewGroup === true) {
                    $groupid++;
                }
            }//end if

            $currentTag = $tokens[$tag]['content'];
            if ($currentTag === '@param') {
                if (($paramGroupid === null
                    && empty($tagGroups[$groupid]) === false)
                    || ($paramGroupid !== null
                    && $paramGroupid !== $groupid)
                ) {
                    $error = 'Parameter tags must be grouped together in a doc comment';
                    $phpcsFile->addError($error, $tag, 'ParamGroup');
                }

                if ($paramGroupid === null) {
                    $paramGroupid = $groupid;
                }

                // All of the $checkTags sections should be separated by a blank
                // line both before and after the sections.
            } else if ($isNewGroup === false
                && (in_array($currentTag, $checkTags) === true || in_array($previousTag, $checkTags) === true)
                && $previousTag !== $currentTag
            ) {
                $error = 'Separate the %s and %s sections by a blank line.';
                $fix   = $phpcsFile->addFixableError($error, $tag, 'TagGroupSpacing', [$previousTag, $currentTag]);
                if ($fix === true) {
                    $phpcsFile->fixer->replaceToken(($tag - 1), "\n".str_repeat(' ', ($tokens[$tag]['column'] - 3)).'* ');
                }
            }//end if

            $previousTag           = $currentTag;
            $tagGroups[$groupid][] = $tag;
        }//end foreach

        foreach ($tagGroups as $group) {
            $maxLength = 0;
            $paddings  = [];
            $pos       = 0;
            foreach ($group as $pos => $tag) {
                $tagLength = strlen($tokens[$tag]['content']);
                if ($tagLength > $maxLength) {
                    $maxLength = $tagLength;
                }

                // Check for a value. No value means no padding needed.
                $string = $phpcsFile->findNext(T_DOC_COMMENT_STRING, $tag, $commentEnd);
                if ($string !== false && $tokens[$string]['line'] === $tokens[$tag]['line']) {
                    $paddings[$tag] = strlen($tokens[($tag + 1)]['content']);
                }
            }

            // Check that there was single blank line after the tag block
            // but account for a multi-line tag comments.
            $lastTag = $group[$pos];
            $next    = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($lastTag + 3), $commentEnd);
            if ($next !== false && $tokens[$next]['column'] === $tokens[$firstTag]['column']) {
                $prev = $phpcsFile->findPrevious([T_DOC_COMMENT_TAG, T_DOC_COMMENT_STRING], ($next - 1), $commentStart);
                if ($tokens[$next]['line'] !== ($tokens[$prev]['line'] + 2)) {
                    $error = 'There must be a single blank line after a tag group';
                    $fix   = $phpcsFile->addFixableError($error, $lastTag, 'SpacingAfterTagGroup');
                    if ($fix === true) {
                        $phpcsFile->fixer->beginChangeset();
                        for ($i = ($prev + 1); $i < $next; $i++) {
                            if ($tokens[$i]['line'] === $tokens[$next]['line']) {
                                break;
                            }

                            $phpcsFile->fixer->replaceToken($i, '');
                        }

                        $indent = str_repeat(' ', $tokens[$stackPtr]['column']);
                        $phpcsFile->fixer->addContent($prev, $phpcsFile->eolChar.$indent.'*'.$phpcsFile->eolChar);
                        $phpcsFile->fixer->endChangeset();
                    }
                }
            }//end if

            // Now check paddings.
            foreach ($paddings as $tag => $padding) {
                if ($padding !== 1) {
                    $error = 'Tag value indented incorrectly; expected 1 space but found %s';
                    $data  = [$padding];

                    $fix = $phpcsFile->addFixableError($error, ($tag + 1), 'TagValueIndent', $data);
                    if ($fix === true) {
                        $phpcsFile->fixer->replaceToken(($tag + 1), ' ');
                    }
                }
            }
        }//end foreach

        // If there is a param group, it needs to be first; with the exception of
        // @code, @todo and link tags.
        if ($paramGroupid !== null && $paramGroupid !== 0
            && in_array($tokens[$tokens[$commentStart]['comment_tags'][0]]['content'], ['@code', '@todo', '@link', '@endlink', '@codingStandardsIgnoreStart']) === false
            // In JSDoc we can have many other valid tags like @function or
            // @constructor before the param tags.
            && $phpcsFile->tokenizerType !== 'JS'
        ) {
            $error = 'Parameter tags must be defined first in a doc comment';
            $phpcsFile->addError($error, $tagGroups[$paramGroupid][0], 'ParamNotFirst');
        }

        $foundTags = [];
        $lastPos   = 0;
        foreach ($tokens[$stackPtr]['comment_tags'] as $pos => $tag) {
            $tagName = $tokens[$tag]['content'];
            // Skip code tags, they can be anywhere.
            if (in_array($tagName, $checkTags) === false) {
                continue;
            }

            if (isset($foundTags[$tagName]) === true) {
                $lastTag = $tokens[$stackPtr]['comment_tags'][$lastPos];
                if ($tokens[$lastTag]['content'] !== $tagName) {
                    $error = 'Tags must be grouped together in a doc comment';
                    $phpcsFile->addError($error, $tag, 'TagsNotGrouped');
                }
            }

            $foundTags[$tagName] = true;
            $lastPos = $pos;
        }

    }//end process()


}//end class
