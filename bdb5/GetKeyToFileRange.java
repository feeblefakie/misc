package org.apache.hadoop.hive.ql.io;

import java.io.*;
import com.sleepycat.db.*;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.serial.StoredClassCatalog;
import com.sleepycat.bind.serial.SerialBinding;

class GetKeyToFileRange {
    public static void main(String[] argv) {
        if (argv.length != 3) {
            System.err.println("GetKeyToFileRange inFile outFile keyType(double|integer)");
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
        dbConfig.setAllowCreate(false);;
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
                DatabaseEntry valEntry = new DatabaseEntry();
                if (table.get(null, keyEntry, valEntry, LockMode.DEFAULT) == OperationStatus.SUCCESS) {
                    FileRange fileRange = (FileRange) valBinding.entryToObject(valEntry);
                    System.out.println("offset:" + fileRange.getOffset() + ", length:" + fileRange.getLength());
                } else {
                    System.err.println("Error");
                }
            }
            in.close();
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
