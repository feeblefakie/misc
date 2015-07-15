import java.io.*;

public class BinSearchTest {
    public static void main(String []args) {
        try {
            RandomAccessFile file = new RandomAccessFile(args[0], "r");
            BinSearch bs = new BinSearch(file);
            int valIndex = bs.search(Integer.parseInt(args[1]));
            System.out.println("valIndex: " + valIndex);
        } catch (IOException e) {
        }
    }
}
