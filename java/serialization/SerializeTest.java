import db.hadooode.*;
import java.util.List;
import java.util.ArrayList;
import java.io.*;

public class SerializeTest implements Serializable {
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
            FileOutputStream fos = new FileOutputStream("SaveData.dat");
            ObjectOutputStream oos = new ObjectOutputStream(fos);
            oos.writeObject(m);
            oos.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        try {
            FileInputStream fis = new FileInputStream("SaveData.dat");
            ObjectInputStream ois = new ObjectInputStream(fis);
            RequestMessage data = (RequestMessage) ois.readObject();
            System.out.println(data);
            ois.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}


