#!/bin/bash
# Exit when any command fails (-e).
set -e

# Add domains to the /etc/hosts file.
sudo echo "127.0.0.1 localhost" >> /etc/hosts

# Replace the entire default VirtualHost site configuration file.
VHOST="
<VirtualHost *:80>
	ServerName localhost

	ServerAdmin webmaster@localhost
	DocumentRoot /var/www/html

  <Directory /var/www/html/>
      Options FollowSymLinks
      AllowOverride All
      DirectoryIndex disabled
      Options -Indexes
  </Directory>

	ErrorLog ${APACHE_LOG_DIR}/error.log
	CustomLog ${APACHE_LOG_DIR}/access.log combined
</VirtualHost>
"
sudo mkdir -p /etc/apache2/sites-available
sudo echo "${VHOST}" > /etc/apache2/sites-available/000-default.conf
