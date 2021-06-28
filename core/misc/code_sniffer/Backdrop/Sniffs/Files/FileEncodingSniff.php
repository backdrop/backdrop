<?php
/**
 * \Backdrop\Sniffs\Files\FileEncodingSniff.
 *
 * @category  PHP
 * @package   PHP_CodeSniffer
 * @author    Klaus Purer <klaus.purer@mail.com>
 * @copyright 2016 Klaus Purer
 * @license   https://github.com/squizlabs/PHP_CodeSniffer/blob/master/licence.txt BSD Licence
 * @link      http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Files;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;

/**
 * \Backdrop\Sniffs\Files\FileEncodingSniff.
 *
 * Validates the encoding of a file against a white list of allowed encodings.
 *
 * @category  PHP
 * @package   PHP_CodeSniffer
 * @author    Klaus Purer <klaus.purer@mail.com>
 * @copyright 2016 Klaus Purer
 * @license   https://github.com/squizlabs/PHP_CodeSniffer/blob/master/licence.txt BSD Licence
 * @version   Release: @package_version@
 * @link      http://pear.php.net/package/PHP_CodeSniffer
 */
class FileEncodingSniff implements Sniff
{

    /**
     * List of encodings that files may be encoded with.
     *
     * Any other detected encodings will throw a warning.
     *
     * @var array<string>
     */
    public $allowedEncodings = ['UTF-8'];


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [
            T_INLINE_HTML,
            T_OPEN_TAG,
        ];

    }//end register()


    /**
     * Processes this sniff, when one of its tokens is encountered.
     *
     * @param \PHP_CodeSniffer\Files\File $phpcsFile The file being scanned.
     * @param int                         $stackPtr  The position of the current token in
     *                                               the stack passed in $tokens.
     *
     * @return int|void
     */
    public function process(File $phpcsFile, $stackPtr)
    {
        // Not all PHP installs have the multi byte extension - nothing we can do.
        if (function_exists('mb_check_encoding') === false) {
            return $phpcsFile->numTokens;
        }

        $fileContent = $phpcsFile->getTokensAsString(0, $phpcsFile->numTokens);

        $validEncodingFound = false;
        foreach ($this->allowedEncodings as $encoding) {
            if (mb_check_encoding($fileContent, $encoding) === true) {
                $validEncodingFound = true;
            }
        }

        if ($validEncodingFound === false) {
            $warning = 'File encoding is invalid, expected %s';
            $data    = [implode(' or ', $this->allowedEncodings)];
            $phpcsFile->addWarning($warning, $stackPtr, 'InvalidEncoding', $data);
        }

        return $phpcsFile->numTokens;

    }//end process()


}//end class
