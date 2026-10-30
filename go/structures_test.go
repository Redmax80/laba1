package main

import (
	"math/rand"
	"reflect"
	"strconv"
	"testing"
)

func TestListsAgainstModel(t *testing.T) {
	single, doubly := singleCreate(), doubleCreate()
	model := []string{}
	random := rand.New(rand.NewSource(2026))
	for step := 0; step < 1000; step++ {
		if len(model) == 0 || random.Intn(2) == 0 {
			index, value := random.Intn(len(model)+1), strconv.Itoa(step)
			singleInsert(single, index, value)
			doubleInsert(doubly, index, value)
			model = append(model, "")
			copy(model[index+1:], model[index:])
			model[index] = value
		} else {
			index := random.Intn(len(model))
			if singleDelete(single, index) != model[index] || doubleDelete(doubly, index) != model[index] {
				t.Fatal("removed value mismatch")
			}
			model = append(model[:index], model[index+1:]...)
		}
		if !reflect.DeepEqual(singleValues(single), model) || !reflect.DeepEqual(doubleValues(doubly, false), model) {
			t.Fatal("content mismatch", step)
		}
		reverse := doubleValues(doubly, true)
		for i := range model {
			if reverse[len(model)-1-i] != model[i] {
				t.Fatal("broken reverse links")
			}
		}
		if single.size != len(model) || doubly.size != len(model) {
			t.Fatal("wrong length")
		}
		if single.tail != nil && single.tail.next != nil {
			t.Fatal("broken tail")
		}
		if doubly.head != nil && doubly.head.prev != nil {
			t.Fatal("broken head")
		}
		if doubly.tail != nil && doubly.tail.next != nil {
			t.Fatal("broken tail")
		}
	}
}

func TestArrayAgainstModel(t *testing.T) {
	array := arrayCreate()
	model := []string{}
	random := rand.New(rand.NewSource(31))
	for step := 0; step < 500; step++ {
		if len(model) == 0 || random.Intn(2) == 0 {
			index, value := random.Intn(len(model)+1), strconv.Itoa(step)
			arrayInsert(array, index, value)
			model = append(model, "")
			copy(model[index+1:], model[index:])
			model[index] = value
		} else {
			index := random.Intn(len(model))
			if arrayDelete(array, index) != model[index] {
				t.Fatal("wrong removed value")
			}
			model = append(model[:index], model[index+1:]...)
		}
		if !reflect.DeepEqual(arrayValues(array), model) {
			t.Fatal("content mismatch")
		}
	}
}

func TestStackAndQueue(t *testing.T) {
	stack, queue := stackCreate(), queueCreate()
	for i := 0; i < 100; i++ {
		stackPush(stack, strconv.Itoa(i))
		queuePush(queue, strconv.Itoa(i))
	}
	for i := 0; i < 100; i++ {
		if stackPop(stack) != strconv.Itoa(99-i) || queuePop(queue) != strconv.Itoa(i) {
			t.Fatal("LIFO/FIFO broken")
		}
	}
	if queue.head != nil || queue.tail != nil {
		t.Fatal("empty queue has dangling links")
	}
	queuePush(queue, "again")
	if queuePop(queue) != "again" {
		t.Fatal("cannot refill queue")
	}
}
