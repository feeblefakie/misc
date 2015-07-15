package org.apache.hadoop.hive.ql.io;

import java.io.*;
import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.serial.StoredClassCatalog;
import com.sleepycat.bind.serial.SerialBinding;

class PutKeyToFileRange {
    public static void main(String[] argv) {
        if (argv.length != 3) {
            System.err.println("PutKeyToFileRange inFile outFile keyType(double|integer)");
            System.exit(1);
        }
        String inFile = argv[0];
        String databaseName = argv[1];
        Utilities.KeyType keyType = Utilities.identifyKeyType(argv[2]);

        // Create the database object.
        // There is no environment for this simple example.
        DatabaseConfig dbConfig = new DatabaseConfig();
        dbConfig.setErrorStream(System.err);
        dbConfig.setErrorPrefix("Test");
        dbConfig.setType(DatabaseType.BTREE);
        dbConfig.setAllowCreate(true);
        dbConfig.setSortedDuplicates(true);
        Database table = null;
        Database classDB = null;
        StoredClassCatalog classCatalog = null;
        try {
            table = new Database(databaseName, null, dbConfig);
            dbConfig.setSortedDuplicates(false);
            classDB = new Database("classDB", null, dbConfig); 
            classCatalog = new StoredClassCatalog(classDB);
            EntryBinding valBinding = new SerialBinding(classCatalog, FileRange.class);

            FileInputStream fstream = new FileInputStream(inFile);
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));
            String line = null;
            while ((line = br.readLine()) != null) {
                String []items = line.split("\\|");
                DatabaseEntry keyEntry = Utilities.getKeyEntry(items[0], keyType);
                FileRange fileRange = new FileRange(Long.parseLong(items[1]),
                                                    Integer.parseInt(items[2]));
                DatabaseEntry valEntry = new DatabaseEntry();
                valBinding.objectToEntry(fileRange, valEntry);
                if (table.put(null, keyEntry, valEntry) != OperationStatus.SUCCESS) {
                    System.err.println("Error");
                }
            }
            in.close();
            table.close();
            classDB.close();
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
