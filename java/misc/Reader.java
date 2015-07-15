import java.io.*;

public class Reader {
    public static void main(String[] args) {
        try {
            RandomAccessFile file = new RandomAccessFile("./tmp.txt", "r");
            file.seek(0);
            System.out.println("counter: " + file.readInt());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
