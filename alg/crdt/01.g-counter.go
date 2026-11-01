package main

import "fmt"

type GCounter struct {
	// each index = 1 replica
	counts []uint64
}

// create a g-counter with n replicas
func newGCounter(n int) *GCounter {
	return &GCounter{ counts: make([]uint64, n), }
}

// increment the item owned by a replica
func (g *GCounter) increment(replicaID int) {
	g.counts[replicaID]++	
}

// calculate total value
func (g *GCounter) value() uint64 {
	var total uint64 = 0

	for _, count := range g.counts {
		total += count
	}

	return total
}

// merge = take max value of each replica
func (g *GCounter) merge(other *GCounter) {
	for i := range g.counts {
		if other.counts[i] > g.counts[i] {
			g.counts[i] = other.counts[i]
		}
	}
}

func main() {
	n := 3
	a := newGCounter(n)
	b := newGCounter(n)

	a.increment(0)
	a.increment(0)

	b.increment(1)

	fmt.Println("A: ", a.counts) // [2, 0, 0] 
	fmt.Println("B: ", b.counts) // [0, 1, 0]

	// merge step
	a.merge(b)
	b.merge(a)

	fmt.Println("A: ", a.counts) // [2, 1, 0] 
	fmt.Println("B: ", b.counts) // [2, 1, 0]
	fmt.Println("Total: ", a.value()) // 3 
}
