#!/bin/sh

USER=`whoami`
if [ $USER != root ]; then
    echo "Must be root to run this script."
    exit
fi

ulimit -n 655360
export LD_LIBRARY_PATH=/usr/local/BerkeleyDB.5.2/lib
time=/usr/bin/time


#echo "Q2 Column-OoODE"
# Q2
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "/toyotaqooo samples/sample100.1.col samples/sample1.2.col 999 999 LE LE 3 0 2 3 1400 true"
#$time ./toyotaqooo samples/sample100.1.col samples/sample1.2.col 999 999 LE LE 3 0 2 3 1400 true

#echo "Q3 Column-OoODE"
# Q3
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "/toyotaqooo samples/sample100.1.col samples/sample1.2.col 9999 99999 LE LE 3 0 2 3 1400 true"
#$time ./toyotaqooo samples/sample100.1.col samples/sample1.2.col 9999 99999 LE LE 3 0 2 3 1400 true

#echo "Q4 Column-OoODE"
# Q4
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "/toyotaqooo samples/sample1.1.col samples/sample1.2.col 0 0 LE GE 2 0 3 3 1400 true"
#$time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col 0 0 LE GE 2 0 3 3 1400 true

# Q4
/home/hiroyuki/svn/scripts/clean-cache.sh
sleep 10
echo "/toyotaqooo samples/sample1.1.col samples/sample1.2.col 999 0 LE LE 32 0 33 33 1400 true"
time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col 999 0 LE LE 32 0 33 33 1000 true

#echo "Q5 Column-OoODE"
# Q5
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "/toyotaqooo samples/sample1.1.col samples/sample1.2.col 999 0 LE GE 2 0 3 3 1400 true"
#$time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col 999 0 LE GE 2 0 3 3 1400 true

#echo "Q2 Column (Scan)"
# Q2
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "./toyotaq samples/sample100.1.col samples/sample1.2.col 999 999 LE LE 3 0 2 3"
#$time ./toyotaq samples/sample100.1.col samples/sample1.2.col 999 999 LE LE 3 0 2 3 

#echo "Q3 Column (Scan)"
# Q3
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "./toyotaq samples/sample100.1.col samples/sample1.2.col 9999 99999 LE LE 3 0 2 3"
#$time ./toyotaq samples/sample100.1.col samples/sample1.2.col 9999 99999 LE LE 3 0 2 3 

#echo "Q4 Column (Scan)"
# Q4
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "./toyotaq samples/sample1.1.col samples/sample1.2.col 999 0 LE LE 2 0 3 3"
#$time ./toyotaq samples/sample1.1.col samples/sample1.2.col 999 0 LE LE 2 0 3 3 

#echo "Q5 Column (Scan)"
# Q5
#/home/hiroyuki/svn/scripts/clean-cache.sh
#sleep 10
#echo "./toyotaq samples/sample1.1.col samples/sample1.2.col 9999 999 LE LE 2 0 3 3"
#$time ./toyotaq samples/sample1.1.col samples/sample1.2.col 9999 999 LE LE 2 0 3 3 
