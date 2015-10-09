
import com.datastax.driver.core.*;

public class GettingStarted {
    //public static Logger logger = Logger.getLogger("RbReport");
    public static void main(String[] args) {

        Cluster cluster;
        Session session;

        try {

            // Connect to the cluster and keyspace "demo"
            cluster = Cluster.builder().addContactPoint("192.168.110.103").build();
            session = cluster.connect("mykeyspace");

            // Insert one record into the users table
            session.execute("INSERT INTO users (user_id, fname, lname, number) VALUES (1000, 'john', 'yamada', 100)");
            System.out.println("hello1");

            // Use select to get the user we just entered
            ResultSet results = session.execute("SELECT * FROM users WHERE lname='yamada'");
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            System.out.println("hello2");

            // Update the same user with a new age
            session.execute("update users set number = 101 where user_id = 1000");
            // Select and show the change
            results = session.execute("select * from users where lname='yamada'");
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            System.out.println("hello3");

            // Delete the user from the users table
            session.execute("DELETE FROM users WHERE user_id = 1000");
            // Show that the user is gone
            results = session.execute("SELECT * FROM users");
            for (Row row : results) {
                System.out.format("%d %s %s %d\n", row.getInt("user_id"), row.getString("fname"), row.getString("lname"), row.getInt("number"));
            }
            System.out.println("hello4");
            /*
            */

            // Clean up the connection by closing it
            cluster.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
