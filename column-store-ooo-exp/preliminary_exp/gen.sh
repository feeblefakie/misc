#!/bin/sh

for i in `seq 1 1`
do
    time ./gen_rand_nums 1000000000 1000000 1000 > /export/data2/column/sample1000.col.$i &
done

for i in `seq 1 1`
do
    time ./gen_rand_nums 1000000000 1000000 100 > /export/data2/column/sample100.col.$i &
done

for i in `seq 1 1`
do
    time ./gen_rand_nums 1000000000 1000000 10 > /export/data2/column/sample10.col.$i &
done

for i in `seq 1 1`
do
    time ./gen_rand_nums 1000000000 1000000 1 > /export/data2/column/sample1.col.$i &
done
wait
