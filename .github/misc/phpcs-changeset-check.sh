#!/bin/bash
# Get the list of relevant changed files compared to base and run phpcs on them.
# Filter the result to only annotate code changes compared to base branch.

BASE='origin/1.x'
FILELIST=/tmp/phpcs-filelist
git diff $BASE..HEAD --name-only --diff-filter=AM | grep -E '\.(php|inc|module|install|profile|engine|test)$' > $FILELIST

if [ ! -s $FILELIST ]
then
  echo 'No files to check'
  rm $FILELIST
  exit 0
fi

RESULT_UNFILTERED=/tmp/phpcs-results
phpcs -nq --basepath=. --standard=../phpcs/Backdrop --report=.github/misc/Github.php --file-list=$FILELIST > $RESULT_UNFILTERED

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

cat $FILELIST | while read FILENAME
do
  # Get line numbers of changes.
  CHANGED_LINES=$(git blame --no-progress -fls $FILENAME | grep -f $REVLIST | awk '{print $3}' | sed -e 's/)$//' | tr '\n' '|' | sed -e 's/|$//')
  # @todo do non-faulty files/lines need some if clause?
  if [ -z $CHANGED_LINES ]
  then
    continue
  fi
  # @todo for newly added files this regex could get simplified (type A vs. type M)
  grep -E "file=$FILENAME,line=($CHANGED_LINES)," $RESULT_UNFILTERED >> $FILTERED_RESULT
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
