package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"math"
	"net"
	"os"
)

type Message struct {
	Method string   `json:"method"`
	Number *float64 `json:"number"`
}
type PrimeResponse struct {
	Method string `json:"method"`
	Prime  bool   `json:"prime"`
}
type ErrorResponse struct {
	Error string `json:"error"`
}

func checkPrime(content float64) bool {
	n := math.Floor(content)
	g := int(n)
	if n != content {
		return false
	}
	if g < 2 {
		return false
	}

	for i := 2; i*i <= g; i++ {
		if g%i == 0 {
			return false
		}
	}
	return true
}

func handleConnection(conn net.Conn) {
	defer conn.Close()

	reader := bufio.NewReader(conn)
	encoder := json.NewEncoder(conn)
	for {
		line, err := reader.ReadBytes('\n')
		if err != nil {
			return
		}
		fmt.Printf("RaW: %s", line)

		var msg Message
		if err := json.Unmarshal(line, &msg); err != nil {
			encoder.Encode(ErrorResponse{Error: "malformed request"})
			return
		}

		if msg.Method != "isPrime" || msg.Number == nil {
			encoder.Encode(ErrorResponse{Error: "malformed request"})
			return
		}
		n := *msg.Number

		resp := PrimeResponse{
			Method: "isPrime",
			Prime:  checkPrime(n),
		}
		if err := encoder.Encode(resp); err != nil {
			return
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		log.Fatalf("usage: %s <port>", os.Args[0])
	}
	listener, err := net.Listen("tcp", ":"+os.Args[1])
	fmt.Println("server running on port :", os.Args[1])
	if err != nil {
		log.Fatal(err)
	}
	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Error Accepting Connection")
			continue
		}
		go handleConnection(conn)
	}
}
