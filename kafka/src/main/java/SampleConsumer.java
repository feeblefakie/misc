import org.apache.kafka.clients.consumer.*;
import org.apache.kafka.common.TopicPartition;

import java.time.Duration;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Properties;

/** Created by hiroyuki on 2017/08/15. */
public class SampleConsumer {
  private Consumer<String, String> consumer;
  // private Consumer<String, Row> consumer;

  public SampleConsumer() {
    Properties props = new Properties();
    props.put("bootstrap.servers", "localhost:9092");
    props.put("group.id", "test");
    props.put("enable.auto.commit", "false");
    props.put("auto.offset.reset", "earliest");
    props.put("key.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
    props.put("value.deserializer", "org.apache.kafka.common.serialization.StringDeserializer");
    // props.put("value.deserializer", "RowDeserializer");

    consumer = new KafkaConsumer<>(props);
    consumer.subscribe(Arrays.asList("my-replicated-topic"));
    System.out.println("subscribe " + consumer.subscription());
    System.out.println("assignments " + consumer.assignment());
  }

  public void process() {
    // List<ConsumerRecord<String, String>> buffer = new ArrayList<>();
    try {
      /*
      TopicPartition tp = new TopicPartition("my-topic", 0);
      System.out.printf("last synced: %d\n", consumer.committed(tp).offset());
      */

      int count = 0;
      long start = System.currentTimeMillis();
      while (true) {
        ConsumerRecords<String, String> records = consumer.poll(Duration.ofMillis(Long.MAX_VALUE));
        // ConsumerRecords<String, Row> records = consumer.poll(Long.MAX_VALUE);
        /*
        if (records.partitions().size() != 1) {
            System.err.println("Error: multiple partitions for some reason");
        }
        */

        for (ConsumerRecord<String, String> record : records) {
          System.out.printf("offset = %d, key = %s, value = %s%n", record.offset(), record.key(), record.value());
          count++;
        }
        consumer.commitSync();
        long end = System.currentTimeMillis();
        System.out.println(count * 1000 / (end - start) + " msgs/s");

        //consumer.commitSync(Collections.singletonMap(last.partition(), new OffsetAndMetadata(last.offset() + 1)));

        /*
        for (TopicPartition partition : records.partitions()) {
          List<ConsumerRecord<String, String>> partitionRecords = records.records(partition);
          for (ConsumerRecord<String, String> record : partitionRecords) {
            System.out.println(record.offset() + ": " + record.value());
          }
          long lastOffset = partitionRecords.get(partitionRecords.size() - 1).offset();
          consumer.commitSync(
              Collections.singletonMap(partition, new OffsetAndMetadata(lastOffset + 1)));
        }
        */

        /*
        TopicPartition partition = records.partitions().iterator().next();
        if (consumer.committed(partition) == null) {
            System.out.printf("consumer.committed(partition) is null");
        } else {
            System.out.printf("last synced: %d\n", consumer.committed(partition).offset());
        }

        //List<ConsumerRecord<String, String>> partitionRecords = records.records(partition);
        List<ConsumerRecord<String, Row>> partitionRecords = records.records(partition);

        System.out.println("partition record size : " + partitionRecords.size());
        for (ConsumerRecord<String, Row> record : partitionRecords) {
            System.out.println(record.offset() + ": " + record.value());
        }
        long lastOffset;
        lastOffset = partitionRecords.get(partitionRecords.size() - 1).offset();
        consumer.commitSync(Collections.singletonMap(partition, new OffsetAndMetadata(lastOffset + 1)));
        System.out.printf("synced: %d\n", lastOffset + 1);
        */
      }

    } finally {
      close();
    }
  }

  public void close() {
    consumer.close();
  }
}
