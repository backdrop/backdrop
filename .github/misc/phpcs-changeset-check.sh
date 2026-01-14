#!/bin/bash
# Get the list of relevant changed files compared to base and run phpcs on them.
# Filter the result to only annotate code changes compared to base branch.

# If a param exists, use it as base branch, use fallback otherwise.
if [ -n "$1" ]
then
  BASE="$1"
else
  BASE='origin/1.x'
fi
FILELIST=/tmp/phpcs-filelist
git diff $BASE..HEAD --name-only --diff-filter=AM | grep -E '\.(php|inc|module|install|profile|engine|test)$' > $FILELIST

# Make sure this clearly fails if something went wrong with git diff.
# But ignore that grep might not have found any changed php files.
if [ $? -gt 1 ]
then
  rm $FILELIST
  exit 1
fi

if [ ! -s $FILELIST ]
then
  echo 'No files to check'
  rm $FILELIST
  exit 0
fi

RESULT_UNFILTERED=/tmp/phpcs-results
phpcs -nq --basepath=. --standard=../phpcs/Backdrop --report=../phpcs/Backdrop/Reports/Github.php --file-list=$FILELIST > $RESULT_UNFILTERED

# Make sure this clearly fails with process or requirements error in phpcs.
# But ignore coding standard violations here.
if [ $? -gt 3 ]
then
  rm $FILELIST $RESULT_UNFILTERED
  exit 1
fi

if [ ! -s $RESULT_UNFILTERED ]
then
  echo 'No code style problems found in changed files'
  rm $FILELIST $RESULT_UNFILTERED
  exit 0
fi

# More tmp files.
REVLIST=/tmp/git-revlist
git rev-list $BASE..HEAD > $REVLIST

FILTERED_RESULT=/tmp/phpcs-filtered-result
echo -n '' > $FILTERED_RESULT

# Override file list with additional status column.
git diff $BASE..HEAD --name-status --diff-filter=AM | grep -E '\.(php|inc|module|install|profile|engine|test)$' > $FILELIST

cat $FILELIST | while read LINE
do
  FILENAME=$(echo $LINE | cut -d ' ' -f 2)
  STATUS=$(echo $LINE | cut -d ' ' -f 1)
  if [ $STATUS == 'M' ]
  then
    # Get line numbers of changes.
    CHANGED_LINES=$(git blame --no-progress -fls $FILENAME | grep -f $REVLIST |
      awk '{print $3}' | sed -e 's/)$//' | tr '\n' '|' | sed -e 's/|$//')
    grep -E "file=$FILENAME,line=($CHANGED_LINES)," $RESULT_UNFILTERED >> $FILTERED_RESULT
  else
    # Newly added files (A) get checked as a whole.
    grep "file=$FILENAME," $RESULT_UNFILTERED >> $FILTERED_RESULT
  fi
done

rm $FILELIST $RESULT_UNFILTERED $REVLIST

if [ -s $FILTERED_RESULT ]
then
  # Remaining code style issues after filtering.
  cat $FILTERED_RESULT
  rm $FILTERED_RESULT
  exit 1
fi

rm $FILTERED_RESULT
exit 0
