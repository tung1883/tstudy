package main
import "fmt" 

type GSet struct {
	Elements map[string]struct{}
}

func NewGSet() *GSet {
	return &GSet {
		Elements: make(map[string]struct{}),
	}
}

func (gs *GSet) Add(value string) {
	gs.Elements[value] = struct{}{}
}

func (gs *GSet) Contains(value string) bool {
	_, exists := gs.Elements[value]
	return exists
}

func (gs *GSet) Merge(other GSet) {
	for value := range other.Elements {
		gs.Add(value)
	}
}

func (gs *GSet) Values() []string {
	values := make([]string, 0, len(gs.Elements))

	for value := range gs.Elements {
		values = append(values, value)
	}

	return values
}

func main() {
    a := NewGSet()
    b := NewGSet()

    // Initial state.
    a.Add("apple")
    b.Merge(*a)

    // Network partition.
    a.Add("banana")
    b.Add("orange")

    fmt.Println("A:", a.Values())
    fmt.Println("B:", b.Values())

    // Reconnect and synchronize.
    a.Merge(*b)
    b.Merge(*a)

    fmt.Println("After merge:")
    fmt.Println("A:", a.Values())
    fmt.Println("B:", b.Values())
}