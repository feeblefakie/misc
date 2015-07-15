package org.apache.hadoop.hive.ql.io;

import com.sleepycat.db.DatabaseEntry;
import com.sleepycat.bind.EntryBinding;
import com.sleepycat.bind.tuple.TupleBinding;

public final class Utilities {

    public static enum KeyType {
        INTEGER, DOUBLE, NONE
    };

    public static KeyType identifyKeyType(String keyType) {
        if (keyType.equals("integer")) {
            return KeyType.INTEGER;
        } else if (keyType.equals("double")) {
            return KeyType.DOUBLE;
        } else {
            return KeyType.NONE;
        }
    }

    public static int genericCompareTo(Object obj1, Object obj2, KeyType keyType) {
        switch (keyType) {
        case INTEGER:
            return ((Integer) obj1).compareTo((Integer) obj2);
        case DOUBLE:
            return ((Double) obj1).compareTo((Double) obj2);
        }
        // TODO
        return -1;
    }

    public static EntryBinding getEntryBinding(KeyType keyType) {
        EntryBinding keyBinding = null;
        switch (keyType) {
        case INTEGER:
            keyBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            break;
        case DOUBLE:
            keyBinding = TupleBinding.getPrimitiveBinding(Double.class);
            break;
        }
        return keyBinding;
    }

    public static DatabaseEntry getKeyEntry(String key, KeyType keyType) {
        DatabaseEntry keyEntry = new DatabaseEntry();
        EntryBinding keyBinding = null;
        switch (keyType) {
        case INTEGER:
            keyBinding = TupleBinding.getPrimitiveBinding(Integer.class);
            keyBinding.objectToEntry(new Integer(key), keyEntry);
            break;
        case DOUBLE:
            keyBinding = TupleBinding.getPrimitiveBinding(Double.class);
            keyBinding.objectToEntry(new Double(key), keyEntry);
            break;
        }
        return keyEntry;
    }

    public static Object getKeyObject(String key, KeyType keyType) {
        Object obj = null;
        switch (keyType) {
        case INTEGER:
            obj = new Integer(key);
            break;
        case DOUBLE:
            obj = new Double(key);
            break;
        }
        return obj;
    }
}
