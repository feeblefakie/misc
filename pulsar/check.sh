#!/bin/bash

FILE1=/tmp/file1
FILE2=/tmp/file2
rm -f $FILE1 $FILE2

NUM_OK=0
NUM_NG=0

for i in {0..999}
do
    echo -n "checking $i ... "
    grep " $i " /tmp/sub1 | awk '{print $2 " " $3}' > $FILE1
    grep " $i " /tmp/sub2 | awk '{print $2 " " $3}' > $FILE2
    diff -u $FILE1 $FILE2
    if [ $? -eq 1 ]; then
        echo "[NG]"
        ((NUM_NG=NUM_NG+1))
        #exit 1
    else
        echo "[OK]"
        ((NUM_OK=NUM_OK+1))
    fi
    rm -f $FILE1 $FILE2
done

echo "OK: $NUM_OK"
echo "NG: $NUM_NG"
