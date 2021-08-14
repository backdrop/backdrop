#!/bin/sh

##
# Prepare an environment on a Zen.ci test server in preparation to run tests.
#
# This script is called by the gitlc.yml file on backdropcms.org:
# https://github.com/backdrop-ops/backdropcms.org/tree/master/www/modules/custom/borg_qa
##

# Set site path variable.
SITEPATH="$HOME/www"

# Remove the old site path.
rm -rf $SITEPATH

# Link to the git checkout.
ln -s $GITLC_DEPLOY_DIR $SITEPATH

# Install Backdrop.
php $SITEPATH/core/scripts/install.sh  --db-url=mysql://test:@localhost/test --root=/home/test/www
