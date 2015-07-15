import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;

class TestDBPut {
    public static void main(String[] argv) {
        String databaseName = "test.db";

        // Create the database object.
        // There is no environment for this simple example.
        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorStream(System.err);
        dbConfig.setErrorPrefix("Test");
        dbConfig.setType(DatabaseType.BTREE);
        dbConfig.setAllowCreate(true);
        Database table = null;
        try {
            table = new Database(databaseName, null, dbConfig);
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        for (int i = 0; i < 1000000; ++i) {
            Integer key = new Integer(i);
            DatabaseEntry keyEntry = new DatabaseEntry();
            EntryBinding keyBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            keyBinding.objectToEntry(key, keyEntry);
            DatabaseEntry valEntry = new DatabaseEntry();
            EntryBinding valBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            valBinding.objectToEntry(key, valEntry);

            try {
                if (table.put(null, keyEntry, valEntry) != OperationStatus.SUCCESS) {
                    System.err.println("Operation failed.");
                }
            } catch (DatabaseException dbe) {
                System.err.println(dbe.toString());
            }
        }
        try {
            table.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}
