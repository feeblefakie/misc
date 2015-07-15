import java.util.Properties;
import java.io.*;

public class Test {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Test file.properties");
            System.exit(1);
        }
        Properties prop = new Properties();
        try {
            InputStream in = new FileInputStream(args[0]);
            prop.load(in);

            for (String key : prop.stringPropertyNames()) {
                String value = prop.getProperty(key);
                String[] items = value.split(",");
                System.out.println("local index: " + items[0]);
                System.out.println("key types: " + items[1]);
                System.out.println("value types: " + items[2]);
                System.out.println(key + " = " + value);
            }
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}
