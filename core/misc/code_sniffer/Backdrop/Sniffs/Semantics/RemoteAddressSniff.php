<?php
/**
 * \Backdrop\Sniffs\Semantics\RemoteAddressSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Semantics;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * Make sure that ip_address() or Backdrop::request()->getClientIp() is used instead of
 * $_SERVER['REMOTE_ADDR'].
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class RemoteAddressSniff implements Sniff
{


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_VARIABLE];

    }//end register()


    /**
     * Processes this test, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The current file being processed.
     * @param int                         $stackPtr  The position of the current token
     *                                               in the stack passed in $tokens.
     *
     * @return void
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        $string           = $phpcsFile->getTokensAsString($stackPtr, 4);
        $startOfStatement = $phpcsFile->findStartOfStatement($stackPtr);
        if (($string === '$_SERVER["REMOTE_ADDR"]' || $string === '$_SERVER[\'REMOTE_ADDR\']') && $stackPtr !== $startOfStatement) {
            $error = 'Use ip_address() or Backdrop::request()->getClientIp() instead of $_SERVER[\'REMOTE_ADDR\']';
            $phpcsFile->addError($error, $stackPtr, 'RemoteAddress');
        }

    }//end process()


}//end class
