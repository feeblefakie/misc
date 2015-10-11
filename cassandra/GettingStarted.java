
import com.datastax.driver.core.*;
import java.util.Calendar;

public class GettingStarted {
    //public static Logger logger = Logger.getLogger("RbReport");
    public static void main(String[] args) {

        Cluster cluster;
        Session session;

        try {

            long start = System.currentTimeMillis();
            // Connect to the cluster and keyspace "demo"
            cluster = Cluster.builder().addContactPoint("192.168.110.103").build();
            session = cluster.connect("mykeyspace");
            long cp0 = System.currentTimeMillis();
            System.out.println(cp0 - start);

            // Insert one record into the users table
            session.execute("INSERT INTO users (user_id, fname, lname, number) VALUES (1000, 'john', 'yamada', 100)");
            long cp1 = System.currentTimeMillis();
            System.out.println(cp1 - cp0);

            // Use select to get the user we just entered
            ResultSet results = session.execute("SELECT * FROM users WHERE lname='yamada'");
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long cp2 = System.currentTimeMillis();
            System.out.println(cp2 - cp1);

            // Update the same user with a new age
            session.execute("update users set number = 101 where user_id = 1000");
            long cp3 = System.currentTimeMillis();
            System.out.println(cp3 - cp2);

            // Select and show the change
            results = session.execute("select * from users where lname='yamada'");
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long cp4 = System.currentTimeMillis();
            System.out.println(cp4 - cp3);

            // Delete the user from the users table
            session.execute("DELETE FROM users WHERE user_id = 1000");
            long cp5 = System.currentTimeMillis();
            System.out.println(cp5 - cp4);

            // Show that the user is gone
            results = session.execute("SELECT * FROM users");
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            long cp6 = System.currentTimeMillis();
            System.out.println(cp6 - cp5);
            /*
            */

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
