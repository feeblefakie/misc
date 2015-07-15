package db.hadooode;

import java.io.BufferedReader;
import java.io.FileReader;
import java.security.MessageDigest;

public class HashTest {
    private static MessageDigest md = null;
    public static void main(String[] args) throws Exception {
        md = MessageDigest.getInstance("MD5");
        BufferedReader reader = new BufferedReader(new FileReader(args[0]));
        while (true) {
            String line = reader.readLine();
            if (line == null) {
                break;
            }
            String[] cols = line.split("\\|");
            long hashValue = hashing2(Integer.parseInt(cols[0]), Integer.parseInt(cols[3]));
            //System.out.println(hashValue % 16);
        }
    }
    private static byte[] toBytes(int a) {
        byte[] bs = new byte[4];
        bs[3] = (byte) (0x000000ff & (a));
        bs[2] = (byte) (0x000000ff & (a >>> 8));
        bs[1] = (byte) (0x000000ff & (a >>> 16));
        bs[0] = (byte) (0x000000ff & (a >>> 24));
        return bs;
    }

    public static long hashing5(int a, int b) throws Exception {
         byte[] bytes = toBytes(a);
         md.update(bytes, 0, bytes.length);
         byte[] digest = md.digest();
         md.reset();
         StringBuffer buffer = new StringBuffer();
         for(int i = digest.length-2; i< digest.length; i++){
            String tmp=Integer.toHexString(digest[i] & 0xff);
            if (tmp.length() == 1) {
                buffer.append('0').append(tmp);
            }else{
                buffer.append(tmp);
            }
        }
        //System.out.println(Integer.parseInt(buffer.toString(), 16));
        return Integer.parseInt(buffer.toString(), 16);
    }

    public static long hashing2(int a, int b) {
        long result = 17;
        result = 37 * result + a;
        result = 37 * result + b;
        return result;
    }

    public static long hashing4(int a, int b) {
        long hash = 1315423911;
        hash ^= ((hash << 5) + a + (hash >> 2));
        //hash ^= ((hash << 5) + b + (hash >> 2));
        return hash;
    }

    public static long hashing3(int v1, int v2) {
        int b = 378551;
        int a = 63689;
        long hash = 0;

        hash = hash * a + v1;
        a    = a * b;
        hash = hash * a + v2;
        return hash;
    }

    public static long hashing(int a, int b) {
        long hash = 0;
        for (int j = 0; j < 4; j++) {
            int ch = a >> j*8;
            hash = ((hash << 5) ^ (hash >> 27)) ^ (char) ch;
        }
        for (int j = 0; j < 4; j++) {
            int ch = b >> j*8;
            hash = ((hash << 5) ^ (hash >> 27)) ^ (char) ch;
        }
        return hash;
    }
}
