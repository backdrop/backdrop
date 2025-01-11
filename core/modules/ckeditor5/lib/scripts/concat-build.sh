#!/bin/bash
# Download and concatenate build.

declare -a PLUGINS=(
"@ckeditor/ckeditor5-alignment"
"@ckeditor/ckeditor5-autoformat"
"@ckeditor/ckeditor5-basic-styles"
"@ckeditor/ckeditor5-block-quote"
"@ckeditor/ckeditor5-code-block"
"@ckeditor/ckeditor5-editor-classic"
"@ckeditor/ckeditor5-essentials"
"@ckeditor/ckeditor5-heading"
"@ckeditor/ckeditor5-horizontal-line"
"@ckeditor/ckeditor5-html-support"
"@ckeditor/ckeditor5-image"
"@ckeditor/ckeditor5-indent"
"@ckeditor/ckeditor5-language"
"@ckeditor/ckeditor5-link"
"@ckeditor/ckeditor5-list"
"@ckeditor/ckeditor5-paste-from-office"
"@ckeditor/ckeditor5-remove-format"
"@ckeditor/ckeditor5-source-editing"
"@ckeditor/ckeditor5-special-characters"
"@ckeditor/ckeditor5-style"
"@ckeditor/ckeditor5-table"
"@ckeditor/ckeditor5-show-blocks"
)

# Download includes every official plugin.
npm install --save ckeditor5

cp node_modules/ckeditor5/build/ckeditor5-dll.js .

for ITEM in "${PLUGINS[@]}"
do
  cat node_modules/$ITEM/build/*.js >> ckeditor5-dll.js
done

# Make a fresh translation directory.
mkdir -p translations
rm -rf translations/*

# Exclude the unnecessary English po file.
EN_FILE=node_modules/@ckeditor/ckeditor5-core/lang/translations/en.po
if [ -f "$EN_FILE" ]; then
  rm $EN_FILE
fi

# Copy all available translations into concatenated files.
LANGCODE_LIST=($(ls node_modules/@ckeditor/ckeditor5-core/lang/translations/ | sed 's/.po//'))

for ITEM in "${PLUGINS[@]}"
do
  for LANGCODE in "${LANGCODE_LIST[@]}"
  do
    FILE=node_modules/$ITEM/build/translations/$LANGCODE.js
    if [ -f $FILE ]
      then
      cat $FILE >> translations/$LANGCODE.js
    fi
  done
done
