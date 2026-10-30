#include "array.h"
#include "singleList.h"
#include "DoubleList.h"
#include "stack.h"
#include "quene.h"
#include "value3.h"
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

int main() {
    auto* array = arrayCreate();
    auto* single = singleCreate();
    auto* doubly = doubleCreate();
    std::vector<std::string> expected;
    std::mt19937 random(2026);
    // Независимая модель проверяет данные и прямые/обратные связи после изменений.
    for (int step = 0; step < 1000; ++step) {
        if (expected.empty() || random() % 2 == 0) {
            const auto value = std::to_string(step);
            arrayPush(array, value);
            singleInsertTail(single, value);
            doubleInsertTail(doubly, value);
            expected.push_back(value);
        } else {
            const auto value = expected.front();
            assert(arrayDelete(array, 0) == value);
            assert(singleDeleteHead(single) == value);
            assert(doubleDeleteHead(doubly) == value);
            expected.erase(expected.begin());
        }
        assert(arrayRead(array) == expected);
        assert(singleRead(single, false) == expected);
        assert(doubleRead(doubly, false) == expected);
        assert(doubleRead(doubly, true) == std::vector<std::string>(expected.rbegin(), expected.rend()));
        assert(singleSize(single) == static_cast<int>(expected.size()));
        assert(doubleSize(doubly) == static_cast<int>(expected.size()));
        assert((single->tail == nullptr) == expected.empty());
        if (single->tail) assert(single->tail->next == nullptr);
        if (doubly->head) assert(doubly->head->prev == nullptr);
        if (doubly->tail) assert(doubly->tail->next == nullptr);
    }
    arrayDestroy(array);
    singleDestroy(single);
    doubleDestroy(doubly);

    auto* stack = stackCreate();
    auto* queue = queueCreate();
    for (int i = 0; i < 100; ++i) {
        stackPush(stack, std::to_string(i));
        queuePush(queue, std::to_string(i));
    }
    for (int i = 0; i < 100; ++i) {
        assert(stackPop(stack) == std::to_string(99 - i));
        assert(queuePop(queue) == std::to_string(i));
    }
    assert(queue->head == nullptr && queue->tail == nullptr);
    queuePush(queue, "again");
    assert(queuePop(queue) == "again");
    stackDestroy(stack);
    queueDestroy(queue);

    auto* tree = treeCreate();
    assert(treeIsComplete(tree));
    assert(!treeContains(tree, 0));
    std::vector<int> inserted;
    for (int i = 0; i < 127; ++i) {
        const int key = static_cast<int>(random() % 20) - 10;
        treeInsert(tree, key);
        inserted.push_back(key);
        assert(treeIsComplete(tree));
        assert(treeValues(tree) == inserted);
        assert(treeContains(tree, key));
    }
    treeDestroy(tree);
    // Отрицательные проверки не ограничиваются деревьями, созданными вставкой.
    tree = treeCreate();
    tree->root = new TreeNode{1, nullptr, new TreeNode{2}};
    assert(!treeIsComplete(tree));
    treeDestroy(tree);
    tree = treeCreate();
    tree->root = new TreeNode{1, new TreeNode{2}, new TreeNode{3, new TreeNode{4}, nullptr}};
    assert(!treeIsComplete(tree));
    treeDestroy(tree);
    std::cout << "C++: model checks, link invariants, LIFO/FIFO and Complete-tree checks passed\n";
}
