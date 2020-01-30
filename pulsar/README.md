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
$ build/install/pulsar/bin/producer-benchmark-executor --topic=my-topic --runtime=30000 --service-url="pulsar://localhost:6650" --threads=128 --record-size=1024
```
