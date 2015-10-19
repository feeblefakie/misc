import com.datastax.driver.core.*;
import com.datastax.driver.core.querybuilder.*;
import com.datastax.driver.core.policies.*;

public class GettingStartedTwo {

    public static void main(String[] args) {

	Cluster cluster;
	Session session;
	ResultSet results;
	Row rows;

        try {

            long start = System.currentTimeMillis();
            // Connect to the cluster and keyspace "demo"
            cluster = Cluster
                .builder()
                .addContactPoint("192.168.110.103")
                .withRetryPolicy(DefaultRetryPolicy.INSTANCE)
                .withLoadBalancingPolicy(new TokenAwarePolicy(new DCAwareRoundRobinPolicy()))
                .build();
            session = cluster.connect("mykeyspace");
            long cp0 = System.currentTimeMillis();
            System.out.println(cp0 - start);

            // Insert one record into the users table
            /*
            PreparedStatement statement = session.prepare("INSERT INTO users (user_id, fname, lname, number) VALUES (?, ?, ?, ?)");
            BoundStatement boundStatement = new BoundStatement(statement);
            session.execute(boundStatement.bind(1000, "john", "yamada", 100));
            */

            /*
            Statement statement = QueryBuilder.insertInto("mykeyspace", "users2")
                .value("user_id", 100)
                .value("fname", "john")
                .value("lname", "yamada")
                .value("number", 100);
            session.execute(statement);
            */

            Insert insert = QueryBuilder.insertInto("mykeyspace", "users2")
                .value("user_id", QueryBuilder.bindMarker())
                .value("fname", QueryBuilder.bindMarker())
                .value("lname", QueryBuilder.bindMarker())
                .value("number", QueryBuilder.bindMarker());
            PreparedStatement preparedStatement = session.prepare(insert);
            BoundStatement boundStatement = new BoundStatement(preparedStatement);
            session.execute(boundStatement.bind(100, "john", "yamada", 100));

            long cp1 = System.currentTimeMillis();
            System.out.println(cp1 - cp0);

            // Use select to get the user we just entered
            Statement select = QueryBuilder.select().all().from("mykeyspace", "users2")
                .where(QueryBuilder.eq("lname", "yamada"));
            results = session.execute(select);
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long cp2 = System.currentTimeMillis();
            System.out.println(cp2 - cp1);

            // Update the same user with a new age
            Statement update = QueryBuilder.update("mykeyspace", "users2")
                .with(QueryBuilder.set("number", 101))
                .where((QueryBuilder.eq("user_id", 100)));
            session.execute(update);
            long cp3 = System.currentTimeMillis();
            System.out.println(cp3 - cp2);
            
            // Select and show the change
            select = QueryBuilder.select().all().from("mykeyspace", "users2").where(QueryBuilder.eq("lname", "yamada"));
            results = session.execute(select);
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long cp4 = System.currentTimeMillis();
            System.out.println(cp4 - cp3);

            // Delete the user from the users table
            Statement delete = QueryBuilder.delete().from("users2").where(QueryBuilder.eq("user_id", 100));
            results = session.execute(delete);
            long cp5 = System.currentTimeMillis();
            System.out.println(cp5 - cp4);

            // Show that the user is gone
            select = QueryBuilder.select().all().from("mykeyspace", "users2");
            results = session.execute(select);
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long cp6 = System.currentTimeMillis();
            System.out.println(cp6 - cp5);

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
