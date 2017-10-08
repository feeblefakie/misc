import java.nio.ByteBuffer;

/**
 * Created by hiroyuki on 2017/08/15.
 */
public class TestProducer {
    public static void main(String[] args) {

        SampleProducer producer = SampleProducer.getInstance();

        for (int i = 0; i < 1; i++) {
            String key = Integer.toString(i);
            String value = Integer.toString(i*10000);

            //producer.send(key, value);

            Row row = new Row();
            boolean b = true;
            row.put(new Column("c1", (ByteBuffer) ByteBuffer.allocate(4).putInt(i).flip(), DataType.INT));
            row.put(new Column("c2", (ByteBuffer) ByteBuffer.allocate("test".length()).put("test".getBytes()).flip(), DataType.STRING));
            row.put(new Column("c3", (ByteBuffer) ByteBuffer.allocate(4).putInt(i*2).flip(), DataType.INT));
            row.put(new Column("c4", (ByteBuffer) ByteBuffer.allocate(4).putInt(i*3).flip(), DataType.INT));
            row.put(new Column("c5", (ByteBuffer) ByteBuffer.allocate(1).put((byte) (b ? 1 : 0)).flip(), DataType.BOOLEAN));

            producer.send(key, row);
        }
        System.out.println("all sent");

        producer.flush();
        System.out.println("flushed");

        producer.close();
    }
}
