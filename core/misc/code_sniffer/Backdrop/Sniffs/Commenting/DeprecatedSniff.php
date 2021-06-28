<?php
/**
 * \Backdrop\Sniffs\Commenting\DeprecatedSniff.
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
 * Ensures standard format of @ deprecated tag text in docblock.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class DeprecatedSniff implements Sniff
{

    /**
     * Show debug output for this sniff.
     *
     * Use phpcs --runtime-set deprecated_debug true
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

        return [T_DOC_COMMENT_TAG];

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
        $debug = Config::getConfigData('deprecated_debug');
        if ($debug !== null) {
            $this->debug = (bool) $debug;
        }

        $tokens = $phpcsFile->getTokens();

        // Only process @deprecated tags.
        if (strcasecmp($tokens[$stackPtr]['content'], '@deprecated') !== 0) {
            return;
        }

        // Get the end point of the comment block which has the deprecated tag.
        $commentEnd = $phpcsFile->findNext(T_DOC_COMMENT_CLOSE_TAG, ($stackPtr + 1));

        // Get the full @deprecated text which may cover multiple lines.
        $textItems = [];
        $lastLine  = $tokens[($stackPtr + 1)]['line'];
        for ($i = ($stackPtr + 1); $i < $commentEnd; $i++) {
            if ($tokens[$i]['code'] === T_DOC_COMMENT_STRING) {
                if ($tokens[$i]['line'] <= ($lastLine + 1)) {
                    $textItems[$i] = $tokens[$i]['content'];
                    $lastLine      = $tokens[$i]['line'];
                } else {
                    break;
                }
            }

            // Found another tag, so we have all the deprecation text.
            if ($tokens[$i]['code'] === T_DOC_COMMENT_TAG) {
                break;
            }
        }

        // The standard format for the deprecation text is:
        // @deprecated in %in-version% and is removed from %removal-version%. %extra-info%.
        $standardFormat = "@deprecated in %%deprecation-version%% and is removed from %%removal-version%%. %%extra-info%%.";

        // Use (?U) 'ungreedy' before the removal-version so that only the text
        // up to the first dot+space is matched, as there may be more than one
        // sentence in the extra-info part.
        $fullText = trim(implode(' ', $textItems));
        $matches  = [];
        preg_match('/^in (.+) and is removed from (?U)(.+)(?:\. | |\.$|$)(.*)$/', $fullText, $matches);
        // There should be 4 items in $matches: 0 is full text, 1 = in-version,
        // 2 = removal-version, 3 = extra-info (can be blank at this stage).
        if (count($matches) !== 4) {
            // The full text does not match the standard. Try to find fixes by
            // testing with a relaxed set of criteria, based on common
            // formatting variations. This is designed for Core fixes only.
            $error = "The text '@deprecated %s' does not match the standard format: ".$standardFormat;
            // All of the standard text should be on the first comment line, so
            // try to match with common formatting errors to allow an automatic
            // fix. If not possible then report a normal error.
            $matchesFix = [];
            $fix        = null;
            if (count($textItems) > 0) {
                // Get just the first line of the text.
                $key   = array_keys($textItems)[0];
                $text1 = $textItems[$key];
                // Matching on (backdrop|) here says that we are only attempting to provide
                // automatic fixes for Backdrop core, and if the project is missing we are
                // assuming it is Backdrop core. Deprecations for contrib projects are much
                // less frequent and faults can be corrected manually.
                preg_match('/^(.*)(as of|in) (backdrop|)( |:|)+([\d\.\-xdev\?]+)(,| |. |)(.*)(removed|removal)([ |from|before|in|the]*) (backdrop|)( |:|)([\d\-\.xdev]+)( |,|$)+(?:release|)(?:[\.,])*(.*)$/i', $text1, $matchesFix);

                if (count($matchesFix) >= 12) {
                    // It is a Backdrop core deprecation and is fixable.
                    if (empty($matchesFix[1]) === false && $this->debug === true) {
                        // For info, to check it is acceptable to remove the text in [1].
                        echo('DEBUG: File: '.$phpcsFile->path.', line '.$tokens[($stackPtr)]['line'].PHP_EOL);
                        echo('DEBUG: "@deprecated '.$text1.'"'.PHP_EOL);
                        echo('DEBUG: Fix will remove: "'.$matchesFix[1].'"'.PHP_EOL);
                    }

                    $ver1 = str_Replace(['-dev', 'x'], ['', '0'], trim($matchesFix[5], '.'));
                    $ver2 = str_Replace(['-dev', 'x'], ['', '0'], trim($matchesFix[12], '.'));
                    // If the version is short, add enough '.0' to correct it.
                    while (substr_count($ver1, '.') < 2) {
                        $ver1 .= '.0';
                    }

                    while (substr_count($ver2, '.') < 2) {
                        $ver2 .= '.0';
                    }

                    $correctedText = trim('in backdrop:'.$ver1.' and is removed from backdrop:'.$ver2.'. '.trim($matchesFix[14]));
                    // If $correctedText is longer than 65 this will make the whole line
                    // exceed 80 so give a warning if running with debug.
                    if (strlen($correctedText) > 65 && $this->debug === true) {
                        echo('WARNING: File '.$phpcsFile->path.', line '.$tokens[($stackPtr)]['line'].PHP_EOL);
                        echo('WARNING: Original  = * @deprecated '.$text1.PHP_EOL);
                        echo('WARNING: Corrected = * @deprecated '.$correctedText.PHP_EOL);
                        echo('WARNING: New line length '.(strlen($correctedText) + 15).' exceeds standard 80 character limit'.PHP_EOL);
                    }

                    $fix = $phpcsFile->addFixableError($error, $key, 'IncorrectTextLayout', [$fullText]);
                    if ($fix === true) {
                        $phpcsFile->fixer->replaceToken($key, $correctedText);
                    }
                }//end if
            }//end if

            if ($fix === null) {
                // There was no automatic fix, so give a normal error.
                $phpcsFile->addError($error, $stackPtr, 'IncorrectTextLayout', [$fullText]);
            }
        } else {
            // The text follows the basic layout. Now check that the versions
            // match backdrop:n.n.n or project:n.x-n.n or project:n.x-n.n-version[n]
            // or project:n.n.n or project:n.n.n-version[n].
            // The text must be all lower case and numbers can be one or two digits.
            foreach (['deprecation-version' => $matches[1], 'removal-version' => $matches[2]] as $name => $version) {
                if (preg_match('/^[a-z\d_]+:(\d{1,2}\.\d{1,2}\.\d{1,2}|\d{1,2}\.x\-\d{1,2}\.\d{1,2})(-[a-z]{1,5}\d{1,2})?$/', $version) === 0) {
                    $error = "The %s '%s' does not match the lower-case machine-name standard: backdrop:n.n.n or project:n.x-n.n or project:n.x-n.n-version[n] or project:n.n.n or project:n.n.n-version[n]";
                    $phpcsFile->addWarning($error, $stackPtr, 'DeprecatedVersionFormat', [$name, $version]);
                }
            }

            // The 'IncorrectTextLayout' above is designed to pass if all is ok
            // except for missing extra info. This is a common fault so provide
            // a separate check and message for this.
            if ($matches[3] === '') {
                $error = 'The @deprecated tag must have %extra-info%. The standard format is: '.str_replace('%%', '%', $standardFormat);
                $phpcsFile->addError($error, $stackPtr, 'MissingExtraInfo', []);
            }
        }//end if

        // The next tag in this comment block after @deprecated must be @see.
        $seeTag = $phpcsFile->findNext(T_DOC_COMMENT_TAG, ($stackPtr + 1), $commentEnd, false, '@see');
        if ($seeTag === false) {
            $error = 'Each @deprecated tag must have a @see tag immediately following it';
            $phpcsFile->addError($error, $stackPtr, 'DeprecatedMissingSeeTag');
            return;
        }

        // Check the format of the @see url.
        $string = $phpcsFile->findNext(T_DOC_COMMENT_STRING, ($seeTag + 1), $commentEnd);
        // If the @see tag exists but has no content then $string will be empty
        // and $tokens[$string]['content'] will return '<?php' which makes the
        // standards message confusing. Better to set crLink to blank here.
        if ($string === false) {
            $crLink = ' ';
        } else {
            $crLink = $tokens[$string]['content'];
        }

        // Allow for the alternative 'node' or 'project/aaa/issues' format.
        preg_match('[^http(s*)://www.backdrop.org/(node|project/\w+/issues)/(\d+)([\.\?\;!]*)$]', $crLink, $matches);
        if (isset($matches[4]) === true && empty($matches[4]) === false) {
            // If matches[4] is not blank it means that the url is OK but it
            // ends with punctuation. This is a common and fixable mistake.
            $error = "The @see url '%s' should not end with punctuation";
            $fix   = $phpcsFile->addFixableError($error, $string, 'DeprecatedPeriodAfterSeeUrl', [$crLink]);
            if ($fix === true) {
                // Remove all of the the trailing punctuation.
                $content = substr($crLink, 0, -(strlen($matches[4])));
                $phpcsFile->fixer->replaceToken($string, $content);
            }//end if
        } else if (empty($matches) === true) {
            $error = "The @see url '%s' does not match the standard: http(s)://www.backdrop.org/node/n or http(s)://www.backdrop.org/project/aaa/issues/n";
            $phpcsFile->addWarning($error, $seeTag, 'DeprecatedWrongSeeUrlFormat', [$crLink]);
        }

    }//end process()


}//end class
