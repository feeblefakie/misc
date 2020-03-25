import java.util.concurrent.TimeUnit;
import org.apache.pulsar.client.api.ConsumerBuilder;
import org.apache.pulsar.client.api.MessageListener;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;
import org.apache.pulsar.client.api.SubscriptionType;

public class MyConsumer {
  private static final int NUM_CONSUMERS = 20;
  private static String subscriptionName = "subscription-1";

  public static void main(String[] args) throws PulsarClientException {
    if (args.length >= 1) {
      subscriptionName = args[0];
    }
    System.err.println(subscriptionName);

    PulsarClient client =
        PulsarClient.builder()
            .serviceUrl("pulsar://localhost:6650")
            .listenerThreads(NUM_CONSUMERS)
            .build();

    MessageListener<byte[]> listener =
        (consumer, msg) -> {
          printSync(consumer.getConsumerName() + " " + msg.getKey());
          consumer.acknowledgeAsync(msg);
        };

    ConsumerBuilder<byte[]> builder =
        client
            .newConsumer()
            .topic("persistent://my-tenant/my-namespace/my-topic8")
            .subscriptionName(subscriptionName)
            .subscriptionType(SubscriptionType.Key_Shared)
            .messageListener(listener)
            .negativeAckRedeliveryDelay(1, TimeUnit.SECONDS);

    for (int i = 0; i < NUM_CONSUMERS; ++i) {
      builder.subscribe();
    }

    while (true) {
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        Thread.interrupted();
        e.printStackTrace();
      }
    }
  }

  private static synchronized void printSync(String str) {
    System.out.println(str);
  }
}
