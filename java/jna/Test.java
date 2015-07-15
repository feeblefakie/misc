import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
 
public class Test {
    public interface CLibrary extends Library {
        // loading libtest.so
        //CLibrary INSTANCE = (CLibrary) Native.loadLibrary("test", CLibrary.class);
        // loading libtext_cxx.so (lookup method is enclosed by extern "C" { } for name mangling.)
        CLibrary INSTANCE = (CLibrary) Native.loadLibrary("test_cxx", CLibrary.class);
        int lookup(int a);
    }

    public static void main(String[] args) {
        long start = System.currentTimeMillis();
        for (int i = 0; i < 1000; ++i) {
            CLibrary.INSTANCE.lookup(20);
        }
        long end = System.currentTimeMillis();
        System.err.println("Total time(s): " + (end - start) / 1000.0);
    }
}
