#!/bin/bash
#
# Starts the Apache manually on the Travis-CI container infrastructure. Because
# the containers are not allowed to use sudo to enable extensions, we manually
# start Apache on port 8080 with a custom configuration file to enable the
# extensions we need to run Backdrop CMS.
#
set -e

export APACHE_RUN_USER=travis
export APACHE_RUN_GROUP=travis
export APACHE_PID_FILE=$TRAVIS_BUILD_DIR/core/misc/travis-ci/apache2.pid
export APACHE_LOCK_DIR=$TRAVIS_BUILD_DIR/core/misc/travis-ci
export APACHE_LOG_DIR=$TRAVIS_BUILD_DIR/core/misc/travis-ci

exec /usr/sbin/apache2 -f $TRAVIS_BUILD_DIR/core/misc/travis-ci/apache2.conf -k start
