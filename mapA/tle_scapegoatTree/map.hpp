/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP
// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"


namespace sjtu {

constexpr double ALPHA = 0.73;

template<class Key, class T, class Compare = std::less<Key> >
class map {
public:
    class iterator;
    class const_iterator;
    typedef pair<const Key, T> value_type;
private:
    bool cmp(const value_type* a, const value_type* b) const {
        if(a == nullptr || b == nullptr) return b == nullptr; // nullptr greater than everything.
        return Compare()(a->first, b->first);
    }
    bool equal(const value_type* a, const value_type* b) const {
        if(a == nullptr || b == nullptr) return a == b;
        return !Compare()(a->first, b->first) && !Compare()(b->first, a->first);
    }
    bool cmp(const value_type* a, const Key* b) const {
        if(a == nullptr || b == nullptr) return b == nullptr; // nullptr greater than everything.
        return Compare()(a->first, *b);
    }
    bool equal(const value_type* a, const Key* b) const {
        if(a == nullptr || b == nullptr) return 0;
        return !Compare()(a->first, *b) && !Compare()(*b, a->first);
    }
    struct Node {
         value_type* v;
         Node *ls, *rs, *fa;
         int siz;
         Node(value_type* _v = nullptr): v(_v), ls(nullptr), rs(nullptr), fa(nullptr), siz(1) {}
         Node(const Node &oth): ls(nullptr), rs(nullptr), fa(nullptr), siz(oth.siz) { v = oth.v == nullptr ? nullptr : new value_type(*oth.v); }
         ~Node() { delete  v; }
         void maintain() { siz = (ls ? ls->siz : 0) + (rs ? rs->siz : 0) + 1; }
         void reset() { ls = rs = fa = nullptr, siz = 1; }
    }*root;
    void dfs(Node** const dst, int& cnt, Node* const x) {
        if(x->ls) dfs(dst, cnt, x->ls);
        dst[++cnt] = x;
        if(x->rs) dfs(dst, cnt, x->rs);
        x->reset();
    }
    Node* rebuild(Node** const src, const int l, const int r) {
        const int mid = (l + r) >> 1;
        Node* ret = src[mid];
        if(l < mid) ret->ls = rebuild(src, l, mid - 1), ret->ls->fa = ret;
        if(mid < r) ret->rs = rebuild(src, mid + 1, r), ret->rs->fa = ret;
        ret->maintain();
        return ret;
    }
    Node *fail, *fail_fa;
    void checkFail(Node* x) {
        while(x) {
            if(x->ls && x->ls->siz > x->siz * ALPHA) fail_fa = x->fa, fail = x;
            if(x->rs && x->rs->siz > x->siz * ALPHA) fail_fa = x->fa, fail = x;
            x = x->fa;
        }
    }

    void checkRebuild(Node* const cur) {
        fail = fail_fa = nullptr, checkFail(cur);
        if(fail) {
            Node** temp = new Node* [fail->siz + 3];
            int cnt = 0;
            dfs(temp, cnt, fail);
            auto t = (fail_fa == nullptr ? root : (fail == fail_fa->ls ? fail_fa->ls : fail_fa->rs)) = rebuild(temp, 1, cnt);
            t->fa = fail_fa; // !
            delete[] temp;
        }
    }
    pair<iterator, bool> insert(value_type* const v) {
        Node* cur = root;
        while(1) {
            if(equal(cur->v, v)) {
                delete v;
                return pair<iterator, bool>(iterator(this, cur), 0); // todo: return an iterator.
            }
            if(cmp(cur->v, v)) {
                if(cur->rs) cur = cur->rs;
                else {
                    cur->rs = new Node(v), cur->rs->fa = cur;
                    cur->maintain(), cur = cur->rs;
                    break;
                }
            } else {
                if(cur->ls) cur = cur->ls;
                else {
                    cur->ls = new Node(v), cur->ls->fa = cur;
                    cur->maintain(), cur = cur->ls;
                    break;
                }
            }
        }
        fixChain(cur), checkRebuild(cur);
        return pair<iterator, bool>(iterator(this, cur), 1);
    }
    void fixChain(Node* pos) {
        while(pos) {
            pos->maintain();
            pos = pos->fa;
        }
    }
    void erase(Node* pos) {
        if(pos->ls == nullptr && pos->rs == nullptr) {
            if(pos->fa) (pos == pos->fa->ls ? pos->fa->ls : pos->fa->rs) = nullptr;
            fixChain(pos->fa), checkRebuild(pos->fa);
            delete pos;
        } else {
            if(pos->ls == nullptr || pos->rs == nullptr) {
                Node* son = pos->ls ? pos->ls : pos->rs;
                if(pos->fa) (pos == pos->fa->ls ? pos->fa->ls : pos->fa->rs) = son, son->fa = pos->fa;
                else root = son, son->fa = nullptr;
                fixChain(pos->fa), checkRebuild(pos->fa);
                delete pos;
            } else {
                Node *son = pos->ls;
                while (son->rs) son = son->rs;
                if(son->fa != pos) {
                    if(pos->fa) (pos == pos->fa->ls ? pos->fa->ls : pos->fa->rs) = son;
                    else root = son;
                    (son == son->fa->ls ? son->fa->ls : son->fa->rs) = pos;
                    std::swap(pos->ls, son->ls), std::swap(pos->rs, son->rs), std::swap(pos->fa, son->fa);
                    if(pos->ls) pos->ls->fa = pos; if(pos->rs) pos->rs->fa = pos;
                    if(son->ls) son->ls->fa = son; if(son->rs) son->rs->fa = son;
                } else {
                    if(pos->fa) (pos == pos->fa->ls ? pos->fa->ls : pos->fa->rs) = son, son->fa = pos->fa;
                    else root = son, son->fa = nullptr;
                    const auto son_ls = son->ls, son_rs = son->rs;
                    (son == pos->ls ? son->ls : son->rs) = pos, pos->fa = son;
                    if((pos == son->ls ? (son->rs = pos->rs) : (son->ls = pos->ls))) (pos == son->ls ? son->rs : son->ls)->fa = son;
                    if((pos->ls = son_ls)) pos->ls->fa = pos;
                    if((pos->rs = son_rs)) pos->rs->fa = pos;
                }
                erase(pos);
            }
        }
    }
    Node* find(const Key* tar) const {
        Node* cur = root;
        while(cur) {
            if(equal(cur->v, tar)) return cur;
            if(cmp(cur->v, tar)) cur = cur->rs;
            else cur = cur->ls;
        }
        return nullptr;
    }
    Node* findPrv(Node* pos) { // return nullptr when failed.
        if(pos == nullptr) return nullptr;
        if(pos->ls) {
            Node* ret = pos->ls;
            while(ret->rs) ret = ret -> rs;
            return ret;
        }
        while(pos->fa && pos == pos->fa->ls) pos = pos->fa;
        return pos->fa;
    }
    Node* findNxt(Node* pos) {
        if(pos == nullptr) return nullptr;
        if(pos->rs) {
            Node* ret = pos->rs;
            while(ret->ls) ret = ret -> ls;
            return ret;
        }
        while(pos->fa && pos == pos->fa->rs) pos = pos->fa;
        return pos->fa;
    }
    Node* findPrv(const Node* pos) const { // const version of previous two functions.
        if(pos == nullptr) return nullptr;
        if(pos->ls) {
            Node* ret = pos->ls;
            while(ret->rs) ret = ret -> rs;
            return ret;
        }
        while(pos->fa && pos == pos->fa->ls) pos = pos->fa;
        return pos->fa;
    }
    Node* findNxt(const Node* pos) const {
        if(pos == nullptr) return nullptr;
        if(pos->rs) {
            Node* ret = pos->rs;
            while(ret->ls) ret = ret -> ls;
            return ret;
        }
        while(pos->fa && pos == pos->fa->rs) pos = pos->fa;
        return pos->fa;
    }

    void deleteAll(Node* pos) {
        if(pos == nullptr) return;
        if(pos->ls) deleteAll(pos->ls);
        if(pos->rs) deleteAll(pos->rs);
        delete pos;
    }
    Node* copyAll(Node* cur) {
        if(cur == nullptr) return nullptr;
        Node* ret = new Node(*cur);
        if(cur->ls) ret->ls = copyAll(cur->ls), ret->ls->fa = ret;
        if(cur->rs) ret->rs = copyAll(cur->rs), ret->rs->fa = ret;
        return ret;
    }

    Node* nodeBegin() const {
        Node* cur = root;
        while(cur->ls) cur = cur->ls;
        return cur;
    }
    Node* nodeEnd() const {
        Node* cur = root;
        while(cur->rs) cur = cur->rs;
        return cur;
    }
public:
	class iterator {
        public:
            map* bel;
            Node* tar;
            iterator(map* _bel = nullptr, Node* _tar = nullptr): bel(_bel), tar(_tar) {}
            iterator(const iterator &other):bel(other.bel), tar(other.tar) {}
            iterator operator++(int) { auto ret = *this; tar = bel->findNxt(tar); if(tar == nullptr) throw invalid_iterator(); else return ret; }
            iterator & operator++() { tar = bel->findNxt(tar); if(tar == nullptr) throw invalid_iterator(); else return *this; }
            iterator operator--(int) { auto ret = *this; tar = bel->findPrv(tar); if(tar == nullptr) throw invalid_iterator(); else return ret; }
            iterator & operator--() { tar = bel->findPrv(tar); if(tar == nullptr) throw invalid_iterator(); else return *this; }
            value_type & operator*() const { return *tar->v; }
            bool operator==(const iterator &rhs) const { return  bel == rhs.bel && tar == rhs.tar; }
            bool operator==(const const_iterator &rhs) const { return  bel == rhs.bel && tar == rhs.tar; }
            bool operator!=(const iterator &rhs) const { return !(*this == rhs);}
            bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
            value_type* operator->() const noexcept { return tar->v; }
	};
	class const_iterator {
		public:
            const map* bel;
            const Node* tar;
			const_iterator(const map* _bel = nullptr, const Node* _tar = nullptr): bel(_bel), tar(_tar) {}
			const_iterator(const const_iterator &other):bel(other.bel), tar(other.tar) {}
			const_iterator(const iterator &other):bel(other.bel), tar(other.tar) {}
            const_iterator operator++(int) { auto ret = *this; tar = bel->findNxt(tar); if(tar == nullptr) throw invalid_iterator(); else return ret; }
            const_iterator & operator++() { tar = bel->findNxt(tar); if(tar == nullptr) throw invalid_iterator(); else return *this; }
            const_iterator operator--(int) { auto ret = *this; tar = bel->findPrv(tar); if(tar == nullptr) throw invalid_iterator(); else return ret; }
            const_iterator & operator--() { tar = bel->findPrv(tar); if(tar == nullptr) throw invalid_iterator(); else return *this; }
            const value_type & operator*() const { return *tar->v; }
            bool operator==(const iterator &rhs) const { return  bel == rhs.bel && tar == rhs.tar; }
            bool operator==(const const_iterator &rhs) const { return  bel == rhs.bel && tar == rhs.tar; }
            bool operator!=(const iterator &rhs) const { return !(*this == rhs);}
            bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
            const value_type* operator->() const noexcept { return tar->v; }
	};
	map() { root = new Node(); }
	map(const map &other) { root = copyAll(other.root); }
	map & operator=(const map &other) { if(this !=&other) deleteAll(root), root = copyAll(other.root); return *this; }
	~map() { deleteAll(root); }
	T & at(const Key &key) { Node* tar = find(&key); if(tar == nullptr) throw index_out_of_bound(); return tar->v->second; }
	const T & at(const Key &key) const { Node* tar = find(&key); if(tar == nullptr) throw index_out_of_bound(); return tar->v->second; }
	T & operator[](const Key &key) {
	    Node* tar = find(&key);
	    if(tar == nullptr) {
	        value_type* nv = new value_type(key, T());
	        tar = insert(nv).first.tar;
        }
	    return tar->v->second;
	}
	const T & operator[](const Key &key) const { return  at(key); }
	iterator begin() { return iterator(this, nodeBegin()); }
	const_iterator cbegin() const { return const_iterator(this, nodeBegin()); }
	iterator end()  { return iterator(this, nodeEnd()); }
	const_iterator cend() const { return const_iterator(this, nodeEnd()); }
	bool empty() const { return size() == 0; }
	size_t size() const { return root->siz - 1; }
	void clear() { deleteAll(root), root = new Node(); }
	pair<iterator, bool> insert(const value_type &value) { value_type* nv = new value_type(value); return insert(nv); }
	void erase(iterator pos) { if(pos.bel != this || pos.tar->v == nullptr) throw invalid_iterator(); else erase(pos.tar); }
	size_t count(const Key &key) const { auto tar = find(&key); return tar != nullptr; }
	iterator find(const Key &key) { auto tar = find(&key); return tar == nullptr ? end() : iterator(this, tar); }
	const_iterator find(const Key &key) const { auto tar = find(&key); return tar == nullptr ? cend() : const_iterator(this, tar); }
};

}

#endif
