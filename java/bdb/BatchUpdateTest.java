import java.util.List;
import java.util.ArrayList;
import com.sleepycat.db.Database;
import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.db.DatabaseException;
import com.sleepycat.db.DatabaseConfig;
import com.sleepycat.db.DatabaseType;
import com.sleepycat.db.OperationStatus;
import com.sleepycat.db.MultipleEntry;
import com.sleepycat.db.MultipleKeyDataEntry;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;

public class BatchUpdateTest {
    public static void main(String[] args) throws Exception {

        DatabaseConfig config = new DatabaseConfig();
        config.setErrorStream(System.err);
        config.setErrorPrefix("Test");
        config.setType(DatabaseType.BTREE);
        config.setAllowCreate(true);
        config.setSortedDuplicates(true);
        config.setCacheSize(134217728);
        Database index = new Database("./test.db", null, config);

        long total = 0;
        byte [] buffer = new byte[10240];
        MultipleKeyDataEntry entry = new MultipleKeyDataEntry(buffer);
        for (int i = 0; i < 10000; ++i) {
            String key = String.valueOf(i);
            String value = key + "-value";
            System.out.println(key + " - " + value);
            DatabaseEntry keyEntry = new DatabaseEntry(key.getBytes());
            DatabaseEntry valueEntry = new DatabaseEntry(value.getBytes());
            total += keyEntry.getSize() + valueEntry.getSize() + 16;  // 4bytes * 4
            System.out.println("total: " + total);
            if (total + 64 > 10240) { // internal representation is unknown, so take 64 bytes just in case
                System.err.println("no more buffer space at " + i);
                break;
            }
            entry.append(keyEntry, valueEntry);
        }

        index.putMultipleKey(null, entry, true);
        index.close();

    }
}
