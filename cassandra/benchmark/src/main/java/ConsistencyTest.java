import java.io.*;

import com.datastax.driver.core.*;
import com.datastax.driver.core.querybuilder.*;
import com.datastax.driver.core.policies.*;

public class ConsistencyTest {

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
                System.err.println("ConsistencyTest -endpoint hostname -keyspace keyspace -table table");
            }
        }
        if (endpoint == null || keyspace == null || table == null) {
            System.err.println("ConsistencyTest -endpoint hostname -keyspace keyspace -table table");
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


            // work with ONE for both write and read
            long s1 = System.currentTimeMillis();
            Statement insert = QueryBuilder.insertInto(table)
                .value("user_id", 100)
                .value("fname", "john")
                .value("lname", "yamada")
                .value("number", 100);
            insert.setConsistencyLevel(ConsistencyLevel.ONE);
            session.execute(insert);
            long e1 = System.currentTimeMillis();
            System.out.println("write ONE time taken (ms) : " + (e1 - s1));

            long s2 = System.currentTimeMillis();
            Statement select = QueryBuilder.select().all().from(table)
                                        .where(QueryBuilder.eq("user_id", 100));
            select.setConsistencyLevel(ConsistencyLevel.ONE);
            results = session.execute(select);
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long e2 = System.currentTimeMillis();
            System.out.println("read ONE time taken (ms) : " + (e2 - s2));

            Statement delete = QueryBuilder.delete().from(table).where(QueryBuilder.eq("user_id", 100));
            results = session.execute(delete);

            // work with QUORUM for both write and read
            long s3 = System.currentTimeMillis();
            Statement insert2 = QueryBuilder.insertInto(table)
                .value("user_id", 200)
                .value("fname", "john")
                .value("lname", "yamada")
                .value("number", 100);
            insert2.setConsistencyLevel(ConsistencyLevel.QUORUM);
            session.execute(insert2);
            long e3 = System.currentTimeMillis();
            System.out.println("write QUORUM time taken (ms) : " + (e3 - s3));

            long s4 = System.currentTimeMillis();
            Statement select2 = QueryBuilder.select().all().from(table)
                                        .where(QueryBuilder.eq("user_id", 200));
            select2.setConsistencyLevel(ConsistencyLevel.QUORUM);
            results = session.execute(select2);
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long e4 = System.currentTimeMillis();
            System.out.println("read QUORUM time taken (ms) : " + (e4 - s4));

            Statement delete2 = QueryBuilder.delete().from(table).where(QueryBuilder.eq("user_id", 200));
            results = session.execute(delete2);

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
