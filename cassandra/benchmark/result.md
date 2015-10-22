# Initial Benchmark Results for Cassandra

## sequential batch insert

### settings

- 1000 records at a time with batch mode
- single threaded

### results

| # of nodes | throughput (records/s) | more details ? |
|:-----------|------------:|:------------:|
| 4 | 7658 | 13081 (s) for about 100 million records |
| 17 | 7223 | 58942 (s) for about 426 million records |


### comment

It is a sequential insert so it doesn't make big difference between 4nodes and 17nodes.
If it is a parallel insertion, it will be make a difference, but making it **TODO** and skipping the benchmarking for now.
Perferably throughput is linear to the number of nodes in that case.


## lookups

### settings

- concurrent_reads=32
- random read workloads 

### results

#### 4 nodes

| request parallelism (threads) | throughput (records/s) | more details ? |
|:-----------|------------:|:------------:|
| 1 | 221.6 | 1/221.6 = 0.0045 (s) for latency |
| 10 | 1286.2 |  |
| 50 | 2235.1 |  |
| 100 | 2382.8 |  |

#### 17 nodes

| request parallelism | throughput (records/s) | more details ? |
|:-----------|------------:|:------------:|
| 1 | 152.0 | 1/152.0 = 0.0066 for latency |
| 10 | 1166.1 |  |
| 50 | 2683.0 |  |
| 100 | 3265.7 |  |

**TODO**: graphs ?

### comment

Thoughput with 1 parallelism , which means synchronous random lookups, is close to what I expected because the current normal SATA disk drives give 150-200 IOPS.
(But the result is only close to what I expected only if Cassandra lookup triggers one disk read, otherwise it is something wrong. (meaing Cassandra is somehow slow.)

Throughput is increased when we have more parallelism in the requests and it goes to a peak at around 100 parallelism.
Throughput in 17 nodes is abount 40% bigger than throughput in 4 nodes, so node scalability in throughput is bad for the test with the current settings. (it should be close to 17/4 times bigger.) 
I should look deeper into that in the near future. (**TODO**)
I increased the concurrent_reads setting but it made it worse.

Latency in 4 nodes is about 5 ms and it is close to the latency in the normak disk drives, so it is kind of close to what I expected.
(But the result is only close to what I expected only if Cassandra lookup triggers one disk read, otherwise it is something wrong. (meaing Cassandra is somehow slow.)
Latency in 17 nodes is 30 % slower than the one in 4 nodes. It is not very good so I have to look deeper into it in the near future. (**TODO**)


## full scan

### settings

- single threaded

### results

| # of nodes | throughput (records/s) | more details ? |
|:-----------|------------:|:------------:|
| 4 | 21896 | |
| 17 | 20519 | |


### comment

It is a sequential scan so it doesn't make big difference between 4nodes and 17nodes.
If it is a parallel scan, it will be make a difference, but I skip the benchmarking for now.
Perferably throughput is linear to the number of nodes in that case.
