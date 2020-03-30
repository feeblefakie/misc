import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import org.apache.pulsar.client.api.MessageListener;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;
import org.apache.pulsar.client.api.SubscriptionType;

public class MyFailoverConsumer {
  public static void main(String[] args) throws PulsarClientException {
    if (args.length != 4) {
      System.err.println(
          "MyConsumer topic subscription partition-from(inclusive) partition-to(inclusive)");
    }
    String topic = args[0];
    String subscriptionName = args[1];
    int partitionFrom = Integer.parseInt(args[2]);
    int partitionTo = Integer.parseInt(args[3]);
    System.err.println(topic + " " + subscriptionName + " " + partitionFrom + "-" + partitionTo);

    List<String> topics = new ArrayList<>();
    for (int i = partitionFrom; i <= partitionTo; ++i) {
      topics.add(topic + "-partition-" + i);
    }
    System.err.println(topics);

    PulsarClient client =
        PulsarClient.builder()
            .serviceUrl("pulsar://localhost:6650")
            .listenerThreads(topics.size())
            .build();

    MessageListener<byte[]> listener =
        (consumer, msg) -> {
          synchronizedPrint(
              consumer.getConsumerName()
                  + " "
                  + msg.getKey()
                  + " "
                  + new String(msg.getValue())
                  + " "
                  + msg.getTopicName()
                  + " "
                  + Thread.currentThread().getName());
          consumer.acknowledgeAsync(msg);
        };

    client
        .newConsumer()
        .topics(topics)
        .subscriptionName(subscriptionName)
        .subscriptionType(SubscriptionType.Failover)
        .messageListener(listener)
        .negativeAckRedeliveryDelay(1, TimeUnit.SECONDS)
        .subscribe();

    while (true) {
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        Thread.interrupted();
        e.printStackTrace();
      }
    }
  }

  private static synchronized void synchronizedPrint(String str) {
    System.out.println(str);
  }
}
