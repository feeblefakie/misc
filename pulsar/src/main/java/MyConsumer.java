import org.apache.pulsar.client.api.Consumer;
import org.apache.pulsar.client.api.Message;
import org.apache.pulsar.client.api.Producer;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;
import org.apache.pulsar.client.api.SubscriptionType;

public class MyConsumer {
  public static void main(String[] args) throws PulsarClientException {
    PulsarClient client = PulsarClient.builder().serviceUrl("pulsar://localhost:6650").build();

    Consumer consumer =
        client
            .newConsumer()
            .topic("my-topic")
            .subscriptionName("sub1")
            //.subscriptionType(SubscriptionType.Failover)
            .subscribe();

    while (true) {
      // Wait for a message
      Message msg = consumer.receive();

      try {
        // Do something with the message
        //System.out.printf("Message received: %s\n", new String(msg.getData()));
        System.out.printf("Message received: %s\n", new String(msg.getData()));

        // Acknowledge the message so that it can be deleted by the message broker
        consumer.acknowledge(msg);
      } catch (Exception e) {
        // Message failed to process, redeliver later
        consumer.negativeAcknowledge(msg);
      }
    }
  }
}
