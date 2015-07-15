import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class ConcurrentQueueMPSCTest {
    public static void main(String[] args) {
        if (args.length != 3) {
            System.err.println("ConcurrentQueueMPSCTest numMessages numProducers 1/0(1:wait-free,0:lock)");
            System.exit(1);
        }
        int numMessages = Integer.parseInt(args[0]);
        int numProducers = Integer.parseInt(args[1]);
        int waitFree = Integer.parseInt(args[2]);

        AtomicInteger ai = new AtomicInteger();
        Queue<Message> queue;
        if (waitFree == 1) {
            queue = new ConcurrentLinkedQueue<Message>();
        } else {
            queue = new LinkedBlockingQueue<Message>();
        }
        int numMessagesPerProducer = numMessages / numProducers;
        Thread[] thread = new Thread[numProducers];
        for (int i = 0; i < numProducers; ++i) {
            thread[i] = new Thread(new SingleProducer(queue, numMessagesPerProducer));
            thread[i].start();
        }
        SingleConsumer c = new SingleConsumer(queue, numMessages);
        Thread tc = new Thread(c);
        tc.start();


        try {
            for (int i = 0; i < numProducers; ++i) {
                thread[i].join();
            }
            tc.join();
        } catch (InterruptedException e) {
            System.err.println(e.getMessage());
        }
    }
}
