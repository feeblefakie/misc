package main

import (
	"flag"
	"fmt"
	"net/http"
	"runtime"
	"sync/atomic"
	"time"
)

type HttpStats struct {
	doneRequests   int64
	numSucceeded   int64
	numFailed      int64
	accumLatencies int64
}

func main() {

	url := flag.String("url", "", "url")
	// random mode only for now
	concurrency := flag.Int("concurrency", 1, "concurrency")
	total := flag.Int64("total", 10000, "total")
	flag.Parse()
	if *url == "" {
		flag.Usage()
		return
	}

	runtime.GOMAXPROCS(runtime.NumCPU())
	stats := HttpStats{0, 0, 0, 0.0}

	for i := 0; i < *concurrency; i++ {
		go httpRequest(url, &stats, *total)
	}

	start := time.Now()
	//var input string
	//fmt.Scanln(&input)
	for stats.doneRequests < *total {
		end := time.Now()
		interval := int64(end.Sub(start).Seconds())
		if interval != 0 {
			fmt.Printf("ok: %6d,  errors: %6d,  reqs/s: %6d,  accumLat: %6d, aveLat(ms): %6d \r", stats.numSucceeded, stats.numFailed, stats.numSucceeded/interval, stats.accumLatencies, stats.accumLatencies/interval/1000000)
		}
		time.Sleep(time.Millisecond * 100)
	}
}

func httpRequest(url *string, stats *HttpStats, total int64) {
	for {
		atomic.AddInt64(&stats.doneRequests, 1)

		req, _ := http.NewRequest("GET", *url, nil)
		client := new(http.Client)
		start := time.Now()
		resp, err := client.Do(req)
		end := time.Now()
		interval := int64(end.Sub(start).Nanoseconds())
		atomic.AddInt64(&stats.accumLatencies, interval)
		if err != nil {
			fmt.Println(err)
			atomic.AddInt64(&stats.numFailed, 1)
		} else {
			atomic.AddInt64(&stats.numSucceeded, 1)
		}
		//fmt.Println(err)
		//fmt.Println(resp)
		resp.Body.Close()
	}
}
