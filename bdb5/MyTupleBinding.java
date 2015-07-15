package db.mc;

import java.util.List;
import java.util.ArrayList;
import com.sleepycat.bind.tuple.TupleBinding;
import com.sleepycat.bind.tuple.TupleInput;
import com.sleepycat.bind.tuple.TupleOutput;

public class MyTupleBinding extends TupleBinding {
    private List<String> entrySeqs = new ArrayList<String>();

    // Write a MyData2 object to a TupleOutput
    public void objectToEntry(Object object, TupleOutput to) {
        MultiColumn mc = (MultiColumn) object;
        List<Object> columns = mc.getColumns();
        for (Object o : columns) {
            if (o instanceof Integer) {
                to.writeInt(((Integer) o).intValue());
            } else if (o instanceof Long) {
                to.writeLong(((Long) o).longValue());
            } else if (o instanceof Double) {
                to.writeDouble(((Double) o).doubleValue());
            }
        }
    }
   
    public Object entryToObject(TupleInput ti) {
        // Data must be read in the same order that it was originally written.
        MultiColumn mc = new MultiColumn();
        for (String entry : entrySeqs) {
            if (entry.compareTo("integer") == 0) {
                Integer value = new Integer(ti.readInt());
                mc.addInteger(value);
            } else if (entry.compareTo("long") == 0) {
                Long value = new Long(ti.readLong());
                mc.addLong(value);
            } else if (entry.compareTo("double") == 0) {
                Double value = new Double(ti.readDouble());
                mc.addDouble(value);
            }
        }
        return mc;
    }

    public void setEntrySequences(List<String> entrySeqs) {
        this.entrySeqs = entrySeqs;
    }
}
