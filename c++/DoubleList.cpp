#include "DoubleList.h"
#include <stdexcept>

using namespace std;

static DoubleNode* locate(const DoubleList* list, const string& target) {
    auto* current = list->head;
    while (current && current->value != target) current = current->next;
    if (!current) throw runtime_error("value not found");
    return current;
}

// Вставка между соседями; nullptr обозначает границу списка.
static void linkBetween(DoubleList* list, DoubleNode* left, DoubleNode* right,
                        const string& value) {
    auto* added = new DoubleNode{value, left, right};
    if (left) left->next = added;
    else list->head = added;
    if (right) right->prev = added;
    else list->tail = added;
    ++list->size;
}
static string unlinkNode(DoubleList* list, DoubleNode* removed) {
    if (!removed) throw runtime_error("no element to delete");
    string result = removed->value;
    if (removed->prev) removed->prev->next = removed->next;
    else list->head = removed->next;
    if (removed->next) removed->next->prev = removed->prev;
    else list->tail = removed->prev;
    delete removed;
    --list->size;
    return result;
}
DoubleList* doubleCreate() { return new DoubleList; }
void doubleDestroy(DoubleList* list) {
    if (!list) return;
    while (list->head) {
        auto* removed = list->head;
        list->head = removed->next;
        delete removed;
    }
    delete list;
}
int doubleSize(const DoubleList* list) { return list->size; }
void doubleInsertHead(DoubleList* list, const string& value) {
    linkBetween(list, nullptr, list->head, value);
}
void doubleInsertTail(DoubleList* list, const string& value) {
    linkBetween(list, list->tail, nullptr, value);
}
void doublePush(DoubleList* list, const string& value) { doubleInsertTail(list, value); }
void doubleInsertBefore(DoubleList* list, const string& target, const string& value) {
    auto* right = locate(list, target);
    linkBetween(list, right->prev, right, value);
}
void doubleInsertAfter(DoubleList* list, const string& target, const string& value) {
    auto* left = locate(list, target);
    linkBetween(list, left, left->next, value);
}
string doubleDeleteHead(DoubleList* list) { return unlinkNode(list, list->head); }
string doubleDeleteTail(DoubleList* list) { return unlinkNode(list, list->tail); }
string doubleDelete(DoubleList* list, const string& target) {
    return unlinkNode(list, locate(list, target));
}
string doubleDeleteBefore(DoubleList* list, const string& target) {
    return unlinkNode(list, locate(list, target)->prev);
}
string doubleDeleteAfter(DoubleList* list, const string& target) {
    return unlinkNode(list, locate(list, target)->next);
}
string doubleRemove(DoubleList* list, const string& value) { return doubleDelete(list, value); }
string doubleGet(const DoubleList* list, const string& value) { return locate(list, value)->value; }
int doubleFind(const DoubleList* list, const string& value) {
    int index = 0;
    for (auto* current = list->head; current; current = current->next, ++index)
        if (current->value == value) return index;
    return -1;
}
vector<string> doubleRead(const DoubleList* list, bool reverse) {
    vector<string> result;
    result.reserve(list->size);
    for (auto* current = reverse ? list->tail : list->head; current;
         current = reverse ? current->prev : current->next)
        result.push_back(current->value);
    return result;
}
