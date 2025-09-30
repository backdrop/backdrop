#!/bin/bash
# Get the list of relevant changed files compared to 1.x and run phpcs on them.

TMPFILE=$(mktemp /tmp/filelist-XXXXXXXX)
git diff origin/1.x --name-only --diff-filter=AM | grep -E '(php|inc|module|install|profile|engine|test)$' | grep -v '^.github' > $TMPFILE

phpcs -nq --basepath=. --standard=../phpcs/Backdrop --report=.github/misc/Github.php --file-list=$TMPFILE
FINALEXIT=$?

rm $TMPFILE
# Otherwise we get the exit code from rm?
echo $FINALEXIT
exit $FINALEXIT
