#!/bin/sh

endpoints="aoba01:aoba02:aoba03:aoba04:aoba05:aoba06:aoba07:aoba08:aoba09:aoba10:aoba11:aoba12:aoba13:aoba14:aoba15:aoba16:aoba17"
#endpoints="aoba01:aoba02:aoba03:aoba04"
keyspace="mykeyspace"
table="users5"
#filename="selectdata.dat.1000000.4nodes"
filename="selectdata.dat.1000000"
concurrency=100

gradle run -Pmain=benchmark.BenchLookup -Pargs="-endpoint $endpoints -keyspace $keyspace -table $table -filename $filename -concurrency $concurrency"
