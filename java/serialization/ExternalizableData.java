import java.util.List;
import java.util.ArrayList;
import java.io.*;

public class ExternalizableData implements Externalizable {
    private List<Object> list = null;
    private int numList = 0;

    public ExternalizableData() {
        list = new ArrayList<Object>();
    }

    public void set() {
        list.add(new Integer(123));
        list.add(new String("hello world"));
        list.add(new Double(1.34));
    }

    public void get() {
        for (Object o : list) {
            System.out.println(o);
        }
    }

    @Override
    public void writeExternal(ObjectOutput out) throws IOException {
        out.writeInt(list.size());
        for (Object o : list) {
            if (o instanceof Integer) {
                out.writeInt(((Integer) o).intValue());
                System.out.println("writeInt");
            } else if (o instanceof Double) {
                out.writeDouble(((Double) o).doubleValue());
                System.out.println("writeDouble");
            } else if (o instanceof String) {
                out.writeChars((String) o);
                System.out.println("writeChars");
            } else {
                System.err.println("Not supported format");
            }
        }
    }
    @Override
    public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
        System.out.println("hello readExternal");
        numList = in.readInt();
        System.out.println("numList: " + numList);
        try {
        for (int i = 0; i < numList; ++i) {
            Object o = in.readObject();
            System.out.println(o);
            System.out.println("list.add");
        }
        } catch (Exception e) {
            System.out.println("error hello");
            System.err.println(e.getMessage());
        }
    }

    public static void main(String[] args) {
        ExternalizableData sd = new ExternalizableData();
        sd.set();
        sd.get();

        try {
            FileOutputStream fos = new FileOutputStream("EData.dat");
            ObjectOutputStream oos = new ObjectOutputStream(fos);
            oos.writeObject(sd);
            oos.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }

        try {
            FileInputStream fis = new FileInputStream("EData.dat");
            ObjectInputStream ois = new ObjectInputStream(fis);
            ExternalizableData data = (ExternalizableData) ois.readObject();
            data.get();
            ois.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
        }
    }
}
