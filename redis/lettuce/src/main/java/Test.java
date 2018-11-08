import io.lettuce.core.Range;
import io.lettuce.core.RedisClient;
import io.lettuce.core.api.StatefulRedisConnection;
import io.lettuce.core.api.sync.RedisCommands;
import io.lettuce.core.api.sync.RedisStringCommands;

import java.util.List;

public class Test {
  public static void main(String[] args) {
    RedisClient client = RedisClient.create("redis://localhost");
    StatefulRedisConnection<String, String> connection = client.connect();
    RedisCommands sync = connection.sync();
    double start = System.currentTimeMillis();
    for (int i = 0; i < 100000; ++i) {
      sync.zadd("assetId", i, "{age:" + i + ",hash:'xxx'}");
      List<String> results = sync.zrevrange("assetId", 0, 0);
      //results.forEach(r -> System.out.println(r));
      Long value = sync.zcount("assetId", Range.from(Range.Boundary.unbounded(), Range.Boundary.unbounded()));
      //System.out.println(value);
      if (value.longValue() > 1) {
        sync.zpopmin("assetId");
      }
      //value = sync.zcount("assetId", Range.from(Range.Boundary.unbounded(), Range.Boundary.unbounded()));
      //System.out.println(value);
    }
    double end = System.currentTimeMillis();
    System.out.println((double) 100000 / (end - start) * 1000);
    //String value = (String) sync.get("a");
    //System.out.println(value);
  }
}
