import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

public class SomeBean {
  private final String name;
  private final int number;
  private final byte[] bytes;

  @JsonCreator
  public SomeBean(@JsonProperty("name") String name, @JsonProperty("number") int number, @JsonProperty("bytes") byte[] bytes) {
    this.name = name;
    this.number = number;
    this.bytes = bytes;
  }

  public String getName() {
    return name;
  }

  public int getNumber() {
    return number;
  }

  public byte[] getBytes() {
    return bytes;
  }

  /*
  public String toString() {
    return name + number + bytes;
  }
  */
}
