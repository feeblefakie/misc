import org.apache.pulsar.client.api.MessageRoutingMode;
import org.apache.pulsar.client.api.Producer;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;

public class MyProducer {
  public static void main(String[] args) throws PulsarClientException {
    PulsarClient client = PulsarClient.builder().serviceUrl("pulsar://localhost:6650").build();

    Producer<byte[]> producer = client.newProducer().topic("my-first-ptopic").create();

    producer.newMessage().key("hello").value("world".getBytes()).send();
    producer.newMessage().key("oh").value("my".getBytes()).send();
    System.out.println("HELL");
    client.close();
  }
}
