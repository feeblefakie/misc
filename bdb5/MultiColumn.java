package db.mc;

import java.util.List;
import java.util.ArrayList;

public class MultiColumn {
    private List<Object> columns = null;

    public MultiColumn() {
        columns = new ArrayList<Object>();
    }
    
    public void addInteger(Integer value) {
        columns.add(value);
    }

    public void addLong(Long value) {
        columns.add(value);
    }

    public void addDouble(Double value) {
        columns.add(value);
    }

    public List<Object> getColumns() {
        return columns;
    }

}
