package xodus;

import jetbrains.exodus.ByteIterable;
import jetbrains.exodus.bindings.IntegerBinding;
import jetbrains.exodus.bindings.StringBinding;
import jetbrains.exodus.env.Cursor;
import jetbrains.exodus.env.Environment;
import jetbrains.exodus.env.EnvironmentConfig;
import jetbrains.exodus.env.Environments;
import jetbrains.exodus.env.Store;
import jetbrains.exodus.env.StoreConfig;
import jetbrains.exodus.env.Transaction;

public class Test {
  public static void main(String[] args) {
    Environment env =
        Environments.newInstance(
            "/tmp/myAppData", new EnvironmentConfig().setLogDurableWrite(false));

    int numKeys = 100000;
    long t1 = System.currentTimeMillis();
    for (int i = 0; i < numKeys; ++i) {
      // final Transaction txn = env.beginTransaction();
      final Transaction txn = env.beginReadonlyTransaction();
      Store store = env.openStore("MyStore", StoreConfig.WITHOUT_DUPLICATES_WITH_PREFIXING, txn);
      try {
        // store.put(
        //    txn, IntegerBinding.intToEntry(i), StringBinding.stringToEntry("hello world " + i));
        // ByteIterable iterable = store.get(txn, IntegerBinding.intToEntry(i));
        // System.out.println(StringBinding.entryToString(iterable));

        ByteIterable from = IntegerBinding.intToEntry(100);
        ByteIterable to = IntegerBinding.intToEntry(150);
        try (Cursor cursor = store.openCursor(txn)) {
          ByteIterable current = cursor.getSearchKeyRange(from);
          if (current != null) {
            while (true) {
              System.out.println(StringBinding.entryToString(current));
              if (!cursor.getNext()) {
                break;
              }
              current = cursor.getValue();
              ByteIterable currentKey = cursor.getKey();
              if (currentKey.compareTo(to) > 0) {
                break;
              }
            }
          }
        }
x
        txn.commit();
      } catch (Exception e) {
        txn.abort();
      }
      break;
    }
    long t2 = System.currentTimeMillis();
    System.out.println(t2 - t1 + " milliseconds");
    System.out.println(numKeys * 1000 / (t2 - t1) + " tps");

    env.close();
  }
}
