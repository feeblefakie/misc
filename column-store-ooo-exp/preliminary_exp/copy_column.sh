#!/bin/sh -x

fromdir=column
todir=samples

for i in `seq 1 300`
do
    cp $fromdir/sample1.col $todir/sample1.$i.col
    cp $fromdir/sample10.col $todir/sample10.$i.col
    cp $fromdir/sample10.col.posidx $todir/sample10.$i.col.posidx
    cp $fromdir/sample100.col $todir/sample100.$i.col
    cp $fromdir/sample100.col.posidx $todir/sample100.$i.col.posidx
    cp $fromdir/sample1000.col $todir/sample1000.$i.col
    cp $fromdir/sample1000.col.posidx $todir/sample1000.$i.col.posidx
done
