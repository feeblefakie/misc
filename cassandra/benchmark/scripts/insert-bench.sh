#!/bin/sh

#endpoints="aoba01:aoba02:aoba03:aoba04:aoba05:aoba06:aoba07:aoba08:aoba09:aoba10:aoba11:aoba12:aoba13:aoba14:aoba15:aoba16:aoba17"
endpoints="aoba01:aoba02:aoba03:aoba04"
keyspace="mykeyspace"
table="users5"
filename="/tmp/rawdata_big.csv.4nodes"

gradle run -Pmain=benchmark.BenchBatchInsert -Pargs="-endpoint $endpoints -keyspace $keyspace -table $table -filename $filename"
