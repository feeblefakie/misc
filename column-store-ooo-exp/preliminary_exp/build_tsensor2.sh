#!/bin/sh

datadir="./data"

#time ./build_column $datadir/sensor100.csv 37 $datadir/rle100/37.col.rle INT rle y &
#time ./build_column $datadir/sensor.csv 37 $datadir/rle/37.col.rle INT rle y &
time ./build_column $datadir/sensor100.csv 15 $datadir/rle100/15.col.rle INT rle y 1 &
#for i in `seq 9 36`
#do
#    time ./build_column $datadir/sensor100.csv $i $datadir/rle100/$i.col.rle INT rle > /tmp/rle-build-$i.log 2>&1 &
#    b=`expr $i + 1`
#    r=`expr $b % 100`
#    echo $r
#    if [ $r -eq 0 ]; then
#        wait
#    fi
#done
wait
