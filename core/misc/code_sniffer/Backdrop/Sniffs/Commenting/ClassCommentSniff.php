<?php
/**
 * Parses and verifies the class doc comment.
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
 * Checks that comment doc blocks exist on classes, interfaces and traits. Largely
 * copied from PHP_CodeSniffer\Standards\Squiz\Sniffs\Commenting\ClassCommentSniff.
 *
 * @category  PHP
 * @package   PHP_CodeSniffer
 * @author    Greg Sherwood <gsherwood@squiz.net>
 * @copyright 2006-2014 Squiz Pty Ltd (ABN 77 084 670 600)
 * @license   https://github.com/squizlabs/PHP_CodeSniffer/blob/master/licence.txt BSD Licence
 * @version   Release: @package_version@
 * @link      http://pear.php.net/package/PHP_CodeSniffer
 */
class ClassCommentSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [
            T_CLASS,
            T_INTERFACE,
            T_TRAIT,
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
        $find   = Tokens::$methodPrefixes;
        $find[] = T_WHITESPACE;
        $name   = $tokens[$stackPtr]['content'];

        $commentEnd = $phpcsFile->findPrevious($find, ($stackPtr - 1), null, true);
        if ($tokens[$commentEnd]['code'] !== T_DOC_COMMENT_CLOSE_TAG
            && $tokens[$commentEnd]['code'] !== T_COMMENT
        ) {
            $fix = $phpcsFile->addFixableError('Missing %s doc comment', $stackPtr, 'Missing', [$name]);
            if ($fix === true) {
                $phpcsFile->fixer->addContent($commentEnd, "\n/**\n *\n */");
            }

            return;
        }

        // Try and determine if this is a file comment instead of a class comment.
        if ($tokens[$commentEnd]['code'] === T_DOC_COMMENT_CLOSE_TAG) {
            $start = ($tokens[$commentEnd]['comment_opener'] - 1);
        } else {
            $start = ($commentEnd - 1);
        }

        $fileTag = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($start + 1), $commentEnd, false, '@file');
        if ($fileTag !== false) {
            // This is a file comment.
            $fix = $phpcsFile->addFixableError('Missing %s doc comment', $stackPtr, 'Missing', [$name]);
            if ($fix === true) {
                $phpcsFile->fixer->addContent($commentEnd, "\n/**\n *\n */");
            }

            return;
        }

        if ($tokens[$commentEnd]['code'] === T_COMMENT) {
            $fix = $phpcsFile->addFixableError('You must use "/**" style comments for a %s comment', $stackPtr, 'WrongStyle', [$name]);
            if ($fix === true) {
                // Convert the comment into a doc comment.
                $phpcsFile->fixer->beginChangeset();
                $comment = '';
                for ($i = $commentEnd; $tokens[$i]['code'] === T_COMMENT; $i--) {
                    $comment = ' *'.ltrim($tokens[$i]['content'], '/* ').$comment;
                    $phpcsFile->fixer->replaceToken($i, '');
                }

                $phpcsFile->fixer->replaceToken($commentEnd, "/**\n".rtrim($comment, "*/\n")."\n */");
                $phpcsFile->fixer->endChangeset();
            }

            return;
        }

        if ($tokens[$commentEnd]['line'] !== ($tokens[$stackPtr]['line'] - 1)) {
            $error = 'There must be exactly one newline after the %s comment';
            $fix   = $phpcsFile->addFixableError($error, $commentEnd, 'SpacingAfter', [$name]);
            if ($fix === true) {
                $phpcsFile->fixer->beginChangeset();
                for ($i = ($commentEnd + 1); $tokens[$i]['code'] === T_WHITESPACE && $i < $stackPtr; $i++) {
                    $phpcsFile->fixer->replaceToken($i, '');
                }

                $phpcsFile->fixer->addContent($commentEnd, "\n");
                $phpcsFile->fixer->endChangeset();
            }
        }

        $comment = [];
        for ($i = $start; $i < $commentEnd; $i++) {
            if ($tokens[$i]['code'] === T_DOC_COMMENT_TAG) {
                break;
            }

            if ($tokens[$i]['code'] === T_DOC_COMMENT_STRING) {
                $comment[] = $tokens[$i]['content'];
            }
        }

        $words = explode(' ', implode(' ', $comment));
        if (count($words) <= 2) {
            $className = $phpcsFile->getDeclarationName($stackPtr);

            foreach ($words as $word) {
                // Check if the comment contains the class name.
                if (strpos($word, $className) !== false) {
                    $error = 'The class short comment should describe what the class does and not simply repeat the class name';
                    $phpcsFile->addWarning($error, $commentEnd, 'Short');
                    break;
                }
            }
        }

    }//end process()


}//end class
