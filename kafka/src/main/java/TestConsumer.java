/**
 * Created by hiroyuki on 2017/08/16.
 */
public class TestConsumer {
   public static void main(String[] args) {
      SampleConsumer consumer = new SampleConsumer();

      consumer.process();

      consumer.close();
   }
}
