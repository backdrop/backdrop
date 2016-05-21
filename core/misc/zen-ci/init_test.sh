#!/bin/sh
# This script create a website ON test server.

#set site path
SITEPATH="$HOME/www"

# Go to domain dir.
cd $SITEPATH

#link backdrop files
ln -s $GITLC_DEPLOY_DIR/* ./
ln -s $GITLC_DEPLOY_DIR/.htaccess ./

# Unlink settings.php and copy instead.
rm -f settings.php
cp $GITLC_DEPLOY_DIR/settings.php ./

# Unlink files and copy instead.
rm -f files
cp -r $GITLC_DEPLOY_DIR/files ./


#install backdrop

php $SITEPATH/core/scripts/install.sh  --db-url=mysql://test:@localhost/test --root=/home/test/www
