#!/bin/sh

USER=`whoami`
if [ $USER != root ]; then
    echo "Must be root to run this script."
    exit
fi

ulimit -n 655360
export LD_LIBRARY_PATH=/usr/local/BerkeleyDB.5.2/lib

time="/usr/bin/time"
results_dir="results/sample10_first"
mkdir -p $results_dir

# lookups

threads="1 1400"
ncolumns="1 8"

#: <<'#__COMMENT_OUT__'
for t in $threads
do
    for n in $ncolumns
    do
        ncol=`expr $n + 2`
        /home/hiroyuki/svn/scripts/clean-cache.sh
        sleep 10
        #$time ./toyotaqooo samples/sample10.1.col samples/sample1.2.col 999 999 LE LE $n 0 0 0 $t true > $results_dir/lookups_${ncol}c_10-3s_10-3s_${t}t.out 2>&1

        /home/hiroyuki/svn/scripts/clean-cache.sh
        sleep 10
        #$time ./toyotaqooo samples/sample10.1.col samples/sample1.2.col 999 9999 LE LE $n 0 0 0 $t true > $results_dir/lookups_${ncol}c_10-3s_10-2s_${t}t.out 2>&1
    done
done
##__COMMENT_OUT__

# scans

for n in $ncolumns
do
    ncol=`expr $n + 2`
    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    $time ./toyotaq samples/sample10.1.col samples/sample1.2.col 999 999 LE LE $n 0 0 0 > $results_dir/scan_${ncol}c_10-3s_10-3s_1t.out 2>&1

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 10
    $time ./toyotaq samples/sample10.1.col samples/sample1.2.col 999 9999 LE LE $n 0 0 0 > $results_dir/scan_${ncol}c_10-3s_10-2s_1t.out 2>&1

    #/home/hiroyuki/svn/scripts/clean-cache.sh
    #sleep 10
    #$time ./toyotaq samples/sample10.1.col samples/sample1.2.col 100 1 LE GE $n 0 0 0 > $results_dir/scan_${ncol}c_10-4s_${t}t.out 2>&1
done
