import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.nio.ByteBuffer;

/**
 * Created by hiroyuki on 11/2/15.
 */
public class TypeConverter {

    private final static Logger LOGGER = LoggerFactory.getLogger(TypeConverter.class);

    public static Object getObjectFromBytes(ByteBuffer bytes, DataType type)
            throws UnsupportedTypeException {
        bytes.rewind();
        Object obj = null;
        switch (type) {
            case BOOLEAN:
                obj = new Boolean((byte) bytes.get() > 0 ? true : false);
                break;
            case INT:
                obj = new Integer(bytes.getInt());
                break;
            case BIGINT:
                obj = new Long(bytes.getLong());
                break;
            case FLOAT:
                obj = new Float(bytes.getFloat());
                break;
            case DOUBLE:
                obj = new Double(bytes.getDouble());
                break;
            case STRING:
                obj = new String(bytes.array());
                break;
            case BLOB:
                obj = bytes.array();
                break;
            default:
                throw new UnsupportedTypeException();
        }
        bytes.clear();
        return obj;
    }

    public static byte[] getByteArrayFrom(Row row) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] bytes;
        try {
            ObjectOutput out = new ObjectOutputStream(bos);
            out.writeObject(row);
            out.flush();
            bytes = bos.toByteArray();
            return bytes;
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        } finally {
            try {
                bos.close();
            } catch (IOException ex) {
                // ignore close exception
            }
        }
    }

    public static Row getRowFrom(byte[] bytes) throws ClassNotFoundException, IOException {
        ByteArrayInputStream bis = new ByteArrayInputStream(bytes);
        ObjectInput in = null;
        try {
            in = new ObjectInputStream(bis);
            Object o = in.readObject();
            return (Row) o;
        } catch (IOException e) {
            e.printStackTrace();
            throw e;
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            throw e;
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (IOException ex) {
                // ignore close exception
            }
        }
    }
}
