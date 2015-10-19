#!/bin/sh

#endpoints="aoba01:aoba02:aoba03:aoba04:aoba05:aoba06:aoba07:aoba08:aoba09:aoba10:aoba11:aoba12:aoba13:aoba14:aoba15:aoba16:aoba17"
endpoints="aoba01:aoba02:aoba03:aoba04"
keyspace="mykeyspace"
table="users5"

java -classpath "cassandra-java-driver-2.1.8/*:cassandra-java-driver-2.1.8/lib/*:/usr/share/cassandra/lib/*:." BenchFullscan -endpoint $endpoints -keyspace $keyspace -table $table
