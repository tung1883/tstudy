package main
import "fmt"

type PNCounter struct {
	inc []uint64
	dec []uint64
}

func newPNCounter(n int) *PNCounter {
	return &PNCounter{
		inc: make([]uint64, n),
		dec: make([]uint64, n),
	}
}

// inc=1 --> increment, = --> decrement
func (pn *PNCounter) op(inc bool, replicaID int) {
	if inc {
		pn.inc[replicaID] += 1
	} else {
		pn.dec[replicaID] -= 1
	}
}

func (pn *PNCounter) value() uint64 {
	var total uint64 = 0

	for i := range pn.inc {
		total += pn.inc[i] - pn.dec[i]
	}
	
	return total
}

func (pn *PNCounter) merge(other *PNCounter) {
	for i := range pn.inc {
		if other.inc[i] > pn.inc[i] {
			pn.inc[i] = other.inc[i]
		}

		if other.dec[i] > pn.dec[i] {
			pn.dec[i] = other.dec[i]
		}
	}
}

func main() {
    a := newPNCounter(2)
    b := newPNCounter(2)

    a.op(true, 0)  // replica 0 increments
    a.op(true, 0)
    b.op(false, 1) // replica 1 decrements

    fmt.Println("a:", a.value()) // 2
    fmt.Println("b:", b.value()) // -1

    a.merge(b)
    b.merge(a)

    fmt.Println("a after merge:", a.value()) // 1
    fmt.Println("b after merge:", b.value()) // 1
}
