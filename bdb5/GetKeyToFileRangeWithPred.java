package org.apache.hadoop.hive.ql.io;

import java.io.*;
import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.serial.StoredClassCatalog;
import com.sleepycat.bind.serial.SerialBinding;

class GetKeyToFileRangeWithPred {
    public static void main(String[] argv) {
        if (argv.length != 3) {
            System.err.println("GetKeyToFileRange predicate outFile keyType(double|integer)");
            System.exit(1);
        }
        // "<(p_retailprice:double,1000)"
        String predicate = argv[0];
        String databaseName = argv[1];
        Utilities.KeyType keyType = Utilities.identifyKeyType(argv[2]);

        PredicateInfo predRange = new PredicateInfo(predicate);

        // Create the database object.
        // There is no environment for this simple example.
        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorStream(System.err);
        dbConfig.setErrorPrefix("Test");
        dbConfig.setType(DatabaseType.BTREE);
        dbConfig.setAllowCreate(false);;
        Database table = null;
        Database classDB = null;
        StoredClassCatalog classCatalog = null;
        try {
            table = new Database(databaseName, null, dbConfig);
            dbConfig.setSortedDuplicates(false);
            dbConfig.setAllowCreate(true);
            classDB = new Database("classDB", null, dbConfig); 
            classCatalog = new StoredClassCatalog(classDB);
            EntryBinding valBinding = new SerialBinding(classCatalog, FileRange.class);

            // create key value entry objects for results
            DatabaseEntry keyEntry;
            if (predRange.getStart() != null) {
                keyEntry = Utilities.getKeyEntry(predRange.getStart(), keyType);
            } else {
                keyEntry = new DatabaseEntry();
            }
            Object keyObjectEnd = null;
            if (predRange.getEnd() != null) {
                keyObjectEnd = Utilities.getKeyObject(predRange.getEnd(), keyType);
            }
            DatabaseEntry valEntry = new DatabaseEntry();

            // set starting point
            Cursor cursor = table.openCursor(null, null);
            if (predRange.getStart() == null) {
                cursor.getFirst(keyEntry, valEntry, null);
            } else {
                cursor.getSearchKey(keyEntry, valEntry, null);
            }
            if (!predRange.isStartContainsBound()) {
                cursor.getNextNoDup(keyEntry, valEntry, null);
            }

            // scan from lower to higher
            EntryBinding keyBinding = Utilities.getEntryBinding(keyType);
            do {
                Object got = keyBinding.entryToObject(keyEntry);
                // chech if the cursor reaches the specified end point
                if (predRange.getEnd() != null) {
                    if (predRange.isEndContainsBound()) {
                        if (Utilities.genericCompareTo(got, keyObjectEnd, keyType) > 0) {
                            break;
                        }
                    } else {
                        if (Utilities.genericCompareTo(got, keyObjectEnd, keyType) == 0) {
                            break;
                        }
                    }
                }
                System.out.println("key: " + got);
                FileRange fileRange = (FileRange) valBinding.entryToObject(valEntry);
                System.out.println("offset:" + fileRange.getOffset() + ", length:" + fileRange.getLength());
            } while (cursor.getNext(keyEntry, valEntry, null) != OperationStatus.NOTFOUND);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
