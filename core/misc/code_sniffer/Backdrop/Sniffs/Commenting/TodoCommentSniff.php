<?php
/**
 * Parses and verifies comment language.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Commenting;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Config;

/**
 * Parses and verifies that comments use the correct @todo format.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class TodoCommentSniff implements Sniff
{

    /**
     * Show debug output for this sniff.
     *
     * Use phpcs --runtime-set todo_debug true
     *
     * @var boolean
     */
    private $debug = false;


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        if (defined('PHP_CODESNIFFER_IN_TESTS') === true) {
            $this->debug = false;
        }

        return [
            T_COMMENT,
            T_DOC_COMMENT_TAG,
            T_DOC_COMMENT_STRING,
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
        $debug = Config::getConfigData('todo_debug');
        if ($debug !== null) {
            $this->debug = (bool) $debug;
        }

        $tokens = $phpcsFile->getTokens();
        if ($this->debug === true) {
            echo "\n------\n\$tokens[$stackPtr] = ".print_r($tokens[$stackPtr], true).PHP_EOL;
            echo 'code = '.$tokens[$stackPtr]['code'].', type = '.$tokens[$stackPtr]['type']."\n";
        }

        // Standard comments and multi-line comments where the "@" is missing so
        // it does not register as a T_DOC_COMMENT_TAG.
        if ($tokens[$stackPtr]['code'] === T_COMMENT || $tokens[$stackPtr]['code'] === T_DOC_COMMENT_STRING) {
            $comment = $tokens[$stackPtr]['content'];
            if ($this->debug === true) {
                echo "Getting \$comment from \$tokens[$stackPtr]['content']\n";
            }

            $this->checkTodoFormat($phpcsFile, $stackPtr, $comment, $tokens);
        } else if ($tokens[$stackPtr]['code'] === T_DOC_COMMENT_TAG) {
            // Document comment tag (i.e. comments that begin with "@").
            // Determine if this is related at all and build the full comment line
            // from the various segments that the line is parsed into.
            $expression = '/^@to/i';
            $comment    = $tokens[$stackPtr]['content'];
            if ((bool) preg_match($expression, $comment) === true) {
                if ($this->debug === true) {
                    echo "Attempting to build comment\n";
                }

                $index = ($stackPtr + 1);
                while ($tokens[$index]['line'] === $tokens[$stackPtr]['line']) {
                    $comment .= $tokens[$index]['content'];
                    $index++;
                }

                if ($this->debug === true) {
                    echo "Result comment = $comment\n";
                }

                $this->checkTodoFormat($phpcsFile, $stackPtr, $comment, $tokens);
            }//end if
        }//end if

    }//end process()


    /**
     * Checks a comment string for the correct syntax.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     * @param string                      $comment   The comment text.
     * @param array<array>                $tokens    The token data.
     *
     * @return void
     */
    private function checkTodoFormat(File $phpcsFile, int $stackPtr, string $comment, array $tokens)
    {
        if ($this->debug === true) {
            echo "Checking \$comment = '$comment'\n";
        }

        $expression = '/(?x)   # Set free-space mode to allow this commenting.
            ^(\/|\s)*          # At the start optionally match any forward slashes and spaces
            (?i)               # set case-insensitive mode.
            (?=(               # Start a positive non-consuming look-ahead to find all possible todos
              @+to(-|\s|)+do   # if one or more @ allow spaces and - between the to and do.
              \h*(-|:)*        # Also match the trailing "-" or ":" so they can be replaced.
              |                # or
              to(-)*do         # If no @ then only accept todo or to-do or to--do, etc, no spaces.
              (\s-|:)*         # Also match the trailing "-" or ":" so they can be replaced.
            ))
            (?-i)              # Reset to case-sensitive
            (?!                # Start another non-consuming look-ahead, this time negative
              @todo\s          # It has to match lower-case @todo followed by one space
              (?!-|:)\S        # and then any non-space except "-" or ":".
            )/m';

        if ((bool) preg_match($expression, $comment, $matches) === true) {
            if ($this->debug === true) {
                echo "Failed regex - give message\n";
            }

            $commentTrimmed = trim($comment, " /\r\n");
            if ($commentTrimmed === '@todo') {
                // We can't fix a comment that doesn't have any text.
                $phpcsFile->addWarning("'%s' should match the format '@todo Fix problem X here.'", $stackPtr, 'TodoFormat', [$commentTrimmed]);
                $fix = false;
            } else {
                // Comments with description text are fixable.
                $fix = $phpcsFile->addFixableWarning("'%s' should match the format '@todo Fix problem X here.'", $stackPtr, 'TodoFormat', [$commentTrimmed]);
            }

            if ($fix === true) {
                if ($tokens[$stackPtr]['code'] === T_DOC_COMMENT_TAG) {
                    // Rewrite the comment past the token content to an empty
                    // string as part of it may be part of the match, but not in
                    // the token content. Then replace the token content with
                    // the fixed comment from the matched content.
                    $phpcsFile->fixer->beginChangeset();
                    $index = ($stackPtr + 1);
                    while ($tokens[$index]['line'] === $tokens[$stackPtr]['line']) {
                        $phpcsFile->fixer->replaceToken($index, '');
                        $index++;
                    }

                    $fixedTodo = str_replace($matches[2], '@todo ', $comment);
                    $phpcsFile->fixer->replaceToken($stackPtr, $fixedTodo);
                    $phpcsFile->fixer->endChangeset();
                } else {
                    // The full comment line text is available here, so the
                    // replacement is fairly straightforward.
                    $fixedTodo = str_replace($matches[2], '@todo ', $tokens[$stackPtr]['content']);
                    $phpcsFile->fixer->replaceToken($stackPtr, $fixedTodo);
                }//end if
            }//end if
        }//end if

    }//end checkTodoFormat()


}//end class
