<?php
/**
 * \Backdrop\Sniffs\Files\TxtFileLineLengthSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Files;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * \Backdrop\Sniffs\Files\TxtFileLineLengthSniff.
 *
 * Checks all lines in a *.txt or *.md file and throws warnings if they are over 80
 * characters in length.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class TxtFileLineLengthSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_INLINE_HTML];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token in the
     *                                               stack passed in $tokens.
     *
     * @return void
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        $fileExtension = strtolower(substr($phpcsFile->getFilename(), -3));
        if ($fileExtension === 'txt' || $fileExtension === '.md') {
            $tokens = $phpcsFile->getTokens();

            $content    = rtrim($tokens[$stackPtr]['content']);
            $lineLength = mb_strlen($content, 'UTF-8');
            if ($lineLength > 80) {
                // Often text files contain long URLs that need to be preceded
                // with certain textual elements that are significant for
                // preserving the formatting of the document - e.g. a long link
                // in a bulleted list. If we find that the line does not contain
                // any spaces after the 40th character we'll allow it.
                if (preg_match('/\s+/', mb_substr($content, 40)) === 0) {
                    return;
                }

                // Lines without spaces are allowed to be longer.
                // Markdown allowed to be longer for lines
                // - without spaces
                // - starting with #
                // - starting with | (tables)
                // - containing a link.
                if (preg_match('/^([^ ]+$|#|\||.*\[.+\]\(.+\))/', $content) === 0) {
                    $data    = [
                        80,
                        $lineLength,
                    ];
                    $warning = 'Line exceeds %s characters; contains %s characters';
                    $phpcsFile->addWarning($warning, $stackPtr, 'TooLong', $data);
                }
            }//end if
        }//end if

    }//end process()


}//end class
