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
 * "version", "project" and "timestamp" are added automatically by backdrop.org
 * packaging scripts.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class AutoAddedKeysSniff implements Sniff
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
        if (preg_match('/\.info$/', $phpcsFile->getFilename()) === 1) {
            // Backdrop 7 style info file.
            $contents = file_get_contents($phpcsFile->getFilename());
            $info     = ClassFilesSniff::backdropParseInfoFormat($contents);
        } else if (preg_match('/\.info\.yml$/', $phpcsFile->getFilename()) === 1) {
            // Backdrop 8 style info.yml file.
            $contents = file_get_contents($phpcsFile->getFilename());
            try {
                $info = \Symfony\Component\Yaml\Yaml::parse($contents);
            } catch (\Symfony\Component\Yaml\Exception\ParseException $e) {
                // If the YAML is invalid we ignore this file.
                return ($phpcsFile->numTokens + 1);
            }
        } else {
            return ($phpcsFile->numTokens + 1);
        }

        if (isset($info['project']) === true) {
            $warning = 'Remove "project" from the info file, it will be added by backdrop.org packaging automatically';
            $phpcsFile->addWarning($warning, $stackPtr, 'Project');
        }

        if (isset($info['datestamp']) === true) {
            $warning = 'Remove "datestamp" from the info file, it will be added by backdrop.org packaging automatically';
            $phpcsFile->addWarning($warning, $stackPtr, 'Timestamp');
        }

        // "version" is special: we want to allow it in core, but not anywhere else.
        if (isset($info['version']) === true && strpos($phpcsFile->getFilename(), '/core/') === false) {
            $warning = 'Remove "version" from the info file, it will be added by backdrop.org packaging automatically';
            $phpcsFile->addWarning($warning, $stackPtr, 'Version');
        }

        return ($phpcsFile->numTokens + 1);

    }//end process()


}//end class
