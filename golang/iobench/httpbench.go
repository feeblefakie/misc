package main

import (
	"flag"
	"fmt"
	"net/http"
	"runtime"
	"sync"
	"time"
)

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
	m := new(sync.Mutex)
	var doneRequests int64 = 0

	for i := 0; i < *concurrency; i++ {
		go httpRequest(url, &doneRequests, *total, m)
	}

	start := time.Now()
	//var input string
	//fmt.Scanln(&input)
	for {
		end := time.Now()
		interval := int64(end.Sub(start).Seconds())
		if interval != 0 {
			fmt.Printf("total: %d, reqs/s: %d\r", doneRequests, doneRequests/interval)
		}
		time.Sleep(time.Millisecond * 100)
	}
}

func httpRequest(url *string, doneRequests *int64, total int64, m *sync.Mutex) {
	for {
		m.Lock()
		*doneRequests++
		if *doneRequests == total {
			return
		}
		m.Unlock()

		req, _ := http.NewRequest("GET", *url, nil)
		client := new(http.Client)
		resp, err := client.Do(req)
		if err != nil {
			fmt.Println(err)
		}
		//fmt.Println(err)
		//fmt.Println(resp)
		resp.Body.Close()
	}
}
