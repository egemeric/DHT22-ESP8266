package main

import (
	homewebserver "go-home/webserver"
	"log"
)

func init() {
	log.Printf("Go-Home DIY iot server")
}
func main() {
	homewebserver.StartServer()
}
