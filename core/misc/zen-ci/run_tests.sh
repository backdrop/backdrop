#!/bin/sh
# This script start tests on test server.

#just testing
SITEPATH="$HOME/www"

echo "Full site path: $SITEPATH"
cd $SITEPATH

php core/scripts/run-tests.sh --url http://localhost --verbose --cache --force --all --concurrency 10 --color --verbose --zenci
