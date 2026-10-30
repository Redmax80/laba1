#include "singleList.h"
#include <algorithm>
#include <stdexcept>

using namespace std;

static SingleNode* locate(const SingleList* list, const string& target) {
    auto* current = list->head;
    while (current && current->value != target) current = current->next;
    if (!current) throw runtime_error("value not found");
    return current;
}

// Изменяем связь перед узлом: одинаково для головы и середины.
static string unlinkNext(SingleList* list, SingleNode* previous) {
    SingleNode*& link = previous ? previous->next : list->head;
    if (!link) throw runtime_error("no element to delete");
    auto* removed = link;
    string result = removed->value;
    link = removed->next;
    if (removed == list->tail) list->tail = previous;
    delete removed;
    --list->size;
    return result;
}

SingleList* singleCreate() { return new SingleList; }
void singleDestroy(SingleList* list) {
    if (!list) return;
    while (list->head) {
        auto* removed = list->head;
        list->head = removed->next;
        delete removed;
    }
    delete list;
}
int singleSize(const SingleList* list) { return list->size; }
void singleInsertHead(SingleList* list, const string& value) {
    auto* added = new SingleNode{value, list->head};
    list->head = added;
    if (!list->tail) list->tail = added;
    ++list->size;
}
void singleInsertTail(SingleList* list, const string& value) {
    auto* added = new SingleNode{value};
    if (list->tail) list->tail->next = added;
    else list->head = added;
    list->tail = added;
    ++list->size;
}
void singlePush(SingleList* list, const string& value) { singleInsertTail(list, value); }
void singleInsertBefore(SingleList* list, const string& target, const string& value) {
    SingleNode** link = &list->head;
    while (*link && (*link)->value != target) link = &(*link)->next;
    if (!*link) throw runtime_error("value not found");
    *link = new SingleNode{value, *link};
    ++list->size;
}
void singleInsertAfter(SingleList* list, const string& target, const string& value) {
    auto* current = locate(list, target);
    current->next = new SingleNode{value, current->next};
    if (current == list->tail) list->tail = current->next;
    ++list->size;
}
string singleDeleteHead(SingleList* list) { return unlinkNext(list, nullptr); }
string singleDeleteTail(SingleList* list) {
    SingleNode* previous = nullptr;
    for (auto* current = list->head; current && current->next; current = current->next)
        previous = current;
    return unlinkNext(list, previous);
}
string singleDelete(SingleList* list, const string& target) {
    SingleNode* previous = nullptr;
    auto* current = list->head;
    while (current && current->value != target) {
        previous = current;
        current = current->next;
    }
    if (!current) throw runtime_error("value not found");
    return unlinkNext(list, previous);
}
string singleDeleteBefore(SingleList* list, const string& target) {
    SingleNode* previous = nullptr;
    SingleNode* beforePrevious = nullptr;
    auto* current = list->head;
    while (current && current->value != target) {
        beforePrevious = previous;
        previous = current;
        current = current->next;
    }
    if (!current) throw runtime_error("value not found");
    if (!previous) throw runtime_error("no element before value");
    return unlinkNext(list, beforePrevious);
}
string singleDeleteAfter(SingleList* list, const string& target) {
    return unlinkNext(list, locate(list, target));
}
string singleRemove(SingleList* list, const string& value) { return singleDelete(list, value); }
string singleGet(const SingleList* list, const string& value) { return locate(list, value)->value; }
int singleFind(const SingleList* list, const string& value) {
    int index = 0;
    for (auto* current = list->head; current; current = current->next, ++index)
        if (current->value == value) return index;
    return -1;
}
vector<string> singleRead(const SingleList* list, bool reverse) {
    vector<string> result;
    result.reserve(list->size);
    for (auto* current = list->head; current; current = current->next)
        result.push_back(current->value);
    if (reverse) std::reverse(result.begin(), result.end());
    return result;
}
