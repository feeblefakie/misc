import java.io.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
 import java.util.concurrent.TimeUnit;

import com.datastax.driver.core.*;
import com.datastax.driver.core.querybuilder.*;
import com.datastax.driver.core.policies.*;

public class BenchLookup {

    private static Cluster cluster;
    private static Session session;
    private static ResultSet results;
    private static BlockingQueue<String> queue;
    private static String endpoint = null;
    private static String keyspace = null;
    private static String table = null;
    private static String filename = null;
    private static int concurrency = 0;

    public static void main(String[] args) {

        for (int i = 0; i < args.length; ++i) {
            if ("-endpoint".equals(args[i])) {
                endpoint = args[++i];
            } else if ("-keyspace".equals(args[i])) {
                keyspace = args[++i];
            } else if ("-table".equals(args[i])) {
                table = args[++i];
            } else if ("-filename".equals(args[i])) {
                filename = args[++i];
            } else if ("-concurrency".equals(args[i])) {
                concurrency = Integer.parseInt(args[++i]);
            } else {
                System.err.println("BenchLookup -endpoint hostname -keyspace keyspace -table table -filename access-list-file -concurrency concurrency");
            }
        }
        if (endpoint == null || keyspace == null || table == null || filename == null) {
            System.err.println("BenchLookup -endpoint hostname -keyspace keyspace -table table -filename access-list-file -concurrency concurrency");
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

            queue = new LinkedBlockingQueue<String>();
            File file = new File(filename);
            BufferedReader br = new BufferedReader(new FileReader(file));
            int numRecords = 0;
            while (true) {
                String line = br.readLine();
                if (line == null) {
                    break;
                }
                queue.add(line);
                ++numRecords;
            }
            br.close();

            long start = System.currentTimeMillis();
            Thread []threads = new Lookuper[concurrency];
            for (int i = 0; i < concurrency; ++i) {
                threads[i] = new BenchLookup().new Lookuper();
                threads[i].start();
            }

            for (int i = 0; i < concurrency; ++i) {
                threads[i].join();
            }
            long end = System.currentTimeMillis();

            long interval = (end - start) / 1000;
            System.out.println("time taken (s) : " + interval);
            System.out.println("throughput (records/s) : " + numRecords/interval);

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private class Lookuper extends Thread {
        public Lookuper() {
        }

        public void run() {
            try {
                while (true) {
                    String line = queue.poll(1, TimeUnit.MILLISECONDS);
                    if (line == null) {
                        return;
                    }
                    String[] items = line.split("=");

                    Statement statement = QueryBuilder.select().all().from(table)
                                                .where(QueryBuilder.eq(items[0], Integer.parseInt(items[1])));
                    results = session.execute(statement);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

}
