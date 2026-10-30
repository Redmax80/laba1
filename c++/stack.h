#pragma once
// Связный стек LIFO: push/pop/get работают с вершиной за O(1).

#include <string>
#include <vector>

struct StackNode {
    std::string value;
    StackNode* next = nullptr;
};

struct Stack {
    StackNode* top = nullptr;
};

Stack* stackCreate();
void stackDestroy(Stack* stack);
void stackPush(Stack* stack, const std::string& value);
std::string stackGet(const Stack* stack);
std::string stackPop(Stack* stack);
std::vector<std::string> stackRead(const Stack* stack);
