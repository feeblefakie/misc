package main

import (
    "fmt"
    "time"
    "os"
    "sync/atomic"
    "math/rand"
    "flag"
    "runtime"
)

func main() {

    filename := flag.String("filename", "", "file(device) name to be benchmarked")
    // random mode only for now
    mode := flag.String("mode", "random", "random/sequential")
    concurrency := flag.Int("concurrency", 1, "concurrency")
    flag.Parse()
    runtime.GOMAXPROCS(runtime.NumCPU())

    file, err := os.Open(*filename)
    if err != nil {
        fmt.Println(err)
        return
    }
    defer file.Close()

    stat, err := file.Stat()
    if err != nil {
        fmt.Println(err)
        return
    }

    if (*mode == "random") {
        randRead(file, stat, *concurrency)
    } else {
        seqRead(*concurrency)
    }
}

func randRead(file *os.File, stat os.FileInfo, concurrency int) {
    var doneBlocks int64 = 0
    for i := 0; i < concurrency; i++ {
        go readBlock(file, stat, &doneBlocks, i)
    }
    fmt.Println("reader goroutines created")

    start := time.Now()
    var input string
    fmt.Scanln(&input)
    for {
        //fmt.Print(doneBlocks)
        end := time.Now()
        //fmt.Printf("IOPS: %d %d\r", doneBlocks, (end.Sub(start).Nanoseconds()))
        interval := int64(end.Sub(start).Seconds())
        if (interval != 0) {
            fmt.Printf("#blocks: %d, IOPS: %d\r", doneBlocks, doneBlocks/interval)
        }
        time.Sleep(time.Millisecond * 100)
    }
}

func seqRead(concurrency int) {
    fmt.Println("hello")
}

func readBlock(file *os.File, stat os.FileInfo, doneBlocks *int64, threadID int) {
    block := make([]byte, 4096);
    numBlocks := stat.Size() / 4096 + 1
    for {
        blockID := rand.Int63() % numBlocks
        file.ReadAt(block, blockID * 4096)
        atomic.AddInt64(doneBlocks, 1)
    }
}
