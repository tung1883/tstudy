package main
import (
	"fmt" 
	"sort"
)

type Dot struct {
	Replica string
	Counter uint64
}

type ORSet struct {
	ID string

	// Elements = set of addition dots
	Entries map[string]map[Dot]struct{}

	// Causal context: replica --> highest observed counter
	Context map[string]uint64
}

func NewORSet(id string) *ORSet {
	return &ORSet{
		ID:			id,
		Entries:	make(map[string]map[Dot]struct{}),
		Context:	make(map[string]uint64),
	}
}

func (s *ORSet) Add(value string) {
	s.Context[s.ID]++

	dot := Dot{
		Replica: s.ID,
		Counter: s.Context[s.ID],
	}

	if (s.Entries[value] == nil) {
		s.Entries[value] = make(map[Dot]struct{})
	}

	s.Entries[value][dot] = struct{}{}
}

func (s *ORSet) Remove(value string) {
	delete(s.Entries, value)
}

func (s *ORSet) Contains(value string) bool {
	dots, exists := s.Entries[value]
	return exists && len(dots) > 0
}

func (s *ORSet) Merge(other ORSet) {
	merged := make(map[string]map[Dot]struct{})

	// Collect all elements from both Entries
	for value := range s.Entries {
		merged[value] = make(map[Dot]struct{})
	}

	for value := range other.Entries {
		if merged[value] == nil {
			merged[value] = make(map[Dot]struct{})
		}
	}

	// Merge the dots using casual context
	// 3 cases for a dot:
	// 1. Present in both A&B		--> Keep
	// 2. Present in A and not B	--> Keep only if B has not observed it
	// 3. Present in B and not A 	--> Keep onyl if A has not observed it
	// A observed a dot = Counter(A) > Counter(dot)
	
	for value := range merged {
		a := s.Entries[value]
		b := other.Entries[value]

		for dot := range a {
			_, existsInB := b[dot]

			if existsInB || dot.Counter > s.Context[dot.Replica] {
				merged[value][dot] = struct{}{}
			}
		}

		for dot := range b {
			_, existsInA := a[dot]

			if existsInA || dot.Counter > s.Context[dot.Replica] {
				merged[value][dot] = struct{}{}
			}
		}

		// Remove empty values
		if len(merged[value]) == 0 {
			delete(merged, value)
		}
	}

	s.Entries = merged

	for replica, counter := range other.Context {
		if counter > s.Context[replica] {
			s.Context[replica] = counter
		}
	}
}

func (s *ORSet) Values() []string {
	values := make([]string, 0, len(s.Entries))

	for value := range s.Entries {
		if s.Contains(value) {
			values = append(values, value)
		}
	}

	sort.Strings(values)
	return values
}

func main() {
	a := NewORSet("A")
	b := NewORSet("B")

	// A adds apple.
	a.Add("apple")

	// B synchronizes with A.
	b.Merge(*a)

	fmt.Println("Initial:")
	fmt.Println("A:", a.Values())
	fmt.Println("B:", b.Values())

	// Network partition.
	// A removes apple.
	a.Remove("apple")

	// B concurrently adds apple.
	b.Add("apple")

	fmt.Println("\nDuring partition:")
	fmt.Println("A:", a.Values())
	fmt.Println("B:", b.Values())

	// Reconnect.
	a.Merge(*b)
	b.Merge(*a)

	fmt.Println("\nAfter merge:")
	fmt.Println("A:", a.Values())
	fmt.Println("B:", b.Values())

	// A adds apple again.
	a.Add("apple")

	// Synchronize again.
	b.Merge(*a)

	fmt.Println("\nAfter re-adding apple:")
	fmt.Println("A:", a.Values())
	fmt.Println("B:", b.Values())
}