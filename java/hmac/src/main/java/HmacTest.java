import com.google.common.hash.HashFunction;
import com.google.common.hash.Hashing;
import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

public class HmacTest {
  private static final String algo = "HMacSHA256";

  public static void main(String[] args) throws NoSuchAlgorithmException, InvalidKeyException {
    String valueToDigest =
        ""
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "The quick brown fox jumps over the lazy dog"
            + "";
    byte[] key = "secret".getBytes();
    int num = 100000;

    // Use javax.crypto.Mac directly
    SecretKeySpec keySpec = new SecretKeySpec(key, algo);
    long start = System.nanoTime();
    for (int i = 0; i < num; ++i) {
      Mac mac = Mac.getInstance(algo);
      mac.init(keySpec);
      byte[] signature = mac.doFinal(valueToDigest.getBytes(StandardCharsets.UTF_8));
    }
    long end = System.nanoTime();
    System.out.println(end - start + " ns for " + num + " records.");
    System.out.println((double) (end - start) / num + " ns/hashing");

    // Use Guava (It uses javax.crypto.Mac internally)
    HashFunction hash = Hashing.hmacSha256(key);
    start = System.nanoTime();
    for (int i = 0; i < num; ++i) {
      byte[] signature =
          hash.newHasher().putString(valueToDigest, StandardCharsets.UTF_8).hash().asBytes();
    }
    end = System.nanoTime();
    System.out.println(end - start + " ns for " + num + " records.");
    System.out.println((double) (end - start) / num + " ns/hashing");

  }
}
