import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class ConcurrentQueueSPMCTest {
    public static void main(String[] args) {
        if (args.length != 3) {
            System.err.println("ConcurrentQueueTest numMessages numConsumers 1/0(1:wait-free,0:lock)");
            System.exit(1);
        }
        int numMessages = Integer.parseInt(args[0]);
        int numConsumers = Integer.parseInt(args[1]);
        int waitFree = Integer.parseInt(args[2]);

        AtomicInteger ai = new AtomicInteger();
        Queue<Message> queue;
        if (waitFree == 1) {
            queue = new ConcurrentLinkedQueue<Message>();
        } else {
            queue = new LinkedBlockingQueue<Message>();
        }
        SingleProducer p = new SingleProducer(queue, numMessages);
        Thread tp = new Thread(p);
        tp.start();

        Thread[] thread = new Thread[numConsumers];
        for (int i = 0; i < numConsumers; ++i) {
            thread[i] = new Thread(new MultiConsumer(queue, ai, numMessages));
            thread[i].start();
        }

        try {
            tp.join();
            for (int i = 0; i < numConsumers; ++i) {
                thread[i].join();
            }
        } catch (InterruptedException e) {
            System.err.println(e.getMessage());
        }
    }
}
