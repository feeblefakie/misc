import java.net.Socket;
import java.net.ServerSocket;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.IOException;

public class EchoServer {

    public static final int ECHO_PORT = 10007;

    public static void main(String args[]) {
        ServerSocket serverSocket = null;
        Socket socket = null;
        try {
            serverSocket = new ServerSocket(ECHO_PORT);
            System.out.println("EchoServerが起動しました(port=" + serverSocket.getLocalPort() + ")");
            while (true) {
                socket = serverSocket.accept();
                System.out.println("接続されました " + socket.getRemoteSocketAddress() );
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                String line;
                while ( (line = in.readLine()) != null ) {
                    System.out.println("受信: " + line);
                    out.println(line);
                    System.out.println("送信: " + line);
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
                try {
                    if (serverSocket != null) {
                    serverSocket.close();
                }
            } catch (IOException e) {}
        }
    }

}
