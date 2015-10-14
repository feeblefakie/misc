#!/bin/sh

endpoints="aoba01:aoba02:aoba03:aoba04:aoba05:aoba06:aoba07:aoba08:aoba09:aoba10:aoba11:aoba12:aoba13:aoba14:aoba15:aoba16:aoba17"
keyspace="mykeyspace"
table="users2"
filename="selectdata.dat.10000"
concurrency=5

java -classpath "cassandra-java-driver-2.1.8/*:cassandra-java-driver-2.1.8/lib/*:/usr/share/cassandra/lib/*:." BenchLookup -endpoint $endpoints -keyspace $keyspace -table $table -filename $filename -concurrency $concurrency
