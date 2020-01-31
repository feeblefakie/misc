import java.util.Properties;
import java.util.Random;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.LongAdder;
import org.apache.pulsar.client.api.Producer;
import org.apache.pulsar.client.api.PulsarClient;
import org.apache.pulsar.client.api.PulsarClientException;

public class PulsarProducerBenchmark {
  private PulsarClient client;
  private String topic;
  private int recordSize;
  private int numRecords;
  private int numThreads;
  private long start;
  private int reportingInterval; // ms
  private int runtime; // ms
  private LongAdder windowCount;
  private LongAdder windowLatency;
  private LongAdder totalCount;
  private LongAdder totalLatency;

  public PulsarProducerBenchmark(
      String serviceUrl,
      String topic,
      int recordSize,
      int numRecords,
      int numThreads,
      int reportingInterval,
      int runtime) throws PulsarClientException {
    this.topic = topic;
    this.recordSize = recordSize;
    this.numRecords = numRecords;
    this.numThreads = numThreads;
    this.reportingInterval = reportingInterval;
    this.runtime = runtime;
    this.windowCount = new LongAdder();
    this.windowLatency = new LongAdder();
    this.totalCount = new LongAdder();
    this.totalLatency = new LongAdder();
    this.client = PulsarClient.builder().serviceUrl(serviceUrl).build();
  }

  public void sendRecords() throws PulsarClientException {
    Producer<byte[]> producer = client.newProducer().topic(topic).create();

    start = System.currentTimeMillis();
    Random random = new Random();
    ExecutorService executor = Executors.newFixedThreadPool(numThreads);
    int recordsPerThread =
        numRecords / numThreads; // should I worry about this if it is not evenly divisible

    for (int i = 0; i < numThreads; i++) {
      executor.execute(
          () -> {
            int j = 0;
            while (true) {
              if (j++ >= recordsPerThread) {
                break;
              }
              long now = System.currentTimeMillis();
              if (now - start >= runtime) {
                break;
              }

              byte[] payload = new byte[recordSize];
              random.nextBytes(payload);
              String key = UUID.randomUUID().toString();

              try {
                long before = System.currentTimeMillis();
                //producer.newMessage().key(key).value(payload).send();
                producer.newMessage().key(key).value(payload).sendAsync().thenAccept(msgId -> {
                  long after = System.currentTimeMillis();

                  windowCount.increment();
                  windowLatency.add(after - before);
                  totalCount.increment();
                  totalLatency.add(after - before);
                });
                /*
                long after = System.currentTimeMillis();

                windowCount.increment();
                windowLatency.add(after - before);
                totalCount.increment();
                totalLatency.add(after - before);
                 */
              } catch (Exception e) {
                e.printStackTrace();
              }
            }
          });
    }

    executor.shutdown();
    while (!executor.isTerminated()) {
      try {
        newWindow();
        Thread.sleep(reportingInterval);
        printWindowStats();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }

    producer.close();
    printTotalStats();
  }

  private void newWindow() {
    windowCount.reset();
    windowLatency.reset();
  }

  private void printWindowStats() {
    long wc = windowCount.sum();
    long wl = windowLatency.sum();

    System.out.printf(
        "%d records sent, %.2f records/sec, %.2f ms avg latency, %d ms total latency\n",
        wc, (float) 1000 * wc / reportingInterval, (float) wl / wc, wl);
  }

  private void printTotalStats() {
    long tc = totalCount.sum();
    long tl = totalLatency.sum();

    System.out.println("=============== Overall Statistics ===============");
    System.out.printf(
        "%d records sent, %.2f records/sec, %.2f ms avg latency, %d ms total latency\n",
        tc, (float) 1000 * tc / (System.currentTimeMillis() - start), (float) tl / tc, tl);
  }
}
