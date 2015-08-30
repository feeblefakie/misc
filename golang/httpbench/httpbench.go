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
	tmpfile := "/tmp/" + strconv.FormatInt(int64(os.Getegid()), 10) + ".err"
	efile := flag.String("error_to", tmpfile, "a file for error messages")
	concurrency := flag.Int("concurrency", 1, "concurrency")
	total := flag.Int64("total", 10000, "total")
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

	for i := 0; i < *concurrency; i++ {
		go httpRequest(lines, stats, *total, ef)
	}

	start := time.Now()
	for stats.doneRequests < *total {
		end := time.Now()
		interval := int64(end.Sub(start).Seconds())
		if interval != 0 {
			fmt.Printf("ok: %6d,  errors: %6d,  reqs/s: %6d,  accumLat: %6d, aveLat(ms): %6d \r",
				stats.numSucceeded, stats.numFailed, stats.numSucceeded/interval, stats.accumLatencies, stats.accumLatencies/interval/1000000)
		}
		time.Sleep(time.Millisecond * 100)
	}
}

func httpRequest(lines []string, stats *HttpStats, total int64, errorFile *os.File) {
	for {
		offset := atomic.AddInt64(&stats.doneRequests, 1)
		if int(offset) > len(lines) {
			break
		}
		line := lines[offset-1]
		//fmt.Println(line)

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
		//fmt.Println(resp)
		atomic.AddInt64(&stats.accumLatencies, interval)
		if err != nil {
			atomic.AddInt64(&stats.numFailed, 1)
			errorFile.WriteString(err.Error() + "\n")
		} else {
			atomic.AddInt64(&stats.numSucceeded, 1)
		}
		resp.Body.Close()
	}
}
