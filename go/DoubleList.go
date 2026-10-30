package main

type DoubleNode struct {
	value      string
	prev, next *DoubleNode
}
type DoubleList struct {
	head, tail *DoubleNode
	size       int
}

func doubleCreate() *DoubleList       { return &DoubleList{} }
func doubleSize(list *DoubleList) int { return list.size }

// Начинаем с ближайшего конца: доступ к обоим краям занимает O(1).
func doubleAt(list *DoubleList, index int) *DoubleNode {
	if index < 0 || index >= list.size {
		panic("index out of range")
	}
	if index < list.size/2 {
		current := list.head
		for position := 0; position < index; position++ {
			current = current.next
		}
		return current
	}
	current := list.tail
	for position := list.size - 1; position > index; position-- {
		current = current.prev
	}
	return current
}
func doubleInsert(list *DoubleList, index int, value string) {
	if index < 0 || index > list.size {
		panic("index out of range")
	}
	var left, right *DoubleNode
	if index == list.size {
		left = list.tail
	} else {
		right = doubleAt(list, index)
		left = right.prev
	}
	added := &DoubleNode{value: value, prev: left, next: right}
	if left == nil {
		list.head = added
	} else {
		left.next = added
	}
	if right == nil {
		list.tail = added
	} else {
		right.prev = added
	}
	list.size++
}
func doubleGet(list *DoubleList, index int) string { return doubleAt(list, index).value }
func doubleDelete(list *DoubleList, index int) string {
	removed := doubleAt(list, index)
	if removed.prev == nil {
		list.head = removed.next
	} else {
		removed.prev.next = removed.next
	}
	if removed.next == nil {
		list.tail = removed.prev
	} else {
		removed.next.prev = removed.prev
	}
	list.size--
	return removed.value
}
func doubleFind(list *DoubleList, value string) int {
	position := 0
	for current := list.head; current != nil; current = current.next {
		if current.value == value {
			return position
		}
		position++
	}
	return -1
}
func doubleValues(list *DoubleList, reverse bool) []string {
	result := make([]string, 0, list.size)
	current := list.head
	if reverse {
		current = list.tail
	}
	for current != nil {
		result = append(result, current.value)
		if reverse {
			current = current.prev
		} else {
			current = current.next
		}
	}
	return result
}
