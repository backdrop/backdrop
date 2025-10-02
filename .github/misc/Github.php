<?php
namespace PHP_CodeSniffer\Reports;

use PHP_CodeSniffer\Files\File;

/**
 * Print reports for GitHub Actions.
 *
 * @see https://github.com/PHPCSStandards/PHP_CodeSniffer/blob/4.x/src/Reports/Report.php
 */
class Github implements Report {

  /**
   * Generate a partial report for a single processed file.
   */
  public function generateFileReport($report, File $phpcsFile, $showSources=FALSE, $width=80) {
    if ($report['errors'] === 0 && $report['warnings'] === 0) {
      // Nothing to print.
      return FALSE;
    }

    foreach ($report['messages'] as $line => $lineErrors) {
      foreach ($lineErrors as $column => $colErrors) {
        foreach ($colErrors as $error) {
          $type = strtolower($error['type']);
          $file = $report['filename'];
          $message = $error['message'];
          echo "::$type file=$file,line=$line,col=$column::$message" . PHP_EOL;
        }
      }
    }

    return TRUE;
  }

  /**
   * Generates a GitHub Actions report.
   */
  public function generate($cachedData, $totalFiles, $totalErrors, $totalWarnings, $totalFixable, $showSources=FALSE, $width=80, $interactive=FALSE, $toScreen=TRUE) {
    echo $cachedData;
  }

}
