# OoODE Query Optimizer PoC

###Build

```
$ gradle assemble
$ gradle dist
```

###Use
```
$ java -cp "build/libs/calcite.jar:build/output/lib/*" ooode.TPCHQueryPlanner -sql "select l.quantity from tpch.customer c join tpch.index_orders2 o on c.custkey = o.custkey join tpch.lineitem l on o.orderkey = l.orderkey where c.custkey < 20000"
```

