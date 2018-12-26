import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;
import java.io.StringReader;
import java.math.BigDecimal;

public class Test {
  public static void main(String[] args) {
    JsonObject json =
        Json.createObjectBuilder()
            .add("name", "Falco")
            .add("age", BigDecimal.valueOf(3))
            .add("bitable", Boolean.FALSE)
            .build();
    String result = json.toString();

    System.out.println(result);

    // Read back
    JsonReader jsonReader =
        Json.createReader(new StringReader("{\"name\" : \"Falco\",\"age\":3,\"bitable\":false}"));
    JsonObject jobj = jsonReader.readObject();
    System.out.println(jobj);
  }
}
