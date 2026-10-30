#include "value3.h"
#include <queue>

using namespace std;

static void destroyNodes(TreeNode* node) {
    if (!node) return;
    destroyNodes(node->left);
    destroyNodes(node->right);
    delete node;
}
static void drawNode(const TreeNode* node, int depth, const string& label, string& text) {
    if (!node) return;
    text += string(depth * 2, ' ') + label + ": " + to_string(node->key) + '\n';
    drawNode(node->left, depth + 1, "L", text);
    drawNode(node->right, depth + 1, "R", text);
}
CompleteTree* treeCreate() { return new CompleteTree; }
void treeDestroy(CompleteTree* tree) {
    if (!tree) return;
    destroyNodes(tree->root);
    delete tree;
}

// Первое свободное место при BFS сохраняет свойство Complete Binary Tree.
void treeInsert(CompleteTree* tree, int key) {
    if (!tree->root) {
        tree->root = new TreeNode{key};
        ++tree->size;
        return;
    }
    queue<TreeNode*> pending;
    pending.push(tree->root);
    while (!pending.empty()) {
        auto* parent = pending.front();
        pending.pop();
        if (!parent->left) {
            parent->left = new TreeNode{key};
            break;
        }
        if (!parent->right) {
            parent->right = new TreeNode{key};
            break;
        }
        pending.push(parent->left);
        pending.push(parent->right);
    }
    ++tree->size;
}
bool treeContains(const CompleteTree* tree, int key) {
    queue<const TreeNode*> pending;
    if (tree->root) pending.push(tree->root);
    while (!pending.empty()) {
        const auto* node = pending.front();
        pending.pop();
        if (node->key == key) return true;
        if (node->left) pending.push(node->left);
        if (node->right) pending.push(node->right);
    }
    return false;
}
bool treeIsComplete(const CompleteTree* tree) {
    queue<const TreeNode*> pending;
    pending.push(tree->root);
    bool gapSeen = false;
    while (!pending.empty()) {
        const auto* node = pending.front();
        pending.pop();
        if (!node) gapSeen = true;
        else {
            // После первого пропуска ненулевых узлов быть не должно.
            if (gapSeen) return false;
            pending.push(node->left);
            pending.push(node->right);
        }
    }
    return true;
}
vector<int> treeValues(const CompleteTree* tree) {
    vector<int> result;
    result.reserve(tree->size);
    queue<const TreeNode*> pending;
    if (tree->root) pending.push(tree->root);
    while (!pending.empty()) {
        const auto* node = pending.front();
        pending.pop();
        result.push_back(node->key);
        if (node->left) pending.push(node->left);
        if (node->right) pending.push(node->right);
    }
    return result;
}
string treeRead(const CompleteTree* tree) {
    if (!tree->root) return "(empty)";
    string text;
    drawNode(tree->root, 0, "ROOT", text);
    text.pop_back();
    return text;
}
