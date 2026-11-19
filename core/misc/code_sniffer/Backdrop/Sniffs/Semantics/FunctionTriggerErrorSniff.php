<?php
/**
 * \Backdrop\Sniffs\Semanitcs\FunctionTriggerErrorSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;

/**
 * Checks that the trigger_error deprecation text message adheres to standards.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class FunctionTriggerErrorSniff extends FunctionCall
{


    /**
     * Returns an array of function names this test wants to listen for.
     *
     * @return array<string>
     */
    public function registerFunctionNames()
    {
        return ['trigger_error'];

    }//end registerFunctionNames()


    /**
     * Processes this function call.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile    The file being scanned.
     * @param int                         $stackPtr     The position of the function call in
     *                                                  the stack.
     * @param int                         $openBracket  The position of the opening
     *                                                  parenthesis in the stack.
     * @param int                         $closeBracket The position of the closing
     *                                                  parenthesis in the stack.
     *
     * @return void
     */
    public function processFunctionCall(
        file $phpcsFile,
        $stackPtr,
        $openBracket,
        $closeBracket
    ) {

        $tokens = $phpcsFile->getTokens();

        // If no second argument then quit.
        if ($this->getArgument(2) === false) {
            return;
        }

        // Only check deprecation messages.
        if (strcasecmp($tokens[$this->getArgument(2)['start']]['content'], 'E_USER_DEPRECATED') !== 0) {
            return;
        }

        // Get the first argument passed to trigger_error().
        $argument = $this->getArgument(1);

        // Extract the message text to check. If if it formed using sprintf()
        // then find the single overall string using ->findNext.
        if ($tokens[$argument['start']]['code'] === T_STRING
            && strcasecmp($tokens[$argument['start']]['content'], 'sprintf') === 0
        ) {
            $messagePosition = $phpcsFile->findNext(T_CONSTANT_ENCAPSED_STRING, $argument['start']);
            // Remove the quotes using substr, because trim would take multiple
            // quotes away and possibly not report a faulty message.
            $messageText = substr($tokens[$messagePosition]['content'], 1, ($tokens[$messagePosition]['length'] - 2));
        } else {
            $messageParts = [];
            // If not sprintf() then extract and store all the items except
            // whitespace, concatenation operators and comma. This will give all
            // real content such as concatenated strings and constants.
            for ($i = $argument['start']; $i <= $argument['end']; $i++) {
                if (in_array($tokens[$i]['code'], [T_WHITESPACE, T_STRING_CONCAT, T_COMMA]) === false) {
                    // For strings, remove the quotes using substr not trim.
                    // Simple strings are T_CONSTANT_ENCAPSED_STRING and strings
                    // with variable interpolation are T_DOUBLE_QUOTED_STRING.
                    if ($tokens[$i]['code'] === T_CONSTANT_ENCAPSED_STRING || $tokens[$i]['code'] === T_DOUBLE_QUOTED_STRING) {
                        $messageParts[] = substr($tokens[$i]['content'], 1, ($tokens[$i]['length'] - 2));
                    } else {
                        $messageParts[] = $tokens[$i]['content'];
                    }
                }
            }

            $messageText = implode(' ', $messageParts);
        }//end if

        // Check if there is a @deprecated tag in an associated doc comment
        // block. If the @trigger_error was level 0 (entire class or file) then
        // try to find a doc comment after the trigger_error also at level 0.
        // If the @trigger_error was at level > 0 it means it is inside a
        // function so search backwards for the function comment block, which
        // will be at one level lower.
        $strictStandard    = false;
        $triggerErrorLevel = $tokens[$stackPtr]['level'];
        if ($triggerErrorLevel === 0) {
            $requiredLevel = 0;
            $block         = $phpcsFile->findNext(T_DOC_COMMENT_OPEN_TAG, $argument['start']);
        } else {
            $requiredLevel = ($triggerErrorLevel - 1);
            $block         = $phpcsFile->findPrevious(T_DOC_COMMENT_OPEN_TAG, $argument['start']);
        }

        if (isset($tokens[$block]['level']) === true
            && $tokens[$block]['level'] === $requiredLevel
            && isset($tokens[$block]['comment_tags']) === true
        ) {
            foreach ($tokens[$block]['comment_tags'] as $tag) {
                $strictStandard = $strictStandard || (strtolower($tokens[$tag]['content']) === '@deprecated');
            }
        }

        // The string standard format for @trigger_error() is:
        // %thing% is deprecated in %deprecation-version% and is removed in
        // %removal-version%. %extra-info%. See %cr-link%
        // For the 'relaxed' standard the 'and is removed in' can be replaced
        // with any text.
        $matches = [];
        if ($strictStandard === true) {
            // Use (?U) 'ungreedy' before the version so that only the text up
            // to the first period followed by a space is matched, as there may
            // be more than one sentence in the extra-info part.
            preg_match('/(.+) is deprecated in (\S+) (and is removed from) (?U)(.+)\. (.*)\. See (\S+)$/', $messageText, $matches);
            $sniff = 'TriggerErrorTextLayoutStrict';
            $error = "The trigger_error message '%s' does not match the strict standard format: %%thing%% is deprecated in %%deprecation-version%% and is removed from %%removal-version%%. %%extra-info%%. See %%cr-link%%";
        } else {
            // Allow %extra-info% to be empty as this is optional in the relaxed
            // version.
            preg_match('/(.+) is deprecated in (\S+) (?U)(.+) (\S+)\. (.*)See (\S+)$/', $messageText, $matches);
            $sniff = 'TriggerErrorTextLayoutRelaxed';
            $error = "The trigger_error message '%s' does not match the relaxed standard format: %%thing%% is deprecated in %%deprecation-version%% any free text %%removal-version%%. %%extra-info%%. See %%cr-link%%";
        }

        // There should be 7 items in $matches: 0 is full text, 1 = thing,
        // 2 = deprecation-version, 3 = middle text, 4 = removal-version,
        // 5 = extra-info, 6 = cr-link.
        if (count($matches) !== 7) {
            $phpcsFile->addError($error, $argument['start'], $sniff, [$messageText]);
        } else {
            // The text follows the basic layout. Now check that the version
            // matches backdrop:n.n.n or project:n.x-n.n. The text must be all
            // lower case and numbers can be one or two digits.
            foreach (['deprecation-version' => $matches[2], 'removal-version' => $matches[4]] as $name => $version) {
                if (preg_match('/^backdrop:\d{1,2}\.\d{1,2}\.\d{1,2}$/', $version) === 0
                    && preg_match('/^[a-z\d_]+:\d{1,2}\.x\-\d{1,2}\.\d{1,2}$/', $version) === 0
                ) {
                    $error = "The %s '%s' does not match the lower-case machine-name standard: backdrop:n.n.n or project:n.x-n.n";
                    $phpcsFile->addWarning($error, $argument['start'], 'TriggerErrorVersion', [$name, $version]);
                }
            }

            // Check the 'See' link.
            $crLink = $matches[6];
            // Allow for the alternative 'node' or 'project/aaa/issues' format.
            preg_match('[^http(s*)://www.backdrop.org/(node|project/\w+/issues)/(\d+)(\.*)$]', $crLink, $crMatches);
            // If cr_matches[4] is not blank it means that the url is correct
            // but it ends with a period. As this can be a common mistake give a
            // specific message to assist in fixing.
            if (isset($crMatches[4]) === true && empty($crMatches[4]) === false) {
                $error = "The url '%s' should not end with a period.";
                $phpcsFile->addWarning($error, $argument['start'], 'TriggerErrorPeriodAfterSeeUrl', [$crLink]);
            } else if (empty($crMatches) === true) {
                $error = "The url '%s' does not match the standard: http(s)://www.backdrop.org/node/n or http(s)://www.backdrop.org/project/aaa/issues/n";
                $phpcsFile->addWarning($error, $argument['start'], 'TriggerErrorSeeUrlFormat', [$crLink]);
            }
        }//end if

    }//end processFunctionCall()


}//end class
