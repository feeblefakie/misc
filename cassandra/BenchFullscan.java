import java.io.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

import com.datastax.driver.core.*;
import com.datastax.driver.core.querybuilder.*;
import com.datastax.driver.core.policies.*;

public class BenchFullscan {

    private static Cluster cluster;
    private static Session session;
    private static ResultSet results;
    private static String endpoint = null;
    private static String keyspace = null;
    private static String table = null;

    public static void main(String[] args) {

        for (int i = 0; i < args.length; ++i) {
            if ("-endpoint".equals(args[i])) {
                endpoint = args[++i];
            } else if ("-keyspace".equals(args[i])) {
                keyspace = args[++i];
            } else if ("-table".equals(args[i])) {
                table = args[++i];
            } else {
                System.err.println("BenchInsert -endpoint hostname -keyspace keyspace -table table");
            }
        }
        if (endpoint == null || keyspace == null || table == null) {
            System.err.println("BenchInsert -endpoint hostname -keyspace keyspace -table table");
            System.exit(1);
        }

        String[] endpoints = endpoint.split(":");

        try {
            cluster = Cluster
                .builder()
                .addContactPoints(endpoints)
                .withRetryPolicy(DefaultRetryPolicy.INSTANCE)
                .withLoadBalancingPolicy(new TokenAwarePolicy(new DCAwareRoundRobinPolicy()))
                .build();
            session = cluster.connect(keyspace);

            int numRecords = 0;
            long start = System.currentTimeMillis();
            Statement select = QueryBuilder.select().all().from(keyspace, table);
            select.setFetchSize(100000);
            results = session.execute(select);
            for (Row row : results) {
                System.out.format("%d,%s,%s,%d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
                ++numRecords;
            }
            long end = System.currentTimeMillis();

            long interval = (end - start) / 1000;
            System.err.println("time taken (s) : " + interval);
            System.err.println("throughput (records/s) : " + numRecords/interval);

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
