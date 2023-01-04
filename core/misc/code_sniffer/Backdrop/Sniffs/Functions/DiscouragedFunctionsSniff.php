<?php
/**
 * \Backdrop\Sniffs\Functions\DiscouragedFunctionsSniff.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */

namespace Backdrop\Sniffs\Functions;

use PHP_CodeSniffer\Standards\Generic\Sniffs\PHP\ForbiddenFunctionsSniff;

/**
 * Discourage the use of debug functions.
 *
 * @category PHP
 * @package  PHP_CodeSniffer
 * @link     http://pear.php.net/package/PHP_CodeSniffer
 */
class DiscouragedFunctionsSniff extends ForbiddenFunctionsSniff
{

    /**
     * A list of forbidden functions with their alternatives.
     *
     * The value is NULL if no alternative exists, i.e., the function should
     * just not be used.
     *
     * @var array<string, null>
     */
    public $forbiddenFunctions = [
                                     // Devel module debugging functions.
        'dargs'               => null,
        'dcp'                 => null,
        'dd'                  => null,
        'ddebug_backtrace'    => null,
        'ddm'                 => null,
        'dfb'                 => null,
        'dfbt'                => null,
        'dpm'                 => null,
        'dpq'                 => null,
        'dpr'                 => null,
        'dprint_r'            => null,
        'backdrop_debug'        => null,
        'dsm'                 => null,
        'dvm'                 => null,
        'dvr'                 => null,
        'kdevel_print_object' => null,
        'kint'                => null,
        'ksm'                 => null,
        'kpr'                 => null,
        'kprint_r'            => null,
        'sdpm'                => null,
                                  // Functions which are not available on all
                                  // PHP builds.
        'fnmatch'             => null,
                                  // Functions which are a security risk.
        'eval'                => null,
    ];

    /**
     * If true, an error will be thrown; otherwise a warning.
     *
     * @var boolean
     */
    public $error = false;

}//end class
