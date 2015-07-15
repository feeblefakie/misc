import java.util.Random;
import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;

class TestDBGetR {
    public static void main(String[] argv) {
        String databaseName = "test.db";

        // Create the database object.
        // There is no environment for this simple example.
        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorStream(System.err);
        dbConfig.setErrorPrefix("Test");
        dbConfig.setType(DatabaseType.BTREE);
        dbConfig.setAllowCreate(false);
        dbConfig.setReadOnly(true);
        Database table = null;
        try {
            table = new Database(databaseName, null, dbConfig);
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        Random R = new Random();
        int max = 1000000;
        for (int i = 0; i < max; ++i) {
            int number = R.nextInt(max);
            Integer key = new Integer(number);
            DatabaseEntry keyEntry = new DatabaseEntry();
            EntryBinding keyBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            keyBinding.objectToEntry(key, keyEntry);
            DatabaseEntry valEntry = new DatabaseEntry();
            EntryBinding valBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            try {
                if (table.get(null, keyEntry, valEntry, null) != OperationStatus.SUCCESS) {
                    System.err.println("Operation failed.");
                }
            } catch (Exception e) {
                System.err.println(e.getMessage());
            }
        }
    }
}
