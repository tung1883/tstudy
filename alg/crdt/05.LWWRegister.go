package main
import "fmt" 

type Version struct {
	Counter uint64
	ReplicaID string
	// Ordering: first compare Counter, then compare ReplicaID
}

type Register struct {
	Value string
	Version Version

	// Local metadata
	clock uint64
	id string // replica ID
}

func NewRegister(id string) *Register {
	return &Register {
		id: id,
	}
}

// Compare 2 versions
func newer(a Version, b Version) bool {
	if a.Counter != b.Counter {
		return a.Counter > b.Counter
	}

	return a.ReplicaID > b.ReplicaID
}

// Write a new value to register
func (r *Register) Write(value string) {
	r.clock++
	r.Value = value
	r.Version = Version {
		Counter: r.clock,
		ReplicaID: r.id,
	}
}

// Merge another replica's state
// Merge(A, B) = (Version(A) > Version(B)) A : B, then advance the logical clock
func (r *Register) Merge(other Register) {
	if newer(other.Version, r.Version) {
		r.Value = other.Value
		r.Version = other.Version
	}

	// Advance the logical clock of r
	if other.Version.Counter > r.clock {
		r.clock = other.Version.Counter
	}
}

func main() {
    a := NewRegister("A")
    b := NewRegister("B")

    // Both replicas start with the same value.
    a.Write("Hello")
    b.Merge(*a)

    // Network partition.
    a.Write("Hello World")
    b.Write("Hello CRDT")

    fmt.Println("A:", a.Value, a.Version)
    fmt.Println("B:", b.Value, b.Version)

    // Reconnect and synchronize.
    a.Merge(*b)
    b.Merge(*a)

    fmt.Println("After merge:")
    fmt.Println("A:", a.Value, a.Version)
    fmt.Println("B:", b.Value, b.Version)
}