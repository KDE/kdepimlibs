#!/bin/bash
#
# Create a diff to last stable branch of all API/ABI-relevant files for review
#
# usage: make_review_diff.sh
# will create ./<lib>-api-changes.diff for all subdirs

branch_url=svn://anonsvn.kde.org/home/kde/branches/KDE/4.4/kdepimlibs/
trunk_url=svn://anonsvn.kde.org/home/kde/trunk/KDE/kdepimlibs/

for lib in `find -mindepth 1 -maxdepth 1 -type d | grep -v "/\." | grep -v includes | grep -v doc`; do
  echo "Generating diff for $lib..."
  rm -f ${lib}-api-changes.diff
  for i in `find $lib -name "*.h" -o -name "*.xml" | grep -v "tests" | grep -v "_p.h"`; do
    echo "Diffing $i..."
    svn diff ${branch_url}$i ${trunk_url}$i 2> /dev/null >> ${lib}-api-changes.diff
    if [ $? == 1 ]; then
      echo "File $i was added in trunk."
      diff -u /dev/null $i >> ${lib}-api-changes.diff
    fi
  done
  if [ -z "`cat ${lib}-api-changes.diff`" ]; then
    rm -f ${lib}-api-changes.diff
    echo "No changes found in $lib."
  else
    echo "Wrote changes in $lib to ${lib}-api-changes.diff"
  fi
done

