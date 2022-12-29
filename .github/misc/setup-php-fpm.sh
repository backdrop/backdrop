#!/bin/bash
#
# Helper script for Github Actions to tweak php-fpm without bloating the
# workflow file.

if [ $# -lt 1 ]; then
  echo 'Fatal: Parameter missing'
  exit 1
fi

# Configure php-fpm to run as user "runner". That makes moving files around
# obsolete. Additionally tweak it for better performance, start and allow more
# child processes. This is done in all config files, sed is fast.
sudo sed -i -e 's/user = www-data/user = runner/' \
  -e 's/listen.owner = www-data/listen.owner = runner/' \
  -e 's/pm.max_children = 5/pm.max_children = 15/' \
  -e 's/pm.start_servers = 2/pm.start_servers = 4/' \
  -e 's/pm.min_spare_servers = 1/pm.min_spare_servers = 2/' \
  -e 's/pm.max_spare_servers = 3/pm.max_spare_servers = 4/' \
  ls /etc/php/*/fpm/pool.d/www.conf

# Let above changes take effect and setup Apache to work with php-fpm.
sudo systemctl restart php${1}-fpm.service
sudo apt-get -q install libapache2-mod-fcgid
sudo a2enmod rewrite proxy fcgid proxy_fcgi
sudo systemctl start apache2.service

exit 0
