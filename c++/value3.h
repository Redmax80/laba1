#pragma once
#include <string>
#include <vector>

// Вариант 3: последний уровень заполняется слева направо.
struct TreeNode {
    int key;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;
};
struct CompleteTree {
    TreeNode* root = nullptr;
    int size = 0;
};

CompleteTree* treeCreate();
void treeDestroy(CompleteTree* tree);
void treeInsert(CompleteTree* tree, int key);
bool treeContains(const CompleteTree* tree, int key);
bool treeIsComplete(const CompleteTree* tree);
std::vector<int> treeValues(const CompleteTree* tree);
std::string treeRead(const CompleteTree* tree);
