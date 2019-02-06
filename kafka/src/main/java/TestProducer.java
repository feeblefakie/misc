import java.nio.ByteBuffer;
import java.util.concurrent.ExecutionException;

/** Created by hiroyuki on 2017/08/15. */
public class TestProducer {
  //private static final String MESSAGE = "xxx";
  private static final String MESSAGE =
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
          + "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

  public static void main(String[] args) throws ExecutionException, InterruptedException {
    SampleProducer producer = SampleProducer.getInstance();

    long start = System.currentTimeMillis();
    int total = 30000;
    for (int i = 0; i < total; i++) {
      String key = Integer.toString(i);
      String value = Integer.toString(i * 10000) + " " + MESSAGE;

      producer.send(key, value);

      /*
      Row row = new Row();
      boolean b = true;
      row.put(new Column("c1", (ByteBuffer) ByteBuffer.allocate(4).putInt(i).flip(), DataType.INT));
      row.put(new Column("c2", (ByteBuffer) ByteBuffer.allocate("test".length()).put("test".getBytes()).flip(), DataType.STRING));
      row.put(new Column("c3", (ByteBuffer) ByteBuffer.allocate(4).putInt(i*2).flip(), DataType.INT));
      row.put(new Column("c4", (ByteBuffer) ByteBuffer.allocate(4).putInt(i*3).flip(), DataType.INT));
      row.put(new Column("c5", (ByteBuffer) ByteBuffer.allocate(1).put((byte) (b ? 1 : 0)).flip(), DataType.BOOLEAN));

      producer.send(key, row);
      */
    }
    producer.flush();
    long end = System.currentTimeMillis();
    System.out.println(total * 1000 / (end - start) + " msgs/s");

    producer.close();
  }
}
