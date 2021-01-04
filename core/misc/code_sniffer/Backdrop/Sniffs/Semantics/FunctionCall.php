<?php
/**
 * \Backdrop\Sniffs\Semantics\FunctionCall.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Helper class to sniff for specific function calls.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
abstract class FunctionCall implements Sniff
{

    /**
     * The currently processed file.
     *
     * @var \PHP_CodeSniffer\Files\File
     */
    protected $phpcsFile;

    /**
     * The token position of the function call.
     *
     * @var integer
     */
    protected $functionCall;

    /**
     * The token position of the opening bracket of the function call.
     *
     * @var integer
     */
    protected $openBracket;

    /**
     * The token position of the closing bracket of the function call.
     *
     * @var integer
     */
    protected $closeBracket;

    /**
     * Internal cache to save the calculated arguments of the function call.
     *
     * @var array<int, array>
     */
    protected $arguments;

    /**
     * Whether method invocations with the same function name should be processed,
     * too.
     *
     * @var boolean
     */
    protected $includeMethodCalls = false;


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_STRING];

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
        $functionName = $tokens[$stackPtr]['content'];
        if (in_array($functionName, $this->registerFunctionNames()) === false) {
            // Not interested in this function.
            return;
        }

        if ($this->isFunctionCall($phpcsFile, $stackPtr) === false) {
            return;
        }

        // Find the next non-empty token.
        $openBracket = $phpcsFile->findNext(Tokens::$emptyTokens, ($stackPtr + 1), null, true);

        $this->phpcsFile    = $phpcsFile;
        $this->functionCall = $stackPtr;
        $this->openBracket  = $openBracket;
        $this->closeBracket = $tokens[$openBracket]['parenthesis_closer'];
        $this->arguments    = [];

        $this->processFunctionCall($phpcsFile, $stackPtr, $openBracket, $this->closeBracket);

    }//end process()


    /**
     * Checks if this is a function call.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     *
     * @return bool
     */
    protected function isFunctionCall(File $phpcsFile, $stackPtr)
    {
        $tokens = $phpcsFile->getTokens();
        // Find the next non-empty token.
        $openBracket = $phpcsFile->findNext(Tokens::$emptyTokens, ($stackPtr + 1), null, true);

        if ($tokens[$openBracket]['code'] !== T_OPEN_PARENTHESIS) {
            // Not a function call.
            return false;
        }

        if (isset($tokens[$openBracket]['parenthesis_closer']) === false) {
            // Not a function call.
            return false;
        }

        // Find the previous non-empty token.
        $search   = Tokens::$emptyTokens;
        $search[] = T_BITWISE_AND;
        $previous = $phpcsFile->findPrevious($search, ($stackPtr - 1), null, true);
        if ($tokens[$previous]['code'] === T_FUNCTION) {
            // It's a function definition, not a function call.
            return false;
        }

        if ($tokens[$previous]['code'] === T_OBJECT_OPERATOR && $this->includeMethodCalls === false) {
            // It's a method invocation, not a function call.
            return false;
        }

        if ($tokens[$previous]['code'] === T_DOUBLE_COLON && $this->includeMethodCalls === false) {
            // It's a static method invocation, not a function call.
            return false;
        }

        return true;

    }//end isFunctionCall()


    /**
     * Returns start and end token for a given argument number.
     *
     * @param int $number Indicates which argument should be examined, starting with
     *                    1 for the first argument.
     *
     * @return array<string, int>|false
     */
    public function getArgument($number)
    {
        // Check if we already calculated the tokens for this argument.
        if (isset($this->arguments[$number]) === true) {
            return $this->arguments[$number];
        }

        $tokens = $this->phpcsFile->getTokens();
        // Start token of the first argument.
        $start = $this->phpcsFile->findNext(Tokens::$emptyTokens, ($this->openBracket + 1), null, true);
        if ($start === $this->closeBracket) {
            // Function call has no arguments, so return false.
            return false;
        }

        // End token of the last argument.
        $end           = $this->phpcsFile->findPrevious(Tokens::$emptyTokens, ($this->closeBracket - 1), null, true);
        $lastArgEnd    = $end;
        $nextSeperator = $this->openBracket;
        $counter       = 1;
        while (($nextSeperator = $this->phpcsFile->findNext(T_COMMA, ($nextSeperator + 1), $this->closeBracket)) !== false) {
            // Make sure the comma belongs directly to this function call,
            // and is not inside a nested function call or array.
            $brackets    = $tokens[$nextSeperator]['nested_parenthesis'];
            $lastBracket = array_pop($brackets);
            if ($lastBracket !== $this->closeBracket) {
                continue;
            }

            // Update the end token of the current argument.
            $end = $this->phpcsFile->findPrevious(Tokens::$emptyTokens, ($nextSeperator - 1), null, true);
            // Save the calculated findings for the current argument.
            $this->arguments[$counter] = [
                'start' => $start,
                'end'   => $end,
            ];
            if ($counter === $number) {
                break;
            }

            $counter++;
            $start = $this->phpcsFile->findNext(Tokens::$emptyTokens, ($nextSeperator + 1), null, true);
            $end   = $lastArgEnd;
        }//end while

        // If the counter did not reach the passed number something is wrong.
        if ($counter !== $number) {
            return false;
        }

        $this->arguments[$counter] = [
            'start' => $start,
            'end'   => $end,
        ];
        return $this->arguments[$counter];

    }//end getArgument()


}//end class
