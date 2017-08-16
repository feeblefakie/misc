import org.apache.kafka.clients.consumer.*;
import org.apache.kafka.common.TopicPartition;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Properties;

/**
 * Created by hiroyuki on 2017/08/15.
 */
public class SampleConsumer {
    private Consumer<String, String> consumer;

    public SampleConsumer() {
        Properties props = new Properties();
        props.put("bootstrap.servers", "localhost:9092");
        props.put("group.id", "test");
        props.put("enable.auto.commit", "false");
        props.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
        props.put("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");

        consumer = new KafkaConsumer<>(props);
        consumer.subscribe(Arrays.asList("my-topic"));
        System.out.printf("subscribe " + consumer.subscription());
        System.out.printf("assignments " + consumer.assignment());
    }

    public void process() {
        //List<ConsumerRecord<String, String>> buffer = new ArrayList<>();
        try {
            while (true) {
                ConsumerRecords<String, String> records = consumer.poll(Long.MAX_VALUE);
                if (records.partitions().size() != 1) {
                    System.err.println("Error: multiple partitions for some reason");
                }
                TopicPartition partition = records.partitions().iterator().next();

                List<ConsumerRecord<String, String>> partitionRecords = records.records(partition);

                for (ConsumerRecord<String, String> record : partitionRecords) {
                    System.out.println(record.offset() + ": " + record.value());
                }
                long lastOffset;
                if (partitionRecords.size() > 1) {
                    lastOffset = partitionRecords.get(partitionRecords.size() - 2).offset(); // set one previous for test
                    System.out.printf("set one previous offset for test");
                } else {
                    lastOffset = partitionRecords.get(partitionRecords.size() - 1).offset();
                }
                consumer.commitSync(Collections.singletonMap(partition, new OffsetAndMetadata(lastOffset + 1)));
            }
        } finally {
            close();
        }
    }

    public void close() {
        consumer.close();
    }
}
