#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <folder-with-results>"
  exit
fi

for x in `ls "$1"`; do
  if [ ${x: -4} == ".tgz" ]; then
    OPP_RUN_NUMBER=`tar --file "$1/$x" --wildcards --no-anchored --extract "lust/RUN*" --strip-components=8 -O |  grep 'run #' | cut -d'#' -f2 | cut -d'.' -f1`
    tar --file "$1/$x" --wildcards --no-anchored --extract "lust/results/*" --strip-components=8 2>/dev/null
    if [ "$?" -eq 0 ]; then
      #success: in ./results/, new files were added
      echo "$OPP_RUN_NUMBER" >> successes.txt
    else
      #failure
      echo "$OPP_RUN_NUMBER" >> failures.txt
    fi
  fi
done
