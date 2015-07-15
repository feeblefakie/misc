package db.mc;

import java.util.List;
import java.util.ArrayList;
import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.tuple.TupleInput;

public class MultiColumnTest {
    public static void main(String[] arg) {

        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorStream(System.err);
        dbConfig.setErrorPrefix("Test");
        dbConfig.setType(DatabaseType.BTREE);
        dbConfig.setAllowCreate(true);
        Database table = null;
        try {
            table = new Database("mctest.db", null, dbConfig);
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        MyTupleBinding keyBinding = new MyTupleBinding();
        MultiColumn columns = new MultiColumn();
        columns.addInteger(new Integer(100));
        columns.addInteger(new Integer(1));
        columns.addDouble(new Double(12345.6789));

        MyTupleBinding keyBinding2 = new MyTupleBinding();
        MultiColumn columns2 = new MultiColumn();
        columns2.addInteger(new Integer(100));
        columns2.addInteger(new Integer(2));
        columns2.addDouble(new Double(12345.6789));

        DatabaseEntry keyEntry = new DatabaseEntry();
        DatabaseEntry valEntry = new DatabaseEntry();
        EntryBinding valBinding = TupleBinding.getPrimitiveBinding(Integer.class);
        valBinding.objectToEntry(new Integer(1), valEntry);

        DatabaseEntry keyEntry2 = new DatabaseEntry();
        DatabaseEntry valEntry2 = new DatabaseEntry();
        valBinding.objectToEntry(new Integer(2), valEntry2);

        try {
            // Store theKeyData in the DatabaseEntry
            keyBinding.objectToEntry(columns, keyEntry);
            keyBinding2.objectToEntry(columns2, keyEntry2);

            if (table.put(null, keyEntry, valEntry) != OperationStatus.SUCCESS) {
                System.err.println("Operation failed.");
            }
            if (table.put(null, keyEntry2, valEntry2) != OperationStatus.SUCCESS) {
                System.err.println("Operation failed.");
            }

            if (table.get(null, keyEntry2, valEntry, null) != OperationStatus.SUCCESS) {
                System.err.println("Operation failed.");
            }
            Integer integer = (Integer) valBinding.entryToObject(valEntry);
            System.out.println("hit: " + integer.intValue());


            // Retrieve the key data
            MyTupleBinding keyBinding3 = new MyTupleBinding();
            List<String> seqs = new ArrayList<String>();
            seqs.add("integer");
            seqs.add("integer");
            seqs.add("double");
            keyBinding3.setEntrySequences(seqs);
            MultiColumn res = (MultiColumn) keyBinding3.entryToObject(keyEntry2);

            List<Object> list = res.getColumns();
            for (Object o : list) {
                if (o instanceof Integer) {
                    System.out.println((Integer) o);
                } else if (o instanceof Long) {
                    System.out.println((Long) o);
                } else if (o instanceof Double) {
                    System.out.println((Double) o);
                }
            }
            table.close();
        } catch (Exception e) {
            // Exception handling goes here
        }
    } 
}
