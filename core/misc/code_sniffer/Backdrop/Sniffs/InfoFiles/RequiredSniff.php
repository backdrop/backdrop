<?php
/**
 * \Backdrop\Sniffs\InfoFiles\RequiredSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\InfoFiles;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * "name", "description" and "core are required fields in Backdrop info files. Also
 * checks the "php" minimum requirement for Backdrop 7.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class RequiredSniff implements Sniff
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
     * @return int
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        // Only run this sniff once per info file.
        $fileExtension = strtolower(substr($phpcsFile->getFilename(), -4));
        if ($fileExtension !== 'info') {
            return ($phpcsFile->numTokens + 1);
        }

        $contents = file_get_contents($phpcsFile->getFilename());
        $info     = ClassFilesSniff::backdropParseInfoFormat($contents);
        if (isset($info['name']) === false) {
            $error = '"name" property is missing in the info file';
            $phpcsFile->addError($error, $stackPtr, 'Name');
        }

        if (isset($info['description']) === false) {
            $error = '"description" property is missing in the info file';
            $phpcsFile->addError($error, $stackPtr, 'Description');
        }

        if (isset($info['backdrop']) === false) {
            $error = '"backdrop" property is missing in the info file';
            $phpcsFile->addError($error, $stackPtr, 'Core version');
        }

        return ($phpcsFile->numTokens + 1);

    }//end process()


}//end class
