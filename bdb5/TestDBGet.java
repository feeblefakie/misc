import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;

class TestDBGet {
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

        for (int i = 0; i < 200; ++i) {
            /*
            Integer key = new Integer(i);
            DatabaseEntry keyEntry = new DatabaseEntry();
            EntryBinding keyBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            keyBinding.objectToEntry(key, keyEntry);
            */
            String a = String.valueOf(i);
            /*
            String key = "111";
            System.out.println(a + " ||| " + key);
            if (key.equals(a)) {
                System.out.println("matched");
            }
            */
            DatabaseEntry keyEntry = new DatabaseEntry(a.getBytes());
            DatabaseEntry valEntry = new DatabaseEntry();
            try {
                OperationStatus status = table.get(null, keyEntry, valEntry, null);
                if (status != OperationStatus.SUCCESS) {
                    System.err.println("Operation failed.");
                    if (status == OperationStatus.KEYEMPTY) {
                        System.err.println("empty");
                    } else if (status == OperationStatus.NOTFOUND) {
                        System.err.println(valEntry);
                    }
                } else {
                    System.out.println(new String(valEntry.getData()));
                }
            } catch (Exception e) {
                System.err.println(e.getMessage());
            }
        }
    }
}
