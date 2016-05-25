#!/bin/sh
# This script start tests on test server.

#just testing
SITEPATH="$HOME/www"

echo "Full site path: $SITEPATH"
cd $SITEPATH

curl -X PUT -H "Content-Type: application/json" -H "Token: $GITLC_API_TOKEN" $GITLC_STATUS_URL --data '{"state": "pending", "message": "Processing Tests"}' > /dev/null

php core/scripts/run-tests.sh --url http://localhost --verbose --cache --force --all --concurrency 10 --color --verbose --summary /tmp/summary

if [ $? -eq 0 ]; then 
  MESSAGE=`cat /tmp/summary| sed -n 1p| tr '\n' ' '`
  curl -X PUT -H "Content-Type: application/json" -H "Token: $GITLC_API_TOKEN" $GITLC_STATUS_URL --data "{\"state\": \"success\", \"message\": \"$MESSAGE\"}" > /dev/null
else
  MESSAGE=`cat /tmp/summary| sed -n 1p| tr '\n' ' '`
  SUMMARY=`cat /tmp/summary| sed ':a;N;$!ba;s/\n/\\n/g' | sed ':a;N;$!ba;s/"/\"/g'`
  curl -X PUT -H "Content-Type: application/json" -H "Token: $GITLC_API_TOKEN" $GITLC_STATUS_URL --data "{\"state\": \"error\", \"message\": \"$MESSAGE\", \"summary\": \"$SUMMARY\" }" > /dev/null
  exit 1
fi
