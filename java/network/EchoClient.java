import java.net.Socket;
import java.net.ServerSocket;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.IOException;

public class EchoClient {

    public static final int ECHO_PORT = 10007;

    public static void main(String args[]) {
        Socket socket = null;
        try {
            socket = new Socket(args[0], ECHO_PORT);
            System.out.println("接続しました" + socket.getRemoteSocketAddress());
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader keyIn = new BufferedReader(new InputStreamReader(System.in));
            String input;
            while ( (input = keyIn.readLine()).length() > 0 ) {
                out.println(input);
                String line = in.readLine();
                if (line != null) {
                    System.out.println(line);
                } else {
                    break;
                }
            }
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
