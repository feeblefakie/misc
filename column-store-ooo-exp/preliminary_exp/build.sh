#!/bin/sh

datadir="./column"

#time ./build_column /data/hadoop/tpch/sf100/lineitem.proj1.sorted 0 $datadir/shipdate.col.rle CHAR8 rle > build.log 2>&1
#time ./build_column /data/hadoop/tpch/sf100/lineitem.proj1.sorted 3 $datadir/returnflag.col.rle CHAR rle > build.log 2>&1
#time ./build_column /data/hadoop/tpch/sf100/lineitem.proj1.sorted 1 $datadir/linenumber.col.rle TINYINT rle > build.log 2>&1
#time ./build_column /data/hadoop/tpch/sf100/lineitem.proj1.sorted 1 $datadir/linenumber.col TINYINT nocomp > build.log 2>&1

#time ./build_column ./column/sample1 0 ./column/sample1.col BIGINT no y &
#time ./build_column ./column/sample10 0 ./column/sample10.col BIGINT rle y &
#time ./build_column ./column/sample100 0 ./column/sample100.col BIGINT rle y &
#time ./build_column ./column/sample1000 0 ./column/sample1000.col BIGINT rle y &

for j in `seq 2 110` 
do
    for i in `seq 0 15` 
    do
    #time ./build_column ./column/new_sample1.$i 0 ./column/new_sample1.1.col.$i BIGINT no n &
        cp ./newsamples/new_sample1.1.col.$i ./newsamples/new_sample1.$j.col.$i
    done
    cp ./newsamples/new_sample1.1.col ./newsamples/new_sample1.$j.col
done
wait
