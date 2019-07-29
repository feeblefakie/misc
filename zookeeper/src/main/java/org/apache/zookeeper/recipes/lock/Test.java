package org.apache.zookeeper.recipes.lock;

import com.google.common.util.concurrent.Striped;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.Watcher;
import org.apache.zookeeper.ZooKeeper;

import java.io.IOException;
import java.util.Random;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class Test {
  private static ZooKeeper zk;
  private static AtomicInteger counter;

  public static void main(String[] args) throws IOException, KeeperException, InterruptedException {
    zk =
        new ZooKeeper(
            "localhost:2181",
            3000,
            event -> {
              if (event.getType() == Watcher.Event.EventType.NodeChildrenChanged) {
                System.out.println("NodeChildrenChanged");
              }
            });
    counter = new AtomicInteger();
    ExecutorService exec = Executors.newFixedThreadPool(100);

    double t1 = System.currentTimeMillis();
    int n = 10000;
    for (int i = 0; i < n; ++i) {
      Striped<Semaphore> striped = Striped.lazyWeakSemaphore(10000, 1);
      String key = Integer.toString(new Random().nextInt(10000));
      Semaphore semaphore = striped.get(key);
      semaphore.acquire();
      //ReadWriteLock lock = new ReentrantReadWriteLock();
      //lock.writeLock().lock();
      //WriteLock lock = lock();
      exec.submit(
          () -> {
            try {
              // some processing
              Thread.sleep(100);
              semaphore.release();
              //lock.writeLock().unlock();
              //lock.unlock();
              //lock.close();
            } catch (Exception e) {
              e.printStackTrace();
            }
          });
    }
    exec.shutdown();
    System.out.println("all are submitted.");
    exec.awaitTermination(Long.MAX_VALUE, TimeUnit.HOURS);

    double t2 = System.currentTimeMillis();
    double tps = n * 1000 / (t2 - t1);

    System.out.println("TPS: " + tps);
  }

  private static WriteLock lock() throws KeeperException, InterruptedException {
    CountDownLatch latch = new CountDownLatch(1);
    int key = new Random().nextInt(10000);
    //System.out.println(key + " is locked");
    WriteLock leader =
        new WriteLock(
            zk,
            "/hello/" + key,
            null,
            new LockListener() {
              @Override
              public void lockAcquired() {
                // System.out.println("acquired");
                latch.countDown();
              }

              @Override
              public void lockReleased() {
                // System.out.println("released");
              }
            });
    leader.lock();
    latch.await();
    return leader;
  }
}
