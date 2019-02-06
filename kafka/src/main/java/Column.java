import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.nio.ByteBuffer;

/**
 * Created by hiroyuki on 10/28/15.
 */
public class Column implements NamedBytes, Cloneable, Serializable {
    private final static Logger LOGGER = LoggerFactory.getLogger(Column.class);
    private String name;
    private byte[] byteArray;
    private transient ByteBuffer bytes;
    private DataType type;

    /*
    public Column() {
    }
    */

    /**
     * Construct a column.
     *
     * @param name  a name of the column
     * @param bytes value of the column
     * @param type  type of the column
     */
    public Column(String name, ByteBuffer bytes, DataType type) {
        this.name = name;
        this.bytes = bytes;
        this.type = type;
        if (bytes != null) {
            byteArray = bytes.array();
        }
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public void setName(String name) {
        this.name = name;
    }

    @Override
    public ByteBuffer getBytes() {
        if (bytes == null && byteArray != null) {
            bytes = ByteBuffer.allocate(byteArray.length).put(byteArray);
        }
        return bytes;
    }

    @Override
    public void setBytes(ByteBuffer bytes) {
        this.bytes = bytes;
        byteArray = bytes.array();
    }

    @Override
    public DataType getType() {
        return type;
    }

    @Override
    public void setType(DataType type) {
        this.type = type;
    }

    @Override
    public boolean equals(Object obj) {
        Column another = (Column) obj;

        try {
            if (this.getName().equals(another.getName()) &&
                    TypeConverter.getObjectFromBytes(this.getBytes(), this.getType()).equals(
                            TypeConverter.getObjectFromBytes(another.getBytes(), another.getType())) &&
                    this.getType() == another.getType()) {
                return true;
            }
        } catch (UnsupportedTypeException e) {
            LOGGER.error("", e);
            return false;
        }
        return false;
    }

    /**
     * Get a string representation of the column
     *
     * @return a string representation of the column
     */
    @Override
    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("name : #" + getName() + "#\n");
        sb.append("bytes(ByteBuffer) : #" + getBytes() + "#\n");
        try {
            sb.append("bytes : #" + TypeConverter.getObjectFromBytes(getBytes(), getType()) + "#\n");
        } catch (Exception e) {
            sb.append("bytes : ERROR (could not be converted correctly)\n");
        }
        sb.append("type : #" + getType() + "#\n");
        return sb.toString();
    }

    /**
     * Get a copy object of the column
     *
     * @return a copy object of the column
     */
    @Override
    public Object clone() {
        try {
            // all the items are immutable, so just do shallow copy
            return super.clone();
        } catch (CloneNotSupportedException e) {
            LOGGER.error("", e);
            return null;
        }
    }

    /*
    private void writeObject(ObjectOutputStream stream) throws IOException {
        stream.writeChars(name);
        stream.write(byteArray);
        stream.writeObject(type);
    }

    private void readObject(java.io.ObjectInputStream stream) throws IOException, ClassNotFoundException {
        name = (String) stream.readObject();
        stream.read(byteArray);
        type = (DataType) stream.readObject();
    }
    */
}
