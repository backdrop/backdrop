#!/bin/bash
# Get the list of relevant changed files compared to 1.x and run phpcs on them.

TMPFILE=$(mktemp /tmp/filelist-XXXXXXXX)
git diff origin/1.x --name-only --diff-filter=AM | grep -E '(php|inc|module|install|profile|engine|test)$' | grep -v '^.github' > $TMPFILE

if [ ! -s $TMPFILE ]
then
  echo 'No files to check'
  rm $TMPFILE
  exit 0
fi

phpcs -nq --basepath=. --standard=../phpcs/Backdrop --report=checkstyle --file-list=$TMPFILE | cs2pr
# Otherwise we get the exit code from rm.
FINALEXIT=$?

rm $TMPFILE

exit $FINALEXIT
