<?php
/**
 * \Backdrop\Sniffs\Commenting\DocCommentStarSniff
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Commenting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Checks that a doc comment block has a doc comment star on every line.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class DocCommentStarSniff implements Sniff
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
        $tokens = $phpcsFile->getTokens();

        $lastLineChecked = $tokens[$stackPtr]['line'];
        for ($i = ($stackPtr + 1); $i < ($tokens[$stackPtr]['comment_closer'] - 1); $i++) {
            // We are only interested in the beginning of the line.
            if ($tokens[$i]['line'] === $lastLineChecked) {
                continue;
            }

            // The first token on the line must be a whitespace followed by a star.
            if ($tokens[$i]['code'] === T_DOC_COMMENT_WHITESPACE) {
                if ($tokens[($i + 1)]['code'] !== T_DOC_COMMENT_STAR) {
                    $error = 'Doc comment star missing';
                    $fix   = $phpcsFile->addFixableError($error, $i, 'StarMissing');
                    if ($fix === true) {
                        if (strpos($tokens[$i]['content'], $phpcsFile->eolChar) !== false) {
                            $phpcsFile->fixer->replaceToken($i, str_repeat(' ', $tokens[$stackPtr]['column'])."* \n");
                        } else {
                            $phpcsFile->fixer->replaceToken($i, str_repeat(' ', $tokens[$stackPtr]['column']).'* ');
                        }

                        // Ordering of lines might have changed - stop here. The
                        // fixer will restart the sniff if there are remaining fixes.
                        return;
                    }
                }
            } else if ($tokens[$i]['code'] !== T_DOC_COMMENT_STAR) {
                $error = 'Doc comment star missing';
                $fix   = $phpcsFile->addFixableError($error, $i, 'StarMissing');
                if ($fix === true) {
                    $phpcsFile->fixer->addContentBefore($i, str_repeat(' ', $tokens[$stackPtr]['column']).'* ');
                }
            }//end if

            $lastLineChecked = $tokens[$i]['line'];
        }//end for

    }//end process()


}//end class
