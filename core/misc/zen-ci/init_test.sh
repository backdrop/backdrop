#!/bin/sh

##
# Prepare an environment on a Zen.ci test server in preparation to run tests.
#
# This script is called by the gitlc.yml file on backdropcms.org:
# https://github.com/backdrop-ops/backdropcms.org/tree/master/www/modules/custom/borg_qa
##

#set site path
SITEPATH="$HOME/www"

# Go to domain directory.
cd $SITEPATH

# Link Backdrop files
ln -s $GITLC_DEPLOY_DIR/* ./
ln -s $GITLC_DEPLOY_DIR/.htaccess ./

# Unlink settings.php and copy instead.
rm -f settings.php
cp $GITLC_DEPLOY_DIR/settings.php ./

# Unlink files and copy instead.
rm -f files
cp -r $GITLC_DEPLOY_DIR/files ./

# Move files to /dev/shm
mv $SITEPATH/files /dev/shm/files
ln -s /dev/shm/files $SITEPATH
 
# Install Backdrop.
php $SITEPATH/core/scripts/install.sh  --db-url=mysql://test:@localhost/test --root=/home/test/www
