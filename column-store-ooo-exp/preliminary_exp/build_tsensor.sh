#!/bin/sh

datadir="./data"

time ./build_column $datadir/sensor100.csv 37 $datadir/nocomp100/37.col INT no y &
#for i in `seq 9 36`
#do
#    time ./build_column $datadir/sensor100.csv $i $datadir/nocomp100/$i.col INT no > /tmp/nocomp-build-$i.log 2>&1 &
#    b=`expr $i + 1`
#    r=`expr $b % 100`
#    echo $r
#    if [ $r -eq 0 ]; then
#        wait
#    fi
#done
wait
