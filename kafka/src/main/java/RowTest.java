import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by hiroyuki on 2017/08/17.
 */
public class RowTest {
    public static void main(String[] args) throws IOException, ClassNotFoundException {

        Row row = new Row();
        boolean b = true;
        row.put(new Column("c1", (ByteBuffer) ByteBuffer.allocate(4).putInt(1).flip(), DataType.INT));
        row.put(new Column("c2", (ByteBuffer) ByteBuffer.allocate("test".length()).put("test".getBytes()).flip(), DataType.STRING));
        row.put(new Column("c3", (ByteBuffer) ByteBuffer.allocate(4).putInt(11).flip(), DataType.INT));
        row.put(new Column("c4", (ByteBuffer) ByteBuffer.allocate(4).putInt(111).flip(), DataType.INT));
        row.put(new Column("c5", (ByteBuffer) ByteBuffer.allocate(1).put((byte) (b ? 1 : 0)).flip(), DataType.BOOLEAN));

        // conversion
        byte[] bytes = TypeConverter.getByteArrayFrom(row);
        Row row2 = TypeConverter.getRowFrom(bytes);

        if (row.equals(row2)) {
            System.out.println("the same row !!!");
        }
        System.out.println(row2);
    }
}
