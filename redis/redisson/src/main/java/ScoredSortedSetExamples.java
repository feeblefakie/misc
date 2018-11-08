import javafx.concurrent.Task;
import org.redisson.Redisson;
import org.redisson.api.RBucket;
import org.redisson.api.RLock;
import org.redisson.api.RScoredSortedSet;
import org.redisson.api.RedissonClient;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class ScoredSortedSetExamples {

  public static void main(String[] args) {
    // connects to 127.0.0.1:6379 by default
    RedissonClient redisson = Redisson.create();
    ExecutorService service = Executors.newFixedThreadPool(3);

    double start = System.currentTimeMillis();
    int num = 3;
    String id = UUID.randomUUID().toString();
    //String id = Double.toString(System.currentTimeMillis());
    List<Future<String>> futures = new ArrayList<>();
    for (int i = 0; i < num; ++i) {
      //exec(redisson, id, i);
      futures.add(exec2(service, redisson, id, i));
    }
    for (Future<String> f : futures) {
      try {
        f.get();
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }
    double end = System.currentTimeMillis();
    System.out.println((end - start) * 1000 / num);

    service.shutdown();
    redisson.shutdown();
  }

  private static Future<String> exec2(ExecutorService service, RedissonClient redisson, String id, int age) {
    return (Future<String>) service.submit(new Runnable() {
      @Override
      public void run() {
        System.out.println("calling exec");
        exec(redisson, id, age);
      }
    });
  }

  private static void exec(RedissonClient redisson, String id, int age) {
    RLock lock = redisson.getFairLock(id); // assetId
    lock.lock();
    System.out.println("lock for " + age + " is acquired.");

    RBucket<String> bucket = redisson.getBucket(id + "a"); // assetId
    String nonce = UUID.randomUUID().toString();
    bucket.set(nonce); // set nonce

    RScoredSortedSet<SomeBean> set = redisson.getScoredSortedSet(id + "1"); // id + some since it can't use the same key
    if (set.isExists() && !set.isEmpty()) {
      SomeBean latest = set.last();

      // actual exec with nonce
      //System.out.println(latest);
      try {
        Thread.sleep(3000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }

    // set age, hash, nonce for each assetId
    set.add(age, new SomeBean(id, age, id.getBytes()));

    // check if the returned nonce is the same as this
    if (bucket.get().equals(nonce)) {
      if (!bucket.delete()) {
        System.out.println("delete error");
      }
    }

    lock.unlock();
    System.out.println("lock for " + age + " is released.");
  }

}
