#!/bin/sh

files="file1 file2 file3 file4"
filelist="filelist"
rm -f $filelist
for file in $files
do
    rm -f $file
    dd if=/dev/zero of=$file bs=4096 count=10000 &
    echo $file >> $filelist
done
