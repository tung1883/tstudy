package main

import "fmt" 

// Key guarantee: if event a happened before event b (i.e., a < b), then L(a) < L(b) with L(x)=lamport clock
type LamportClock struct {
	counter uint64
}

func NewLamportClock() *LamportClock{ 
	return &LamportClock{
		counter: 0,
	}
}

// Increment for a local event
func (lc *LamportClock) Tick() uint64 {
	lc.counter++
	return lc.counter
}

// Update clock when receiving a message
func (lc *LamportClock) Receive(timestamp uint64) uint64 {
	// Update rule: L(x) = max(L(x), t) + 1
	if timestamp > lc.counter {
		lc.counter = timestamp
	}

	lc.counter += 1
	return lc.counter
}

// Read current clock
func (c *LamportClock) Now() uint64 {
    return c.counter
}

type Message struct {
    From      string
    To        string
    Payload   string
    Timestamp uint64
}

type Process struct {
    ID    string
    Clock *LamportClock
}

func NewProcess(id string) *Process {
    return &Process{
        ID:    id,
        Clock: NewLamportClock(),
    }
}

// Execute an independent local event.
func (p *Process) LocalEvent(description string) {
    ts := p.Clock.Tick()

    fmt.Printf(
        "[%s] Local event: %s (L=%d)\n",
        p.ID, description, ts,
    )
}

// Send a message.
func (p *Process) Send(to, payload string) Message {
    ts := p.Clock.Tick()

    msg := Message{
        From:      p.ID,
        To:        to,
        Payload:   payload,
        Timestamp: ts,
    }

    fmt.Printf(
        "[%s] Send to %s: %s (L=%d)\n",
        p.ID, to, payload, ts,
    )

    return msg
}

// Receive a message.
func (p *Process) Receive(msg Message) {
    ts := p.Clock.Receive(msg.Timestamp)

    fmt.Printf(
        "[%s] Receive from %s: %s (L=%d)\n",
        p.ID, msg.From, msg.Payload, ts,
    )
}

func main() {
    a := NewProcess("A")
    b := NewProcess("B")

    a.LocalEvent("Initialize")
    b.LocalEvent("Initialize")

    msg := a.Send("B", "Hello B")
    b.Receive(msg)

    b.LocalEvent("Process message")

    reply := b.Send("A", "Hello A")
    a.Receive(reply)
}
