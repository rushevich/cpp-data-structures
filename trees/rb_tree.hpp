#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>
namespace rushevich {

enum class Color : uint8_t { red, black };

struct NodeBase {
    Color color {};
    NodeBase* parent { nullptr };
    NodeBase* left { nullptr };
    NodeBase* right { nullptr };

    virtual ~NodeBase() = default;
};
template <typename T> struct Node : public NodeBase {
    T data {};

    Node() = default;
    Node(T value) : data { std::move(value) } {}
};

template <typename T> struct NodeTraits {
    static void right_rotate(NodeBase* y, NodeBase*& root) {
        auto* x = y->left;
        y->left = x->right;
        if (x->right != nullptr) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == nullptr) {
            root = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
        x->right = y;
        y->parent = x;
    }
    static void left_rotate(NodeBase* x, NodeBase*& root) {
        auto* y = x->right; // create a temporary node that refers to x’s right child
        x->right = y->left; // after the rotation, x’s right child will be the left child of y
        if (y->left != nullptr) {
            y->left->parent = x; // in correspondence to the above
        }
        // this else if chain ensures that the parent -> x relationship is maintained when y becomes
        // x’s parent
        y->parent = x->parent;
        if (x->parent == nullptr) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    static void rebalance(NodeBase* z, NodeBase*& root) {
        using enum Color;
        while (z->parent != nullptr && z->parent->color == red) {
            // If z’s parent is a left child
            if (z->parent == z->parent->parent->left) {
                NodeBase* y = z->parent->parent->right;
                if (y != nullptr && y->color == red) {
                    z->parent->color = black;
                    y->color = black;
                    z->parent->parent->color = red;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z, root);
                    }
                    z->parent->color = black;
                    z->parent->parent->color = red;
                    right_rotate(z->parent->parent, root);
                }
            } else {
                // Symmetric right-child branch
                NodeBase* y = z->parent->parent->left;
                if (y != nullptr && y->color == red) {
                    z->parent->color = black;
                    y->color = black;
                    z->parent->parent->color = red;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z, root);
                    }
                    z->parent->color = black;
                    z->parent->parent->color = red;
                    left_rotate(z->parent->parent, root);
                }
            }
        }
        root->color = black;
    }

    static auto* get_derived(NodeBase* ptr) { return static_cast<Node<T>*>(ptr); }
};

template <typename T> class RedBlackTree {
public:
    void insert_and_rebalance(T key) {
        NodeBase* x = static_cast<NodeBase*>(_root);
        NodeBase* y = nullptr;
        while (x != nullptr) {
            y = x;
            if (auto* casted = NodeTraits<T>::get_derived(x); key < casted->data) {
                x = x->left;
            } else {
                x = x->right;
            }
        }
        Node<T>* z = new Node(std::move(key)); // color is red by default
        z->parent = y;
        if (y == nullptr) {
            _root = z;
        } else if (auto* casted = NodeTraits<T>::get_derived(y); z->data < casted->data) {
            y->left = z;
        } else {
            y->right = z;
        }
        auto* downcasted = static_cast<NodeBase*>(_root);
        NodeTraits<T>::rebalance(z, downcasted);
        _root = static_cast<Node<T>*>(downcasted);
    }

    Node<T>* root() { return _root; }

private:
    Node<T>* _root { nullptr };
};
} // namespace rushevich
