import java.io.*;
import com.datastax.driver.core.*;
import com.datastax.driver.core.querybuilder.*;
import com.datastax.driver.core.policies.*;

public class BenchLookup {

    public static void main(String[] args) {
        String endpoint = null;
        String keyspace = null;
        String table = null;
        String filename = null;

        for (int i = 0; i < args.length; ++i) {
            if ("-endpoint".equals(args[i])) {
                endpoint = args[++i];
            } else if ("-keyspace".equals(args[i])) {
                keyspace = args[++i];
            } else if ("-table".equals(args[i])) {
                table = args[++i];
            } else if ("-filename".equals(args[i])) {
                filename = args[++i];
            } else {
                System.err.println("BenchInsert -endpoint hostname -keyspace keyspace -table table -filename access-list-file");
            }
        }
        if (endpoint == null || keyspace == null || table == null || filename == null) {
            System.err.println("BenchInsert -endpoint hostname -keyspace keyspace -table table -filename access-list-file");
            System.exit(1);
        }

        Cluster cluster;
        Session session;
        ResultSet results;
        Row rows;

        try {
            cluster = Cluster
                .builder()
                .addContactPoint(endpoint)
                .withRetryPolicy(DefaultRetryPolicy.INSTANCE)
                .withLoadBalancingPolicy(new TokenAwarePolicy(new DCAwareRoundRobinPolicy()))
                .build();
            session = cluster.connect(keyspace);

            long start = System.currentTimeMillis();

            File file = new File(filename);
            BufferedReader br = new BufferedReader(new FileReader(file));

            int numRecords = 0;
            while (true) {
                String line = br.readLine();
                if (line == null) {
                    break;
                }

                String[] items = line.split("=");

                Statement statement = QueryBuilder.select().all().from(table)
                                            .where(QueryBuilder.eq(items[0], Integer.parseInt(items[1])));
                results = session.execute(statement);
                ++numRecords;
                System.out.println("selected " + numRecords);
            }
            br.close();

            long end = System.currentTimeMillis();
            long interval = end - start / 1000;
            System.out.println("time taken (ms) : " + interval);
            System.out.println("throughput (records/s) : " + interval/numRecords/1000);

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
