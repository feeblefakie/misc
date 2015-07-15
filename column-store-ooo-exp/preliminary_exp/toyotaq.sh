#!/bin/sh

ulimit -n 655360
export LD_LIBRARY_PATH=/usr/local/BerkeleyDB.5.2/lib

#preds="0 9 99 999"
#for pred in $preds
#do
#    sudo /home/hiroyuki/svn/scripts/clean-cache.sh
#    cat samples/sample1.10[1-9].col > /dev/null
#    sleep 10
#    echo "toyotaqooo1mbio #_of_column:100 $pred"
#    time ./toyotaqooo1mbio samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE 98 0 0 0 1000 false
#done
    
preds="9"
for pred in $preds
do
    sudo /home/hiroyuki/svn/scripts/clean-cache.sh
    cat samples/sample1.10[1-9].col > /dev/null
    sleep 10
    echo "toyotaqooo #_of_column:100 $pred index-based(in-order)"
    time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE 98 0 0 0 1 true
done

preds="999"
for pred in $preds
do
    sudo /home/hiroyuki/svn/scripts/clean-cache.sh
    cat samples/sample1.10[1-9].col > /dev/null
    sleep 10
    echo "toyotaqooo #_of_column:100 $pred index-based(OoO)"
    time ./toyotaqooo samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE 98 0 0 0 1000 true
done

#preds="0 9 99 999 9999 99999 999999"
: <<'#__COMMENT_OUT__'
for pred in $preds
do
    sudo /home/hiroyuki/svn/scripts/clean-cache.sh
    cat samples/sample1.10[1-9].col > /dev/null
    sleep 10
    echo "toyotaq1mbio #_of_column:100 $pred"
    time ./toyotaq1mbio samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE 98 0 0 0 
done

preds="0 9 99 999"
for pred in $preds
do
    sudo /home/hiroyuki/svn/scripts/clean-cache.sh
    cat samples/sample1.10[1-9].col > /dev/null
    sleep 10
    echo "toyotaqooo1mbio #_of_column:100 $pred"
    time ./toyotaqooo1mbio samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE 98 0 0 0 1000 false
done

preds="0 9 99 999 9999 99999 999999"
for pred in $preds
do
    sudo /home/hiroyuki/svn/scripts/clean-cache.sh
    cat samples/sample1.10[1-9].col > /dev/null
    sleep 10
    echo "toyotaq #_of_column:100 $pred"
    time ./toyotaq samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE 98 0 0 0 
done
##__COMMENT_OUT__
