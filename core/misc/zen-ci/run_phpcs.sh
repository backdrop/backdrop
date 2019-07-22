#!/bin/sh

##
# Run phpcs code standards tests.
#
# This utilizes the PHP_CodeSniffer (phpcs) library included as part of the
# contrib "coder" module.
##

# Set site path
SITEPATH="$HOME/www"

# Go to domain directory.
cd $SITEPATH

# Install and run backdrop/coder
composer require backdrop/coder

# Get the files that are changed and run phpcs on them.
files=$(git diff --name-only 1.x)
./vendor/bin/phpcs --standard=./vendor/backdrop/coder/coder_sniffer/Backdrop $files
