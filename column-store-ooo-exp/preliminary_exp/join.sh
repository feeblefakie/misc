#!/bin/sh

ulimit -n 999999 

pats="q3_nocomp q3_rle q4_nocomp q4_rle q1_nocomp q1_rle q2_nocomp q2_rle"
preds="50 128 255"

export LD_LIBRARY_PATH=/usr/local/BerkeleyDB.5.2/lib

for pat in $pats
do
: <<'#__COMMENT_OUT__'
    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 5
    echo "./join N EM off 1 1 $pat.def 144 EQ"
    time ./join N EM off 1 1 $pat.def 144 EQ

    /home/hiroyuki/svn/scripts/clean-cache.sh
    sleep 5
    echo "./join N EM on 1400 1 $pat.def 144 EQ"
    time ./join N EM on 1400 1 $pat.def 144 EQ
#__COMMENT_OUT__

    for pred in $preds
    do
        /home/hiroyuki/svn/scripts/clean-cache.sh
        sleep 5
        echo "./join H EM off 1 1 $pat.def $pred EQ"
        time ./join H EM off 1 1 $pat.def $pred EQ

        /home/hiroyuki/svn/scripts/clean-cache.sh
        sleep 5
        echo "./join H EM on 1400 1 $pat.def $pred EQ"
        time ./join H EM on 1400 1 $pat.def $pred EQ
    done
done
