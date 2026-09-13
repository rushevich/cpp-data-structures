#include "rb_tree.hpp"

#include <algorithm>
#include <print>
#include <ranges>

const auto inorder_print = [](this auto& recurse, rushevich::NodeBase* root) -> void {
    if (root != nullptr) {
        recurse(root->left);
        std::println("{} ", rushevich::NodeTraits<int>::get_derived(root)->data);
        recurse(root->right);
    }
};

int main() {
    rushevich::RedBlackTree<int> my_tree {};

    for (int i = 0; i < 100; ++i) {
        my_tree.insert_and_rebalance(i);
    }
    rushevich::NodeBase* root = static_cast<rushevich::NodeBase*>(my_tree.root());
    inorder_print(root);
    std::println("Hello world");
    return 0;
}
