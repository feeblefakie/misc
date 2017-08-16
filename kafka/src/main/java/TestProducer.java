/**
 * Created by hiroyuki on 2017/08/15.
 */
public class TestProducer {
    public static void main(String[] args) {

        SampleProducer producer = SampleProducer.getInstance();

        for (int i = 0; i < 1000; i++) {
            String key = Integer.toString(i);
            String value = Integer.toString(i*10000);

            producer.send(key, value);
        }
        System.out.println("all sent");

        producer.flush();
        System.out.println("flushed");

        producer.close();
    }
}
