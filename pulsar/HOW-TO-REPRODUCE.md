## How to reproduce an inconsistent order issue with KeyShared in pulsar-2.6.0

## Steps to reproduce

0. Start pulsar 2.6.0
   - Use standalone for quick testing
   - Try `AUTO_SPLIT` mode and consistent hashing
1. create a partitioned topic named
```
$ bin/pulsar-admin tenants create my-tenant
$ bin/pulsar-admin namespaces create my-tenant/my-namespace
$ bin/pulsar-admin topics create-partitioned-topic persistent://my-tenant/my-namespace/my-topic --partitions 32
```
2. Build the code to reproduce
```
$ git clone https://github.com/feeblefakie/misc.git
$ cd /misc/pulsar
$ ./gradlew installDist
```
3. Start a producer
```
$ build/install/pulsar/bin/my-producer persistent://my-tenant/my-namespace/my-topic
```
This produces 10000 messages in total like this; <"0", "timestamp">, <"1", "timestamp"> ... <"999", "timestamp">, <"0", "timestamp">, <"1", "timestamp">.

The code is located at https://github.com/feeblefakie/misc/blob/master/pulsar/src/main/java/MyProducer.java
.

4. Start consumers with "subscription1"
```
 $ build/install/pulsar/bin/my-consumer  persistent://my-tenant/my-namespace/my-topic subscription1 > /tmp/sub1
 ```
This starts 20 consumers one by one so the number of consumers are changing during consuming.

The code is located at https://github.com/feeblefakie/misc/blob/master/pulsar/src/main/java/MyConsumer.ava.
.

5. Start consumers with "subscription2"
```
 $ build/install/pulsar/bin/my-consumer  persistent://my-tenant/my-namespace/my-topic subscription2 > /tmp/sub2
```
6. Wait they consume all the messages
7. Compare results to see inconsistencies between consumers for the same key with different subscriptions
```
./check.sh
```
This will check all the keys (from 0 to 999) to see if the consumers consume the messsages in the same order.

## Results

- An inconsistent order is produced with the default `AUTO_SPLIT` mode.
- An inconsistent order is **NOT** produced with consistent hashing mode (enabled by setting `subscriptionKeySharedUseConsistentHashing` to `true`)
