#!/bin/sh

preds="19920103 19930101 19940101 19950101 19960101 19970101 19980101 19981231"
progs="em_pipe em_para lm_pipe lm_para"
#progs="em_para"

echo ""
echo "$prog (RLE,RLE)"
for prog in $progs
do
    for pred in $preds
    do
        sudo ~/svn/scripts/clean-cache.sh
        sleep 10
        echo $pred
        time ./$prog column/shipdate.col.rle column/linenumber.col.rle $pred NULL LE LE
        echo ""
    done 
done

echo ""
echo "$prog (RLE,NOCOMP)"
for prog in $progs
do
    for pred in $preds
    do
        sudo ~/svn/scripts/clean-cache.sh
        sleep 10
        echo $pred
        time ./$prog column/shipdate.col.rle column/linenumber.col $pred NULL LE LE
        echo ""
    done 
done
