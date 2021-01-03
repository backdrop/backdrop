<?php
/**
 * \Backdrop\Sniffs\NamingConventions\ValidGlobalSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\NamingConventions;

use PHP_CodeSniffer\Files\File;
use PHP_CodeSniffer\Sniffs\Sniff;
use PHP_CodeSniffer\Util\Tokens;

/**
 * Ensures that global variables start with an underscore.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class ValidGlobalSniff implements Sniff
{

    /**
     * List of allowed Backdrop core global variable names.
     *
     * @var array<string>
     */
    public $coreGlobals = [
        '$argc',
        '$argv',
        '$base_insecure_url',
        '$base_path',
        '$base_root',
        '$base_secure_url',
        '$base_theme_info',
        '$base_url',
        '$channel',
        '$conf',
        '$config',
        '$config_directories',
        '$cookie_domain',
        '$databases',
        '$db_prefix',
        '$db_type',
        '$db_url',
        '$backdrop_hash_salt',
        '$backdrop_test_info',
        '$element',
        '$forum_topic_list_header',
        '$image',
        '$install_state',
        '$installed_profile',
        '$is_https',
        '$is_https_mock',
        '$item',
        '$items',
        '$language',
        '$language_content',
        '$language_url',
        '$locks',
        '$menu_admin',
        '$multibyte',
        '$pager_limits',
        '$pager_page_array',
        '$pager_total',
        '$pager_total_items',
        '$tag',
        '$theme',
        '$theme_engine',
        '$theme_info',
        '$theme_key',
        '$theme_path',
        '$timers',
        '$update_free_access',
        '$update_rewrite_settings',
        '$user',
    ];


    /**
     * Returns an array of tokens this test wants to listen for.
     *
     * @return array<int|string>
     */
    public function register()
    {
        return [T_GLOBAL];

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
        $tokens = $phpcsFile->getTokens();

        $varToken = $stackPtr;
        // Find variable names until we hit a semicolon.
        $ignore   = Tokens::$emptyTokens;
        $ignore[] = T_SEMICOLON;
        while (($varToken = $phpcsFile->findNext($ignore, ($varToken + 1), null, true, null, true)) !== false) {
            if ($tokens[$varToken]['code'] === T_VARIABLE
                && in_array($tokens[$varToken]['content'], $this->coreGlobals) === false
                && $tokens[$varToken]['content'][1] !== '_'
            ) {
                $error = 'global variables should start with a single underscore followed by the module and another underscore';
                $phpcsFile->addError($error, $varToken, 'GlobalUnderScore');
            }
        }

    }//end process()


}//end class
