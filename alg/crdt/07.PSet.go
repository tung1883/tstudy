package main
import "fmt"

type PSet struct {
	Added		map[string]struct{}
	Removed 	map[string]struct{}
}

func NewPSet() *PSet {
	return &PSet{
		Added:		make(map[string]struct{}),
		Removed:	make(map[string]struct{}),
	}
}

func (ps *PSet) Add(value string) {
	ps.Added[value] = struct{}{}
}

func (ps *PSet) Remove(value string) {
	_, exists := ps.Added[value]

	if exists {
		ps.Removed[value] = struct{}{}
	}
}

func (ps *PSet) Merge (other PSet) {
	for value := range other.Added {
		ps.Add(value)
	}

	for value := range other.Removed {
		ps.Remove(value)
	}
}