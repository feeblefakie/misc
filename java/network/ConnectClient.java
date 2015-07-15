import java.net.Socket;
import java.net.ServerSocket;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.IOException;

public class ConnectClient {

    public static final int ECHO_PORT = 10007;

    public static void main(String args[]) {
        Socket socket = null;
        try {
            double t1 = System.currentTimeMillis();
            socket = new Socket(args[0], ECHO_PORT);
            System.out.println("接続しました" + socket.getRemoteSocketAddress());
            socket.close();
            socket = null;
            double t2 = System.currentTimeMillis();
            System.out.println("connection time: " + (t2-t1));
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (socket != null) {
                socket.close();
                }
            } catch (IOException e) {}
                System.out.println("切断されました " + socket.getRemoteSocketAddress());
        }
    }

}
