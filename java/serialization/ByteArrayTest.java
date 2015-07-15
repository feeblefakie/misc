import db.hadooode.*;
import java.util.List;
import java.util.ArrayList;
import java.io.*;

public class ByteArrayTest implements Serializable {
    public static void main(String[] args) {
        MultiColumn mc = new MultiColumn();
        mc.addInteger(new Integer(10));
        mc.addString(new String("hello world"));
        mc.addDouble(new Double(10.111));
        List<Integer> columnIDs = new ArrayList<Integer>();
        columnIDs.add(new Integer(1));
        columnIDs.add(new Integer(4));
        RequestMessage m = new RequestMessage(new String("index"), mc, columnIDs);
        System.out.println(m);

        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            ObjectOutputStream oos = new ObjectOutputStream(baos);
            oos.writeObject(m);
            oos.flush();
            byte[] bytes = baos.toByteArray();
            System.out.println("size: " + bytes.length);
            oos.close();
            baos.close();

            ByteArrayInputStream bais = new ByteArrayInputStream(bytes);
            ObjectInputStream ois = new ObjectInputStream(bais);
            RequestMessage data = (RequestMessage) ois.readObject();
            System.out.println(data);
            ois.close();
            bais.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}


