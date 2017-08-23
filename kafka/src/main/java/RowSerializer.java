import org.apache.kafka.common.errors.SerializationException;
import org.apache.kafka.common.serialization.Serializer;

import java.util.Map;

/**
 * Created by hiroyuki on 2017/08/23.
 */

public class RowSerializer implements Serializer<Row> {
    private boolean isKey;

    @Override
    public void configure(Map<String, ?> configs, boolean isKey) {
        this.isKey = isKey;
    }

    @Override
    public byte[] serialize(String topic, Row row) {
        if (row == null) { return null; }

        try {
            return TypeConverter.getByteArrayFrom(row);
        } catch (Exception e) {
            throw new SerializationException("Error serializing value", e);
        }
    }

    @Override
    public void close() {
    }
}

