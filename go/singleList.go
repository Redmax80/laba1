package main

// Узлы размещаются отдельно; head и tail исключают проход при вставке на концах.
type SingleNode struct {
	value string
	next  *SingleNode
}
type SingleList struct {
	head, tail *SingleNode
	size       int
}

func singleCreate() *SingleList       { return &SingleList{} }
func singleSize(list *SingleList) int { return list.size }
func singleAt(list *SingleList, index int) *SingleNode {
	if index < 0 || index >= list.size {
		panic("index out of range")
	}
	current := list.head
	for position := 0; position < index; position++ {
		current = current.next
	}
	return current
}
func singleInsert(list *SingleList, index int, value string) {
	if index < 0 || index > list.size {
		panic("index out of range")
	}
	added := &SingleNode{value: value}
	switch {
	case index == 0:
		added.next = list.head
		list.head = added
		if list.tail == nil {
			list.tail = added
		}
	case index == list.size:
		list.tail.next = added
		list.tail = added
	default:
		previous := singleAt(list, index-1)
		added.next = previous.next
		previous.next = added
	}
	list.size++
}
func singleGet(list *SingleList, index int) string { return singleAt(list, index).value }
func singleDelete(list *SingleList, index int) string {
	if index < 0 || index >= list.size {
		panic("index out of range")
	}
	var previous *SingleNode
	link := &list.head
	if index > 0 {
		previous = singleAt(list, index-1)
		link = &previous.next
	}
	removed := *link
	*link = removed.next
	if removed == list.tail {
		list.tail = previous
	}
	list.size--
	return removed.value
}
func singleFind(list *SingleList, value string) int {
	position := 0
	for current := list.head; current != nil; current = current.next {
		if current.value == value {
			return position
		}
		position++
	}
	return -1
}
func singleValues(list *SingleList) []string {
	result := make([]string, 0, list.size)
	for current := list.head; current != nil; current = current.next {
		result = append(result, current.value)
	}
	return result
}
