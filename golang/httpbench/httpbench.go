package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"runtime"
	"strconv"
	"strings"
	"sync"
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
	file := flag.String("file", "", "a file which has list of URLs to be accessed")
	tmpfile := "/tmp/" + strconv.FormatInt(int64(os.Getpid()), 10) + ".err"
	efile := flag.String("error_to", tmpfile, "a file for error messages")
	concurrency := flag.Int("concurrency", 1, "concurrency")
	flag.Parse()

	if *file == "" {
		flag.Usage()
		return
	}

	f, err := os.Open(*file)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer f.Close()

	ef, err := os.Create(*efile)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer ef.Close()

	scanner := bufio.NewScanner(f)
	var lines []string
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}

	runtime.GOMAXPROCS(runtime.NumCPU())
	stats := &HttpStats{}

	mutex := new(sync.Mutex)
	for i := 0; i < *concurrency; i++ {
		go httpRequest(lines, stats, ef, mutex)
	}

	start := time.Now()
	for int(stats.doneRequests) < len(lines) {
		end := time.Now()
		interval := int64(end.Sub(start).Seconds())
		if interval != 0 {
			printStats(stats, interval)
		}
		time.Sleep(time.Millisecond * 100)
	}
	end := time.Now()
	interval := int64(end.Sub(start).Seconds())
	printStats(stats, interval)
}

func httpRequest(lines []string, stats *HttpStats, errorFile *os.File, mutex *sync.Mutex) {
	for {
		offset := atomic.AddInt64(&stats.doneRequests, 1)
		if int(offset) > len(lines) {
			break
		}
		line := lines[offset-1]

		// format: [Method URL BodyParameters(for POST)]
		items := strings.Split(line, " ")
		var body io.Reader
		if len(items) > 2 {
			body = strings.NewReader(items[2])
		}

		req, _ := http.NewRequest(items[0], items[1], body)
		client := new(http.Client)

		start := time.Now()
		resp, err := client.Do(req)
		end := time.Now()

		interval := int64(end.Sub(start).Nanoseconds())
		atomic.AddInt64(&stats.accumLatencies, interval)
		if err != nil {
			atomic.AddInt64(&stats.numFailed, 1)
			mutex.Lock()
			errorFile.WriteString(err.Error() + "\n")
			mutex.Unlock()
		} else {
			atomic.AddInt64(&stats.numSucceeded, 1)
		}
		resp.Body.Close()
	}
}

func printStats(stats *HttpStats, interval int64) {
	fmt.Printf("ok: %6d,  errors: %6d,  reqs/s: %6d,  aveLat(ms): %6d \r",
		stats.numSucceeded, stats.numFailed, stats.numSucceeded/interval, stats.accumLatencies/interval/1000000)
}
