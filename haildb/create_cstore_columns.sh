#!/bin/sh

# lineitem (L1)
#./index_rle "l1_shipdate_rle" < lineitem-cstore.shipdate_rle
#awk '{print $1}' lineitem-cstore.tbl.shipdate_suppkey-sorted | ./index_uncompressed "l1_orderkey_uncompressed" 
#awk '{print $6}' lineitem-cstore.tbl.shipdate_suppkey-sorted | ./index_uncompressed "l1_extendedprice_uncompressed" 

# orders (L3)
./index_rle "l3_orderdate_rle" < orders-cstore.orderdate_rle
#awk '{print $2}' orders-cstore.tbl.orderdate-sorted | ./index_uncompressed "l3_custkey_uncompressed" 
#awk '{print $3}' orders-cstore.tbl.orderdate-sorted | ./index_uncompressed "l3_orderkey_uncompressed" 

# customer (L5)
#awk '{print $1}' customer-cstore.tbl | ./index_key_rle "l5_customer_key_rle"
#awk '{print $2}' customer-cstore.tbl | ./index_uncompressed "l5_nationkey_uncompressed" 

# L6 (l_orderkey, ..., l_extendedprice, ..., l_shipdate | l_orderkey)
#./index_rle "l6_orderkey_rle" < lineitem-cstore.orderkey_rle
awk '{print $8}' lineitem-cstore.tbl.orderkey-sorted | ./index_uncompressed "l6_shipdate_uncompressed" 
awk '{print $6}' lineitem-cstore.tbl.orderkey-sorted | ./index_uncompressed "l6_extendedprice_uncompressed" 

# JI (L3 -> L1)
#./index_ji "ji_l3-l1" <  ji_l3-l1
