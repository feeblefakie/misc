#!/bin/sh

preds="1 10 100 1000"
#preds="1000000000"

echo -e "\n\nNLJ-EM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "NLJ-EM(OoO) : $pred" 1>&2
    ./join N EM on 1000 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
    #./join N EM on 1000 1 table3.def $pred LT
done

echo -e "\n\nNLJ-LM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "NLJ-LM(OoO) : $pred" 1>&2
    ./join N LM on 1000 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done

: <<'#__COMMENT_OUT__'

echo -e "\n\nHJ-EM" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-EM : $pred" 1>&2
    ./join H EM off 1 3 table1.def $pred LT table2.def NULL LT table3.def NULL LT
    #./join H EM off 1 1 table3.def $pred LT
done

echo -e "\n\nHJ-LM" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-LM : $pred" 1>&2
    #./join H LM off 1 3 table1.def $pred LT table2.def NULL LT table3.def NULL LT
    ./join H LM off 1 1 table3.def $pred LT
done

preds="100000000"
echo -e "\n\nHJ-EM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-EM(OoO) : $pred" 1>&2
    #./join H EM on 1000 3 table3.def $pred LT table2.def $pred2 LT table1.def NULL LT
    ./join H EM on 1000 1 table3.def $pred LT
done

preds="1 10 100 1000 10000 100000"

preds="1 10 100 1000"
echo -e "\n\nNLJ-EM" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "NLJ-EM : $pred" 1>&2
    #./join N EM off 1 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
    ./join N EM off 1 1 table3.def $pred LT
done
#__COMMENT_OUT__


: <<'#__COMMENT_OUT__'

echo -e "\n\nHJ-LM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-LM : $pred" 1>&2
    #./join H LM on 1000 3 table1.def $pred LT table2.def NULL LT table3.def NULL LT
    ./join H LM on 1000 1 table3.def $pred LT
done


#preds="1 10 100 1000 10000 100000 1000000 10000000"

echo -e "\n\nHJ-EM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-EM(OoO) : $pred" 1>&2
    ./join H EM on 1000 3 table3.def $pred LT table2.def $pred2 LT table1.def NULL LT
done
#__COMMENT_OUT__


: <<'#__COMMENT_OUT__'

echo -e "\n\nNLJ-LM" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "NLJ-LM : $pred" 1>&2
    ./join N LM off 1 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done
#__COMMENT_OUT__
: <<'#__COMMENT_OUT__'

preds="10000"



echo -e "\n\nHJ-LM" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-LM : $pred" 1>&2
    ./join H LM off 1 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done

echo -e "\n\nHJ-LM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-LM : $pred" 1>&2
    ./join H LM on 1000 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done

echo -e "\n\nHJ-EM" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-EM : $pred" 1>&2
    ./join H EM off 1 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done

echo -e "\n\nHJ-EM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "HJ-EM(OoO) : $pred" 1>&2
    ./join H EM on 1000 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done


echo -e "\n\nNLJ-EM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "NLJ-EM(OoO) : $pred" 1>&2
    ./join N EM on 1000 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done

echo -e "\n\nNLJ-LM(OoO)" 1>&2
for pred in $preds
do
    pred2=`expr $pred \/ 100`
    sudo ~/svn/scripts/clean-cache.sh
    sleep 5
    echo -e "NLJ-LM(OoO) : $pred" 1>&2
    ./join N LM on 1000 3 table1.def $pred LT table2.def $pred2 LT table3.def NULL LT
done
#__COMMENT_OUT__
