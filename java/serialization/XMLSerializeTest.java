import java.beans.XMLEncoder;
import java.beans.XMLDecoder;
import java.io.*;
import java.util.*;

public class XMLSerializeTest {
    public static void main(String[] args) {

        TestClass tc = new TestClass();
        List<Object> list = new ArrayList<Object>();
        list.add((Object) new Integer(10));
        list.add((Object) new Double(123.4));
        list.add((Object) new String("hello"));

        tc.setName(new String("world")); 
        tc.setList(list);

        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            XMLEncoder encoder = new XMLEncoder(baos);
            encoder.writeObject(tc);
            encoder.close();
            String xml = baos.toString();
            System.out.println(xml);

            ByteArrayInputStream bais = new ByteArrayInputStream(xml.getBytes());
            XMLDecoder decoder = new XMLDecoder(bais);
            TestClass tc2 = (TestClass) decoder.readObject();
            System.out.println(tc2.getName());
            System.out.println(tc2.getList());


        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
