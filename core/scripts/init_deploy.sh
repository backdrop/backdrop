#!/bin/sh
# This script create a website based on ENV variables provided by qithab-auto

#set site path
SITEPATH="$HOME/domains/$DOMAIN"

# Go to domain dir
cd $SITEPATH

#link backdrop files
ln -s $GITLC_DEPLOY_DIR/* ./
ln -s $GITLC_DEPLOY_DIR/.htaccess ./

#install backdrop
php $GITLC_DEPLOY_DIR/core/scripts/install.sh --root=$SITEPATH --db-url=mysql://$DATABASE_USER:$DATABASE_PASSWORD@localhost/$DATABASE_NAME --account-mail=$ACCOUNT_MAIL --account-name=$ACCOUNT_NAME --account-pass="$DATABASE_PASSWORD" --site-mail=$SITE_MAIL --site-name="$SITE_NAME"
echo "USER: $ACCOUNT_NAME PASS: $DATABASE_PASSWORD"
