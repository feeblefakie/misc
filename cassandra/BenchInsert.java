import java.io.*;
import com.datastax.driver.core.*;
import com.datastax.driver.core.querybuilder.*;
import com.datastax.driver.core.policies.*;

public class BenchInsert {

    public static void main(String[] args) {
        String endpoint = null;
        String keyspace = null;
        String filename = null;

        for (int i = 0; i < args.length; ++i) {
            if ("-endpoint".equals(args[i])) {
                endpoint = args[++i];
            } else if ("-keyspace".equals(args[i])) {
                keyspace = args[++i];
            } else if ("-filename".equals(args[i])) {
                filename = args[++i];
            } else {
                System.err.println("BenchInsert -endpoint hostname -keyspace keyspace -filename access-list-file");
            }
        }
        if (endpoint == null || keyspace == null || filename == null) {
            System.err.println("BenchInsert -endpoint hostname -keyspace keyspace -filename access-list-file");
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

            PreparedStatement statement = session.prepare("INSERT INTO users (user_id, fname, lname, number) VALUES (?, ?, ?, ?)");
            BoundStatement boundStatement = new BoundStatement(statement);

            int numRecords = 0;
            while (true) {
                String line = br.readLine();
                if (line == null) {
                    break;
                }

                String[] items = line.split(",");

                // Insert one record into the users table
                session.execute(boundStatement.bind(Integer.parseInt(items[0]), items[1], items[2], Integer.parseInt(items[3])));
                ++numRecords;
                System.out.println("inserted " + numRecords);
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