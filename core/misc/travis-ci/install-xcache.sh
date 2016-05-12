#!/bin/sh

wget http://xcache.lighttpd.net/pub/Releases/3.2.0/xcache-3.2.0.tar.gz
tar xf xcache-3.2.0.tar.gz
cd xcache-3.2.0
phpize
./configure
make
sudo make install
printf "extension=xcache.so\nxcache.size=64M\nxcache.var_size=16M\nxcache.test=On" > xcache.ini
phpenv config-add xcache.ini
php -v
