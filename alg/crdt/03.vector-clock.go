package main
import "fmt"

type Order int
const (
	lower Order = -1
	equal Order = 0
	greater Order = 1
	concurrent Order = 2
)

type VectorClock struct {
	counts []uint64
}

func newVectorClock(n int) *VectorClock {
	return &VectorClock{ counts: make([]uint64, n), }
}

func compare(a *VectorClock, b *VectorClock) Order {
	flag := equal
	
	for i := range a.counts {
		if a.counts[i] > b.counts[i] {
			if flag == lower {
				return concurrent
			}

			flag = greater
		} else if a.counts[i] < b.counts[i] {
			if flag == greater {
				return concurrent
			}

			flag = lower
		}
	}

	return flag
}

func main() {
    a := newVectorClock(3)
    b := newVectorClock(3)

    a.counts = []uint64{2, 1, 0}
    b.counts = []uint64{1, 1, 0}
    fmt.Println("a vs b:", compare(a, b)) // greater

    b.counts = []uint64{2, 1, 0}
    fmt.Println("a vs b:", compare(a, b)) // equal

    b.counts = []uint64{3, 1, 0}
    fmt.Println("a vs b:", compare(a, b)) // lower

    b.counts = []uint64{1, 1, 1}
    fmt.Println("a vs b:", compare(a, b)) // concurrent
}
