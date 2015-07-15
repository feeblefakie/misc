package org.apache.hadoop.hive.ql.io;

import java.io.*;
import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.serial.StoredClassCatalog;
import com.sleepycat.bind.serial.SerialBinding;
import org.apache.hadoop.hive.ql.io.FileRange;

class CreateFileRange {
    public static void main(String[] argv) {

        // Create the database object.
        // There is no environment for this simple example.
        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorStream(System.err);
        dbConfig.setErrorPrefix("Test");
        dbConfig.setType(DatabaseType.BTREE);
        dbConfig.setAllowCreate(true);
        dbConfig.setSortedDuplicates(false);
        Database classDB = null;
        StoredClassCatalog classCatalog = null;
        try {
            classDB = new Database("/hadooode/warehouse/part_e/class.idx", null, dbConfig); 
            classCatalog = new StoredClassCatalog(classDB);
            EntryBinding valBinding = new SerialBinding(classCatalog, org.apache.hadoop.hive.ql.io.FileRange.class);
            FileRange fileRange = new FileRange(0, 125);
            DatabaseEntry valEntry = new DatabaseEntry();
            valBinding.objectToEntry(fileRange, valEntry);
            classDB.close();
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
