import org.apache.pulsar.client.api.BatcherBuilder;
import org.apache.pulsar.client.api.MessageRoutingMode;
import org.apache.pulsar.client.api.Producer;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;

public class MyProducer {
  public static void main(String[] args) throws PulsarClientException {
    PulsarClient client = PulsarClient.builder().serviceUrl("pulsar://localhost:6650").build();

    Producer<byte[]> producer =
        client
            .newProducer()
            .topic("persistent://my-tenant/my-namespace/my-topic8")
            .messageRoutingMode(MessageRoutingMode.SinglePartition)
            .batcherBuilder(BatcherBuilder.KEY_BASED)
            .create();

    for (int j = 0; j < 10; ++j) {
      for (int i = 0; i < 1000; ++i) {
        String value = Integer.toString(i);
        producer.newMessage().key(value).value(value.getBytes()).send();
      }
    }

    producer.close();
    client.close();
  }
}
