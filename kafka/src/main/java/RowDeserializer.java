import org.apache.kafka.common.errors.SerializationException;
import org.apache.kafka.common.serialization.Deserializer;

import java.io.IOException;
import java.util.Map;

/**
 * Created by hiroyuki on 2017/08/23.
 */
public class RowDeserializer implements Deserializer<Row> {
    private boolean isKey;

    @Override
    public void configure(Map<String, ?> configs, boolean isKey) {
        this.isKey = isKey;
    }

    @Override
    public Row deserialize(String s, byte[] value) {
        if (value == null) {
            return null;
        }

        try {
            return TypeConverter.getRowFrom(value);
        } catch (ClassNotFoundException e) {
            //e.printStackTrace();
            throw new SerializationException("Error deserializing value", e);
        } catch (IOException e) {
            //e.printStackTrace();
            throw new SerializationException("Error deserializing value", e);
        }
    }

    @Override
    public void close() {

    }
}
