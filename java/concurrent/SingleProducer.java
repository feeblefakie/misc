import java.util.Queue;

public class SingleProducer implements Runnable {

    private static int ctr;
    private final Queue<Message> messageQueue;
    private int numMessages;

    public SingleProducer(Queue<Message> messageQueue, int numMessages) {
        this.messageQueue = messageQueue;
        this.numMessages = numMessages;
    }

    @Override
    public void run() {
        for (int i = 0; i < numMessages; ++i) {
            Message m = new Message(++ctr, "Example message.");
            messageQueue.offer(m);
        }
        System.out.println("producer done.");
    }
}
