package main

import (
    "fmt"
    "os"
    "sync"
    //"runtime"
)

func main() {

    file, err := os.Open("test.txt")
    if err != nil {
        return
    }
    defer file.Close()

    stat, err := file.Stat()
    if err != nil {
        return
    }

    randRead(file, stat, 3)
}

func randRead(file *os.File, stat os.FileInfo, concurrency int) {
    fmt.Println(stat.Size())
    block := make([]byte, 4096);
    var wg sync.WaitGroup
    for {
        // using GOMAXPROCS is way easier to limit the number of os threads
        // runtime.GOMAXPROCS(cpus)
        for i := 0; i < concurrency; i++ {
            // get random offset
            go randReadImpl(file, block, int64(i), &wg)
            wg.Add(1)
        }
        fmt.Println("waiting")
        wg.Wait()
    }
}

func randReadImpl(file *os.File, block []byte, off int64, wg *sync.WaitGroup) {
    fmt.Println(off)
    file.ReadAt(block, 0)
    wg.Done()
    return
}

