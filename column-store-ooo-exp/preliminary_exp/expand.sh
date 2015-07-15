#!/bin/sh

dir=column2

#for i in `seq 1 10`
#do
#    cat $dir/sample1 >> $dir/bsample1
#done
#./build_column $dir/bsample1 0 $dir/bsample1.col BIGINT no y > $dir/bsample1.col.idxlist

for i in `seq 1 10`
do
    cat $dir/sample10 >> $dir/bsample10
done
./build_column $dir/bsample10 0 $dir/bsample10.col BIGINT rle y &

for i in `seq 1 10`
do
    cat $dir/sample100 >> $dir/bsample100
done
./build_column $dir/bsample100 0 $dir/bsample100.col BIGINT rle y & 

for i in `seq 1 10`
do
    cat $dir/sample1000 >> $dir/bsample1000
done
./build_column $dir/bsample1000 0 $dir/bsample1000.col BIGINT rle y & 

wait
