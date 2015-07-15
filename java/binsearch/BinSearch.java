import java.io.*;
import java.util.*;
import java.nio.*;

class BinSearch {
    private RandomAccessFile index;
    //private Integer[] keys;
    //private Integer[] vals;
    //private ArrayList<Integer> list;

    public BinSearch(RandomAccessFile index) {
        this.index = index;
        //List<Integer> l = Arrays.asList(keys);
        //list = new ArrayList<Integer>(l);
    }

    public int search(Integer key) throws IOException {
        int numRecords = (int) index.length() / 8;
        int high = numRecords - 1;
        int low = 0;
        int bound = numRecords;
        System.err.println("numRecords: " + numRecords);
        byte []record = new byte[8];
        boolean isFound = false;
        Integer indexedKey = null;
        Integer indexedVal = null;
        while (low <= high) {
            int mid = (low + high) / 2;
            long offset = mid * 8;
            index.seek(offset);
            index.readFully(record);
            ByteBuffer buf = ByteBuffer.allocate(8);
            buf.order(ByteOrder.LITTLE_ENDIAN);
            buf.put(record);
            buf.flip();
            indexedKey = buf.getInt();
            if (indexedKey.intValue() == key) {
                indexedVal = buf.getInt();
                isFound = true;
                break;
            } else if (indexedKey.intValue() > key) {
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }

        /*
        index.seek(0);
        for (int i = 0; i < numRecords; ++i) {
            index.readFully(record);
            ByteBuffer buf = ByteBuffer.allocate(8);
            buf.order(ByteOrder.LITTLE_ENDIAN);
            buf.put(record);
            buf.flip();
            indexedKey = buf.getInt();
            indexedVal = buf.getInt();
            if (indexedKey.intValue() >= key) {
                isFound = true;
                break;
            }
        }
        */
        if (isFound) {
            return indexedVal.intValue();
        }
        /*
        int offset = Collections.binarySearch(list, key);
        if (offset >= 0 || offset < vals.length) {
            return vals[offset];
        }
        */
        return -1;
    }
}
