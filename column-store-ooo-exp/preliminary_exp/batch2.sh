#!/bin/sh

USER=`whoami`
if [ $USER != root ]; then
    echo "Must be root to run this script."
    exit
fi

ulimit -n 655360
export LD_LIBRARY_PATH=/usr/local/BerkeleyDB.5.2/lib
time=/usr/bin/time

#seconds="9999 999"
#seconds="0 9 99 999"
seconds="0 9"

for second in $seconds
do

    echo "first sample1000"

: <<'#__COMMENT_OUT__'
    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "/toyotaqooo samples/sample1000.1.col samples/sample1.2.col 999 $second LE LE 33 0 33 32 1400 true"
    $time ./toyotaqooo samples/sample1000.1.col samples/sample1.2.col 999 $second LE LE 33 0 33 32 1400 true

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "./toyotaqooo samples/sample1000.1.col samples/sample1.2.col 999 $second LE LE 33 0 33 32 1 true"
    $time ./toyotaqooo samples/sample1000.1.col samples/sample1.2.col 999 $second LE LE 33 0 33 32 1 true

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "./toyotaq samples/sample1000.1.col samples/sample1.2.col 999 $second LE LE 33 0 33 32"
    $time ./toyotaq samples/sample1000.1.col samples/sample1.2.col 999 $second LE LE 33 0 33 32 
#__COMMENT_OUT__

    echo "first sample1"

    #/home/hiroyuki/svn/scripts/clean-cache.sh
    #sleep 10
    #echo "/toyotaqooo samples/sample1.1.col samples/sample1.2.col $second 0 LE GE 32 0 33 33 1400 true"
    #$time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col $second 0 LE GE 32 0 33 33 1400 true

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "./toyotaqooo samples/sample1.1.col samples/sample1.2.col $second 0 LE GE 32 0 33 33 1 true"
    $time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col $second 0 LE GE 32 0 33 33 1 true
: <<'#__COMMENT_OUT__'

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "./toyotaq samples/sample1.1.col samples/sample1.2.col $second 0 LE GE 32 0 33 33"
    $time ./toyotaq samples/sample1.1.col samples/sample1.2.col $second 0 LE GE 32 0 33 33 

#__COMMENT_OUT__

: <<'#__COMMENT_OUT__'
    echo "first sample100"

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "/toyotaqooo samples/sample100.1.col samples/sample1.2.col 999 $second LE LE 33 0 32 33 1400 true"
    $time ./toyotaqooo samples/sample100.1.col samples/sample1.2.col 999 $second LE LE 33 0 32 33 1400 true

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "./toyotaqooo samples/sample100.1.col samples/sample1.2.col 999 $second LE LE 33 0 32 33 1 true"
    $time ./toyotaqooo samples/sample100.1.col samples/sample1.2.col 999 $second LE LE 33 0 32 33 1 true

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    echo "./toyotaq samples/sample100.1.col samples/sample1.2.col 999 $second LE LE 33 0 32 33"
    $time ./toyotaq samples/sample100.1.col samples/sample1.2.col 999 $second LE LE 33 0 32 33 

#__COMMENT_OUT__
done
