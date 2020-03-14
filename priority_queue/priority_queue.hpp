#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

/*#include <iostream>
#define debug cout
using namespace std;*/

namespace sjtu {

/**
 * a container like std::priority_queue which is a heap internal.
 */

    template<typename T, class Compare = std::less<T> >
    class priority_queue {
    private:
        struct Node {
            T val;
            Node *ls, *rs;
            int dis;
            Node(const T &_val): val(_val), ls(nullptr), rs(nullptr), dis(0) {}
            void maintain() {
                if(ls == nullptr || (rs != nullptr && ls->dis < rs->dis)) std::swap(ls, rs);
                dis = (rs == nullptr ? -1 : rs->dis) + 1;
            }
        }*root;

        inline void deleteAll(Node* pos) {
            if(pos == nullptr) return;
            deleteAll(pos->ls), deleteAll(pos->rs);
            delete pos;
        }
        inline Node* copy(const Node* x) {
            if(x == nullptr) return nullptr;
            Node* ret = new Node(x->val);
            ret->ls = copy(x->ls);
            ret->rs = copy(x->rs);
            ret->maintain();
            return ret;
        }
        inline Node* merge(Node* a, Node* b) {
            if(a == nullptr || b == nullptr) return b == nullptr ? a : b;
            if(Compare()(a->val, b->val)) std::swap(a, b);
            a->rs = merge(a->rs, b);
            a->maintain();
            return a;
        }
        size_t _size;
    public:
        priority_queue(): _size(0) {
            root = nullptr;
        }
        priority_queue(const priority_queue &other): _size(other._size) {
            root = copy(other.root);
        }
        ~priority_queue() {
            deleteAll(root);
        }
        priority_queue &operator=(const priority_queue &other) {
            if(this == &other) return *this;
            deleteAll(root);
            root = copy(other.root);
            _size = other._size;
            return *this;
        }

        const T & top() const {
            if(root == nullptr) throw container_is_empty();
            return root->val;
        }

        void push(const T &e) {
            ++_size;
            Node* nv = new Node(e);
            root = merge(nv, root);
        }
        void pop() {
            if(root == nullptr) throw container_is_empty();
            --_size;
            Node* mem = root;
            root = merge(root->ls, root->rs);
            delete  mem;
        }
        size_t size() const {
            return _size;
        }
        bool empty() const {
            return root == nullptr;
        }

        void merge(priority_queue &other) {
            _size += other._size;
            root = merge(root, other.root);
            other.root = nullptr, other._size = 0;
        }
    };

}

#endif
