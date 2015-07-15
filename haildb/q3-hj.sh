#!/bin/sh

orderdates="8036 8046 8056 8066 8076 8086 8096 8100 8200 8300 8400 8500"
for orderdate in $orderdates
do
    ./q3-1_1-hjsim $orderdate 1
done
