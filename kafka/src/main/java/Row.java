import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.Serializable;
import java.util.*;

/**
 * Created by hiroyuki on 11/5/15.
 */
public class Row implements Cloneable, Serializable {
    private Map<String, Column> columns;
    private final static Logger LOGGER = LoggerFactory.getLogger(Row.class);

    /**
     * Construct a empty row.
     * Row is composed of some columns
     */
    public Row() {
        this.columns = new LinkedHashMap<String, Column>();
    }

    /**
     * Construct a row having the columns.
     * @param cols columns
     */
    public Row(Column... cols) {
        this();
        for (Column c : cols) {
            this.put(c);
        }
    }

    /**
     * Construct a row having the columns.
     * @param cols a list of rows
     */
    public Row(List<Column> cols) {
        this();
        for (Column c : cols) {
            this.put(c);
        }
    }

    /**
     * Get a number of columns in the row.
     * @return a number of columns in the row
     */
    public int size() {
        return columns.size();
    }

    /**
     * Return true if the row is empty.
     * @return true if the row is empty
     */
    public boolean isEmpty() {
        return columns.isEmpty();
    }

    /**
     * Return true if the row has the key.
     * @param key a name of key
     * @return true if the row has the key
     */
    public boolean containsKey(String key) {
        return columns.containsKey(key);
    }

    /**
     * Return true if the row has the column.
     * @param column a column
     * @return true if the row has the column
     */
    public boolean containsValue(Column column) {
        return columns.containsValue(column);
    }

    /**
     * Get a column corresponding to the key.
     * @param key a name of column
     * @return a column
     */
    public Column get(String key) {
        return columns.get(key);
    }

    /**
     * Put a column to the row.
     * @param column a column
     * @return a column
     */
    public Column put(Column column) {
        return columns.put(column.getName(), column);
    }

    /**
     * Put all columns to the row.
     * @param another a row
     */
    public void put(Row another) {
        // TODO: is there any better way ?
        for (Map.Entry<String, Column> entry : another.entrySet()) {
            this.put(entry.getValue());
        }
    }

    /**
     * Remove a column corresponding to the key.
     * @param key a name of column
     * @return a column corresponding to the key
     */
    public Column remove(String key) { return columns.remove(key); }

    /**
     * Clear all columns in the row.
     */
    public void clear() {
        columns.clear();
    }

    /**
     * Get all columns' names as a set of string.
     * @return a set of names of columns
     */
    public Set<String> keySet() {
        return columns.keySet();
    }

    /**
     * Get all columns.
     * @return a collection of columns
     */
    public Collection<Column> values() {
        return columns.values();
    }

    /**
     * Get all columns as a set.
     * @return a set of columns
     */
    public Set<Map.Entry<String, Column>> entrySet() {
        return columns.entrySet();
    }

    /**
     * Return true if the object equals to the row
     * @param obj an object compared with the row
     * @return true if the object equals to the row
     */
    @Override
    public boolean equals(Object obj) {
        Row another = (Row) obj;
        for (Map.Entry<String, Column> entry : this.entrySet()) {
            if (!another.containsKey(entry.getKey()) ||
                    !entry.getKey().equals(another.get(entry.getKey()).getName())) {
                return false;
            }
            if (!entry.getValue().equals(another.get(entry.getKey()))) {
                return false;
            }
        }
        for (Map.Entry<String, Column> entry : another.entrySet()) {
            if (!this.containsKey(entry.getKey()) ||
                    !entry.getKey().equals(this.get(entry.getKey()).getName())) {
                return false;
            }
            if (!entry.getValue().equals(this.get(entry.getKey()))) {
                return false;
            }
        }
        return true;
    }

    /**
     * Get a string representation of the row. Should
     * only be used for debugging purposes.
     * @return a string representation of the row
     */
    @Override
    public String toString() {
        StringBuffer sb = new StringBuffer();
        for (Column column : this.values()) {
            sb.append(column);
        }
        return sb.toString();
    }

    /**
     * Get a string having names and types of all columns
     * @return a string having names and types of columns
     */
    public String getSerializedName() {
        StringBuffer sb = new StringBuffer();
        for (Column column : this.values()) {
            sb.append(column.getName());
            sb.append(column.getType());
        }
        return sb.toString();
    }

    /**
     * Get a copy object of the row
     * @return a copy object of the row
     */
    @Override
    public Object clone() {
        Row row = new Row();
        // deep copy
        for (Column column : columns.values()) {
            row.put(new Column(column.getName(), column.getBytes(), column.getType()));
        }
        return row;
    }
}
