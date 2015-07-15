import java.beans.XMLEncoder;
import java.beans.XMLDecoder;
import java.io.*;
import java.util.*;

public class XMLTest {
    public static void main(String[] args) {
        List<B> bs = new ArrayList<B>();
        B b1 = new B();
        b1.setName("hello");
        B b2 = new B();
        b2.setName("world");
        bs.add(b1);
        bs.add(b2);
        A a = new A();
        a.setBs(bs);

        try {
            long start = System.currentTimeMillis();
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            XMLEncoder encoder = new XMLEncoder(baos);
            encoder.writeObject(a);
            encoder.close();
            String xml = baos.toString();
            System.out.println(xml);
            long end = System.currentTimeMillis();
            System.out.println("serialization time(s): " + (end - start) / 1000.0);

            start = System.currentTimeMillis();
            ByteArrayInputStream bais = new ByteArrayInputStream(xml.getBytes());
            XMLDecoder decoder = new XMLDecoder(bais);
            A tmp = (A) decoder.readObject();
            end = System.currentTimeMillis();
            System.out.println("deserialization time(s): " + (end - start) / 1000.0);
            List<B> blist = tmp.getBs();
            for (B b : blist) {
                System.out.println(b.getName());
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
