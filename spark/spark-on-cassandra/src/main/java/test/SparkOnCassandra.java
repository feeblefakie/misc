package test;

import org.apache.spark.sql.AnalysisException;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;

public class SparkOnCassandra {
  public static void main(String[] args) throws AnalysisException {
    SparkSession session = SparkSession.builder().getOrCreate();
    session
        .read()
        .format("org.apache.spark.sql.cassandra")
        .option("keyspace", "scalar")
        .option("table", "asset")
        .load()
        .createTempView("asset");

    Dataset<Row> rows = session.sql("SELECT * FROM asset WHERE age = 1");
    rows.show();

    session.close();
  }
}
