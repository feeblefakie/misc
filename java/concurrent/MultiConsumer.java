import java.util.Queue;
import java.util.concurrent.atomic.AtomicInteger;

public class MultiConsumer implements Runnable {
    private final Queue<Message> queue;
    private AtomicInteger ai;
    private int numMessages;

    public MultiConsumer(Queue<Message> queue, AtomicInteger ai, int numMessages) {
        this.queue = queue;
        this.ai = ai;
        this.numMessages = numMessages;
    }

    @Override
    public void run() {
        while (ai.get() != numMessages) {
            Message m = queue.poll();
            if (m != null) {
                //System.out.println("Consumer: " + m.toString());
                while (true) {
                    int current = ai.get();
                    int update = current + 1;
                    if (ai.weakCompareAndSet(current, update)) {
                        break;
                    }
                }
            }
        }
        System.out.println("consumer done.");
        return;
    }
}
