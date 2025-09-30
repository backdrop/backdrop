#!/bin/bash
# Get the list of relevant changed files compared to 1.x and run phpcs on them.

FILES=$(git diff 1.x --name-only --diff-filter=AM | grep -E '(php|inc|module|install|profile|engine|test)$')

echo $FILES | while read FILENAME
do
  phpcs -nq --basepath=../.. --standard=../../../phpcs/Backdrop --report=json $FILENAME
done

exit 0
