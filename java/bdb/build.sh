#!/bin/sh

javac -cp /usr/lib/hadoop-0.20-mapreduce/lib/db.jar BatchUpdateTest.java
java -cp "/usr/lib/hadoop-0.20-mapreduce/lib/db.jar:." BatchUpdateTest
