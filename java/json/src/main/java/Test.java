import io.github.resilience4j.core.IntervalFunction;
import io.github.resilience4j.retry.Retry;
import io.github.resilience4j.retry.RetryConfig;
import java.util.Arrays;

public class Test {

  public static void main(String[] args) {

    String csv = "aaa,bbb,ccc,ddd,eee";
    String replaced = csv.replace("ddd", "").replace("ccc", "");
    System.out.println(replaced);

    for (String id : replaced.split(",")) {
      if (!id.isEmpty()) {
        System.out.println(id);
      }
    }

    Arrays.stream(replaced.split(",")).filter(s -> !s.isEmpty()).forEach(System.out::println);


    String stripped = replaced.replaceAll(",+", ",");
    System.out.println(stripped);

    Retry retry = Retry.of(
        "retry",
        RetryConfig.custom()
            .maxAttempts(3)
            .intervalFunction(IntervalFunction.ofExponentialBackoff(1, 2.0))
            .retryOnResult(r -> !(boolean) r)
            .build());

    Retry.decorateSupplier(retry, Test::test).get();
  }

  private static boolean test() {
    System.out.println("TEST");
    return false;
  }
}
