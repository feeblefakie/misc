import java.util.Queue;
import java.util.concurrent.atomic.AtomicInteger;

public class SingleConsumer implements Runnable {
    private final Queue<Message> queue;
    private int numMessages;

    public SingleConsumer(Queue<Message> queue, int numMessages) {
        this.queue = queue;
        this.numMessages = numMessages;
    }

    @Override
    public void run() {
        int counter = 0;
        while (counter < numMessages) {
            Message m = queue.poll();
            if (m != null) {
                ++counter;
            }
        }
        System.out.println("consumer done.");
        return;
    }
}
