#!/bin/bash

EXTENSIONS="avi mkv wmv mpg mpeg mp4 mov m2v"

SCAN_DIR=$1

OUT_FILE=/tmp/results

for ext in $EXTENSIONS; do
	CMD="find $SCAN_DIR -follow -name '*.$ext' -printf ',\"%p\",\n'"
	eval $CMD >> $OUT_FILE
done

sort $OUT_FILE -o $OUT_FILE
uniq $OUT_FILE > $OUT_FILE~
mv $OUT_FILE{~,}
