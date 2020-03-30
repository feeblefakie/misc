#!/bin/sh -x

if [ $# -ne 2 ]; then
    echo "$0 topic subscription"
    exit 1;
fi

topic=$1
sub=$2
rm -f /tmp/$sub.*

#build/install/pulsar/bin/my-fo-consumer $topic $sub 0 7 > /tmp/$sub-1 &
#build/install/pulsar/bin/my-fo-consumer $topic $sub 8 15 > /tmp/$sub-2 &
#build/install/pulsar/bin/my-fo-consumer $topic $sub 16 23 > /tmp/$sub-3 &
#build/install/pulsar/bin/my-fo-consumer $topic $sub 24 31 > /tmp/$sub-4 &
build/install/pulsar/bin/my-fo-consumer $topic $sub 0 255 > /tmp/$sub-1 &
build/install/pulsar/bin/my-fo-consumer $topic $sub 256 511 > /tmp/$sub-2 &
build/install/pulsar/bin/my-fo-consumer $topic $sub 512 767 > /tmp/$sub-3 &
build/install/pulsar/bin/my-fo-consumer $topic $sub 768 1023 > /tmp/$sub-4 &

wait

