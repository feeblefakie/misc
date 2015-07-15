import java.io.Serializable;
import java.util.List;
import java.util.ArrayList;
import java.io.*;
 
public class SerializableData implements Serializable {
    private static final long serialVersionUID = 6255752248513019027L;
    private List<Object> list = null;

    public SerializableData() {
        list = new ArrayList<Object>();
    }

    public void set() {
        list.add(new Integer(123));
        list.add(new String("hello world"));
        list.add(new String("hello world"));
        list.add(new String("hello world"));
        list.add(new String("hello world"));
        list.add(new String("hello world"));
        list.add(new String("hello world"));
        list.add(new String("hello world"));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
        list.add(new Double(1.34));
    }

    public void get() {
        for (Object o : list) {
            System.out.println(o);
        }
    }

    public static void main(String[] args) {
        SerializableData sd = new SerializableData();
        sd.set();
        sd.get();

        try {
            long start = System.currentTimeMillis();
            FileOutputStream fos = new FileOutputStream("SaveData.dat");
            ObjectOutputStream oos = new ObjectOutputStream(fos);
            oos.writeObject(sd);
            oos.close();
            long end = System.currentTimeMillis();
            System.out.println("serialization time(s): " + (end - start) / 1000.0);

        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        try {
            long start = System.currentTimeMillis();
            FileInputStream fis = new FileInputStream("SaveData.dat");
            ObjectInputStream ois = new ObjectInputStream(fis);
            SerializableData data = (SerializableData) ois.readObject();
            //data.get();
            ois.close();
            long end = System.currentTimeMillis();
            System.out.println("deserialization time(s): " + (end - start) / 1000.0);
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}


