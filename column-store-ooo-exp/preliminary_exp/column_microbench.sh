#!/bin/sh

preds="NULL 0 9 99 999 9999 99999 999999"
: <<'#__COMMENT_OUT__'
for pred in $preds
do
    sudo ~/svn/scripts/clean-cache.sh
    sleep 10
    mpstat -P ALL 1 > /tmp/column_scan_bench_$pred.mpstat &
    vmstat 1 > /tmp/column_scan_bench_$pred.vmstat &
    echo "pred: $pred" >> /tmp/column_scan_bench_$pred.out
    ./column_scan_bench samples/sample1.1.col $pred LE >> /tmp/column_scan_bench_$pred.out 2>&1
    ps auxww | grep vmstat | awk '{print $2}' | xargs kill -9 
    ps auxww | grep mpstat | awk '{print $2}' | xargs kill -9 
    sync; sync; sync;
done
#__COMMENT_OUT__

prefix=".1mbio"
preds="0 9"
strides="1 10 100 1000 10000 100000 1000000"
#strides="2100 2200 2300 2400"
for stride in $strides
do
    for pred in $preds
    do
        sudo ~/svn/scripts/clean-cache.sh
        cat samples/sample1.10*.col > /dev/null
        sleep 10
        mpstat -P ALL 1 > /tmp/column_lookup_bench_${pred}_$stride.mpstat$prefix &
        vmstat 1 > /tmp/column_lookup_bench_${pred}_$stride.vmstat$prefix &
        echo "stride: $stride" >> /tmp/column_lookup_bench_${pred}_$stride.out$prefix
        ./column_lookup_bench samples/sample1.1.col $pred LE $stride >> /tmp/column_lookup_bench_${pred}_$stride.out$prefix 2>&1
        ps auxww | grep vmstat | awk '{print $2}' | xargs kill -9 
        ps auxww | grep mpstat | awk '{print $2}' | xargs kill -9 
        sync; sync; sync;
    done
done
