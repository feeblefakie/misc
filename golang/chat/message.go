package main

import (
	"time"
)

type message struct {
	Name    string
	Email   string
	Message string
	When    time.Time
}
