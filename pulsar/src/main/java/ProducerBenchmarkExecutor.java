import java.util.concurrent.Callable;
import org.apache.pulsar.client.api.PulsarClientException;
import picocli.CommandLine;
import picocli.CommandLine.Option;

@CommandLine.Command(name = "producer-benchmark", description = "Run a producer benchmark")
public class ProducerBenchmarkExecutor implements Callable<Void> {

  @Option(
      names = "--service-url",
      required = true,
      description = "Pulsar service url to connect to (pulsar://ip:port)")
  private String serviceUrl;

  @Option(
      names = "--topic",
      defaultValue = "my-topic",
      description = "the topic to use (default: ${DEFAULT-VALUE})")
  private String topic;

  @Option(
      names = "--record-size",
      defaultValue = "100",
      description = "the size of the record to send (default: ${DEFAULT-VALUE})")
  private int recordSize;

  @Option(
      names = "--num-records",
      defaultValue = "10000000",
      description = "the number of records to send (default: ${DEFAULT-VALUE})")
  private int numRecords;

  @Option(
      names = "--threads",
      defaultValue = "1",
      description = "the number of threads to use (default: ${DEFAULT-VALUE})")
  private int numThreads;

  @Option(
      names = "--reporting-interval",
      defaultValue = "2000",
      description = "the reporting interval in milliseconds (default: ${DEFAULT-VALUE})")
  private int reportingInterval;

  @Option(
      names = "--runtime",
      defaultValue = "10000",
      description = "the runtime in milliseconds (default: ${DEFAULT-VALUE})")
  private int runtime;

  @Option(names = "--help", usageHelp = true, description = "display this help and exit")
  boolean help;

  public static void main(String... args) {
    int exitCode = new CommandLine(new ProducerBenchmarkExecutor()).execute(args);
    System.exit(exitCode);
  }

  @Override
  public Void call() throws PulsarClientException {
    PulsarProducerBenchmark benchmark =
        new PulsarProducerBenchmark(
            serviceUrl,
            topic,
            recordSize,
            numRecords,
            numThreads,
            reportingInterval,
            runtime);
    benchmark.sendRecords();

    return null;
  }
}
