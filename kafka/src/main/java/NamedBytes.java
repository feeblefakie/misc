import java.nio.ByteBuffer;

/**
 * Created by hiroyuki on 10/28/15.
 */
public interface NamedBytes {

    /**
     * Get a name.
     * @return a name as string
     */
    String getName();
    /**
     * Set a name.
     * @param name a name
     */
    void setName(String name);

    /**
     * Get a value of the object.
     * @return a value of the object
     */
    ByteBuffer getBytes();
    /**
     * Set a value into the object.
     * @param bytes a value
     */
    void setBytes(ByteBuffer bytes);

    /**
     * Get a data type of the object.
     * @return a data type
     */
    DataType getType();
    /**
     * Set a data type into the object.
     * @param type a data type
     */
    void setType(DataType type);

    /**
     * Compare the object to the specified object.
     * @param obj a compared object
     * @return return true if the name, the type and the value are equal to those of the object
     */
    @Override
    boolean equals(Object obj);
}
