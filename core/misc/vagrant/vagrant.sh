#!/bin/bash
# Centos 6.5 dependencies for Backdrop
# get the epel and remi repo listings so we can get additional packages like mcrypt
wget http://dl.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm
wget http://rpms.famillecollet.com/enterprise/remi-release-6.rpm
rpm -Uvh remi-release-6*.rpm epel-release-6*.rpm
# make sure we're up to date w/ the remi repos
yes | yum update
# using yum to install the main packages
yes | yum -y install patch git nano gcc make mysql mysql-server httpd php php-gd php-xml php-pdo php-mbstring php-mysql php-pear php-devel php-pecl-ssh2 php-pecl-apc php-mcrypt*

yes | yum groupinstall 'Development Tools'
# using pecl to install uploadprogress
pecl channel-update pecl.php.net
# uploadprogress to get rid of that warning
pecl install uploadprogress

# adding uploadprogresss to php
touch /etc/php.d/uploadprogress.ini
echo extension=uploadprogress.so > /etc/php.d/uploadprogress.ini

# set httpd_can_sendmail so  emails go out
setsebool -P httpd_can_sendmail on
# home directory ease of use for root
cd ~
ln -s /var/www/html html
echo "alias l='ls -laHF'" >> .bashrc
# establish for vagrant user
cd /home/vagrant
ln -s /var/www/html html
echo "alias l='ls -laHF'" >> .bashrc
# move to vagrant directory
cd /var/www/html/core/misc/vagrant
# basic http conf file
cp backdrop.conf /etc/httpd/conf.d/backdrop.conf
# common browser performance boost
mkdir -p /etc/httpd/conf.d/web_performance
cp web_performance/* -rf /etc/httpd/conf.d/web_performance/
# optimize apc / php / mysql w/ sane defaults
yes | cp backdrop.conf /etc/php.d/apc.ini
yes | cp php.ini /etc/php.ini
yes | cp my.cnf /etc/my.cnf
# start apache/mysql to ensure that it is running
/etc/init.d/mysqld restart
/etc/init.d/httpd restart
# make an admin group
groupadd admin
mysql -e 'SET GLOBAL wait_timeout = 5400;' -u root
mysql -e 'create database backdrop;' -u root
# Install Backdrop with the installation script.
cd /var/www/html/
chmod a+w . settings.php
./core/scripts/install.sh --db-url=mysql://root:@127.0.0.1/backdrop --account-name=admin --account-pass=admin
chmod go-w . settings.php
