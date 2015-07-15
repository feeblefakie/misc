import java.io.*;

public class Counter {
    public static void main(String[] args) {
        try {
            RandomAccessFile file = new RandomAccessFile("./tmp.txt", "rw");
            file.seek(0);
            file.writeInt(10);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
