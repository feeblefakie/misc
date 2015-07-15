import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
 
/** Simple example of native library declaration and usage. */
public class HelloWorld {
    public interface CLibrary extends Library {
        CLibrary INSTANCE = (CLibrary) Native.loadLibrary("c", CLibrary.class);
        void printf(String format, Object... args);
    }

    public static void main(String[] args) {
        for (int i = 0; i < 1000; ++i) {
            CLibrary.INSTANCE.printf("Hello, World\n");
        }
    }
}
