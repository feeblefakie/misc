/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.spark.examples.sql;

import java.io.Serializable;
import java.util.Arrays;
import java.util.List;

import org.apache.spark.SparkConf;
import org.apache.spark.SparkContext;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function;

import org.apache.spark.sql.DataFrame;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SQLContext;
import org.apache.spark.sql.DataFrame;


import org.apache.spark.sql.cassandra.CassandraSQLContext;

public class JavaSparkSQLCassandra {

  public static void main(String[] args) throws Exception {
    SparkConf sparkConf = new SparkConf().setAppName("JavaSparkSQL").set("spark.cassandra.connection.host", "192.168.110.101").set("spark.executor.extraClassPath", "/home/hiroyuki/packages/spark-cassandra-connector-1.4.0-s_2.10/spark-cassandra-connector-java/target/scala-2.10/spark-cassandra-connector-java-assembly-1.4.0-SNAPSHOT.jar");

    SparkContext ctx = new SparkContext("spark://192.168.110.101:7077", "app", sparkConf);
    //JavaSparkContext jctx = new JavaSparkContext(sparkConf);
    //SQLContext sqlContext = new SQLContext(jctx);
    CassandraSQLContext cassandraContext = new CassandraSQLContext(ctx);

    DataFrame users = cassandraContext.sql("select * from mykeyspace.users limit 3");
    users.show();

    System.out.println(users.schema().fieldNames());

  }
}
