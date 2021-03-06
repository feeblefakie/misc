import org.apache.kafka.clients.producer.*;

import java.util.Properties;
import java.util.concurrent.ExecutionException;

/**
 * Created by hiroyuki on 2017/08/15.
 */
public class SampleProducer {
    private static SampleProducer instance = null;
    private Producer<String, String> producer;

    private SampleProducer() {
        Properties props = new Properties();
        props.put("bootstrap.servers", "localhost:9092");
        props.put("acks", "all");
        props.put("retries", Integer.MAX_VALUE);
        props.put("batch.size", 16384);
        //props.put("linger.ms", 1000);
        //props.put("linger.ms", 5);
        //props.put("buffer.memory", 33554432);
        props.put("key.serializer", "org.apache.kafka.common.serialization.StringSerializer");
        props.put("value.serializer", "org.apache.kafka.common.serialization.StringSerializer");
        //props.put("value.serializer", "RowSerializer");

        producer = new KafkaProducer<String, String>(props);
    }

    public static synchronized SampleProducer getInstance() {
        if (instance == null) {
            instance = new SampleProducer();
        }
        return instance;
    }

    public void send(String key, String value) throws ExecutionException, InterruptedException {
        //producer.send(new ProducerRecord<>("my-replicated-topic", key, value)).get();
        producer.send(new ProducerRecord<>("my-replicated-topic", key, value),
                new Callback() {
                    public void onCompletion(RecordMetadata metadata, Exception e) {
                        if (e != null) {
                            e.printStackTrace();
                            System.err.println("Failed: " + metadata.offset());
                        } else {
                            //System.out.println("The offset of the record we just sent is: " + metadata.offset());
                        }
                    }
                });
    }

    /*
    public void send(String key, Row row) {
        //producer.send(new ProducerRecord<>("my-topic", key, value));
        producer.send(new ProducerRecord<String, Row>("my-topic", key, row),
                new Callback() {
                    public void onCompletion(RecordMetadata metadata, Exception e) {
                        if (e != null) {
                            e.printStackTrace();
                            System.err.println("Failed: " + metadata.offset());
                        } else {
                            System.out.println("The offset of the record we just sent is: " + metadata.offset());
                        }
                    }
                });
    }
    */

    public void flush() {
        producer.flush();
    }

    public void close() {
        producer.close();
    }
}
