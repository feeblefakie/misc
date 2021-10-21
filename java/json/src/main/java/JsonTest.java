import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.google.common.base.Joiner;
import com.google.common.base.Splitter;
import com.jsoniter.JsonIterator;
import java.io.StringReader;
import java.util.Iterator;
import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonObjectBuilder;

public class JsonTest {
  private static final String KEY = "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk";
  private static final String VALUE = "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv";

  public static void main(String[] args) throws JsonProcessingException {

    int count = 10000;
    ObjectMapper mapper = new ObjectMapper();
    ObjectNode node = mapper.createObjectNode();
    node.put(KEY, VALUE);
    node.put(KEY, VALUE);
    node.put(KEY, VALUE);
    node.put(KEY, VALUE);
    node.put(KEY, VALUE);

    long start = System.nanoTime();
    String json = null;
    for (int i = 0; i < count; ++i) {
      json = mapper.writeValueAsString(node);
    }
    long end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for serialize (Jackson)");

    JsonObject jsonObject =
        Json.createObjectBuilder()
            .add(KEY, VALUE)
            .add(KEY, VALUE)
            .add(KEY, VALUE)
            .add(KEY, VALUE)
            .add(KEY, VALUE)
            .build();

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      json = jsonObject.toString();
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for serialize (JSONP)");

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      JsonNode jsonNode = mapper.readTree(json);
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deserialize (Jackson)");

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      JsonObject jsonObject2 = Json.createReader(new StringReader(json)).readObject();
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deserialize (JSONP)");

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      JSONObject jsonObject3 = JSON.parseObject(json);
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deserialize (fastjson)");

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      JsonIterator iterator = JsonIterator.parse(json);
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deserialize (json-iterator)");

    String joined = Joiner.on(",").join(KEY, VALUE, KEY, VALUE, KEY, VALUE, KEY, VALUE, KEY, VALUE);
    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      // String[] array = joined.split(",");
      Iterable<String> itr = Splitter.on(",").split(joined);
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deserialize (string)");

    String str =
        "{\"0\":{\"age\":0,\"data\":{\"customer_name\":\"Name 0\",\"checking_balance\":100000,\"savings_balance\":100000}},\"596\":{\"age\":0,\"data\":{\"customer_name\":\"Name 596\",\"checking_balance\":100000,\"savings_balance\":100000}}}";
    JsonNode jsonNode = mapper.readTree(str);
    for (Iterator<JsonNode> it = jsonNode.elements(); it.hasNext(); ) {
      JsonNode sub = it.next();
      System.out.println("toString" + sub.toString());
      System.out.println("mapper.writeValueAsString" + mapper.writeValueAsString(sub));
    }

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      ObjectNode copied = jsonNode.deepCopy();
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deepcopy (jackson)");

    start = System.nanoTime();
    for (int i = 0; i < count; ++i) {
      JsonObjectBuilder copied = Json.createObjectBuilder(jsonObject);
    }
    end = System.nanoTime();
    System.out.println((double) (end - start) / count + " ns/op for deepcopy (javax.json)");

    /*
    for (Entry<String, JsonValue> entry : proof.getInput().entrySet()) {
      String id = entry.getKey();
      int age = entry.getValue().asJsonObject().getInt(AssetRecord.AGE);
      keys.put(id, new Key(id, age));
    }
     */
  }
}
