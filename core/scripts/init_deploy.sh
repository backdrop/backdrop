#!/bin/sh
echo "INIT"
printenv

#set site pacth
SITEPATH="$HOME/domains/$DOMAIN"

# Go to domain dir
cd $SITEPATH

#link backdrop files
ln -s $GITLC_DEPLOY_DIR/* ./

#install backdrop
sh $GITLC_DEPLOY_DIR/core/scripts/install.sh --root=$SITEPATH --db-url=mysql://$DATABASE_USER:$DATABASE_PASSWORD@localhost/$DATABASE_NAME --account-mail=$ACCOUNT_MAIL --account-name=$ACCOUNT_NAME --site-mail=$SITE_MAIL --site-name=$SITE_NAME
