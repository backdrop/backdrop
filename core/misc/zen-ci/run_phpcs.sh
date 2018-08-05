#!/bin/sh

##
# Run phpcs code standards tests.
##

# Set site path.
SITEPATH="$HOME/www"

# Go to domain directory.
cd $SITEPATH

# Install and run backdrop/coder
composer require backdrop/coder
./vendor/bin/phpcs --standard=./vendor/backdrop/coder/coder_sniffer/Backdrop core/modules
