#!/bin/sh
# This script start tests on test server.

#just testing
SITEPATH="$HOME/www"

echo "Full site path: $SITEPATH"
cd $SITEPATH

curl -X PUT -H "Content-Type: application/json" -H "Token: $GITLC_API_TOKEN" $GITLC_STATUS_URL -v --data '{"state": "pending", "message": "Processing Tests"}'

php core/scripts/run-tests.sh --url http://localhost --verbose --cache --force --all --concurrency 10 --color --verbose --summary /tmp/summary

if [ $? -eq 0 ]; then 
  MESSAGE=`cat /tmp/summary| sed -n 1p| tr '\n' ' '`
  echo '{"state": "success", "message": "'$MESSAGE'"}'
  curl -X PUT -H "Content-Type: application/json" -H "Token: $GITLC_API_TOKEN" $GITLC_STATUS_URL -v --data '{"state": "success", "message": "'$MESSAGE'"}'
else
  MESSAGE=`cat /tmp/summary| sed -n 1p| tr '\n' ' '`
  SUMMARY=`cat /tmp/summary| sed ':a;N;$!ba;s/\n/\\n/g'`
  echo '{"state": "error", "message": "'$MESSAGE'", "summary": "'$SUMMARY'" }'
  curl -X PUT -H "Content-Type: application/json" -H "Token: $GITLC_API_TOKEN" $GITLC_STATUS_URL -v --data '{"state": "error", "message": "'$MESSAGE'", "summary": "'$SUMMARY'" }'

fi
