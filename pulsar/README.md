# Pulsar

## Setup    

* Cluster 
```
$ bin/pulsar-admin clusters create my-cluster --url http://localhost:8080 --broker-url pulsar://localhost:6650
```
```
$ bin/pulsar-admin clusters get my-cluster
```

* Tenant

```
$ bin/pulsar-admin tenants create my-tenant
```

```
$ bin/pulsar-admin tenants get my-tenant
```


* Namespace

```
$ bin/pulsar-admin namespaces create my-tenant/my-namespace
```

* Topic

```
$ bin/pulsar-admin topics create-partitioned-topic persistent://my-tenant/my-namespace/my-topic --partitions 32
```

* Run a simple benchmark
```
$ ./gradlew installDist
$ build/install/pulsar/bin/producer-benchmark-executor --topic=my-topic --runtime=30000 --service-url="pulsar://localhost:6650" --threads=128 --record-size=1024
```

```
$ bin/pulsar-perf produce -threads 2 -u pulsar://localhost:6650 -o 2000 -p 100000  -n 1 -s 1000 -r 300000 -b 1 -z SNAPPY my-topic 

# better than my-topic3 (1000 partitions)
$ bin/pulsar-perf produce -threads 8 -u pulsar://localhost:6650 -o 2000 -p 100000  -n 1 -s 1000 -r 300000 -b 1 -z SNAPPY my-topic

$ bin/pulsar-perf produce -threads 8 -u pulsar://localhost:6650 -o 2000 -p 100000  -n 1 -s 1000 -r 300000 -b 1 -z SNAPPY my-topic3 
```
NOTE: 
- The number of partitions doesn't seem to matter 
- SNAPPY makes things really good
- Having more threads gives more throughput
- -o 2000 makes it unstable (-o 1000 seems more stable), but might be just a timing issue
- Max latency fluctuates but 99pct latency won't be too much



Example outputs:
```
$ bin/pulsar-perf produce -threads 4 -u pulsar://localhost:6650 -o 2000 -p 100000  -n 1 -s 1000 -r 300000 -b 1 -z SNAPPY  my-topic 

07:15:16.759 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 273434.0  msg/s ---   2086.1 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  17.717 ms - med:  17.053 - 95pct:  25.099 - 99pct:  29.691 - 99.9pct:  39.983 - 99.99pct:  44.065 - Max:  56.315
07:15:26.819 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 300473.1  msg/s ---   2292.4 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  15.604 ms - med:  15.114 - 95pct:  21.741 - 99pct:  28.905 - 99.9pct:  35.127 - 99.99pct:  39.994 - Max:  46.071
07:15:36.881 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 237873.4  msg/s ---   1814.8 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  26.740 ms - med:  16.355 - 95pct:  29.333 - 99pct:  37.992 - 99.9pct: 2184.495 - 99.99pct: 2186.447 - Max: 2201.055
07:15:46.913 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 311514.1  msg/s ---   2376.7 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  16.990 ms - med:  16.316 - 95pct:  24.039 - 99pct:  29.010 - 99.9pct:  49.773 - 99.99pct:  58.895 - Max:  59.452
```

```
$ bin/pulsar-perf produce -threads 8 -u pulsar://localhost:6650 -o 2000 -p 100000  -n 1 -s 1000 -r 500000 -b 1 -z SNAPPY  my-topic 

07:25:00.452 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 455980.7  msg/s ---   3478.9 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  20.533 ms - med:  19.623 - 95pct:  30.340 - 99pct:  36.155 - 99.9pct:  43.773 - 99.99pct:  49.371 - Max:  64.130
07:25:10.560 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 367875.4  msg/s ---   2806.6 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  34.143 ms - med:  20.382 - 95pct:  35.118 - 99pct:  64.477 - 99.9pct: 2765.615 - 99.99pct: 2771.807 - Max: 2772.799
07:25:20.607 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 545349.2  msg/s ---   4160.7 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  21.235 ms - med:  20.161 - 95pct:  31.622 - 99pct:  38.450 - 99.9pct:  59.892 - 99.99pct:  62.794 - Max:  94.955
07:25:30.646 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 409611.3  msg/s ---   3125.1 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  32.354 ms - med:  21.029 - 95pct:  33.684 - 99pct:  48.711 - 99.9pct: 2652.319 - 99.99pct: 2659.663 - Max: 2660.383
07:25:40.671 [main] INFO  org.apache.pulsar.testclient.PerformanceProducer - Throughput produced: 508319.0  msg/s ---   3878.2 Mbit/s --- failure      0.0 msg/s --- Latency: mean:  19.745 ms - med:  19.050 - 95pct:  28.083 - 99pct:  32.214 - 99.9pct:  37.712 - 99.99pct:  43.122 - Max:  54.484
```
