#!/bin/sh

preds="NULL 0 9 99 999 9999 99999 999999"

for pred in $preds
do
    echo "$pred"
    sudo ~/svn/scripts/clean-cache.sh
    cat samples/sample1.10*.col > /dev/null
    sleep 10
    time ./em_pipe samples/sample1.1.col samples/sample1.2.col $pred NULL LE LE
done
