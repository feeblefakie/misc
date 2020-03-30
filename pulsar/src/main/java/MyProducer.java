import org.apache.pulsar.client.api.HashingScheme;
import org.apache.pulsar.client.api.MessageRoutingMode;
import org.apache.pulsar.client.api.Producer;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;

public class MyProducer {
  public static void main(String[] args) throws PulsarClientException {
    if (args.length != 1) {
      System.err.println("MyProducer topic");
    }
    String topic = args[0];
    System.err.println(topic);

    PulsarClient client = PulsarClient.builder().serviceUrl("pulsar://localhost:6650").build();

    Producer<byte[]> producer =
        client
            .newProducer()
            .topic(topic)
            .messageRoutingMode(MessageRoutingMode.SinglePartition)
            // .batcherBuilder(BatcherBuilder.KEY_BASED)
            .hashingScheme(HashingScheme.Murmur3_32Hash)
            .create();

    for (int j = 0; j < 10; ++j) {
      for (int i = 0; i < 1000; ++i) {
        String key = Integer.toString(i);
        String value = new Long(System.currentTimeMillis()).toString();
        producer.newMessage().key(key).value(value.getBytes()).send();
      }
    }

    producer.close();
    client.close();
  }
}
