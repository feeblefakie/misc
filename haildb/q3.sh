#!/bin/sh

orderdates="8046 8056 8066 8076 8086 8096"
for orderdate in $orderdates
do
    ./q3-1_1 $orderdate 0
    ./q3-1_1 $orderdate 1
done
