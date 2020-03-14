#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

// #define DEBUG
#ifdef DEBUG // debugging
#include <cassert>
#include <iostream>
#define debug cerr
using namespace std;
#endif


namespace sjtu {
    constexpr int BlockSiz = 1700;
#ifndef  DEBUG
    constexpr int iniSt = 301;
    constexpr int maxSiz = 1000; // if size == 1000, then split.
    constexpr int maxSt = 601;
    constexpr int minSt = 101;
#else
    constexpr int iniSt = 4;
    constexpr int maxSiz = 10; // if size == 1000, then split.
    constexpr int maxSt = 6;
    constexpr int minSt = 3;
#endif

template<class T>
class deque {
public:
    class iterator;
    class const_iterator;
private:
    struct DataType {
        T* dat;
        DataType(): dat(nullptr) {}
        DataType(const DataType &x) {dat = x.dat;}
        explicit DataType(const T &x) {dat = new T(x);}
        ~DataType() {if(dat != nullptr) { delete dat; }}
        DataType& operator = (DataType &&x) { if(this != &x) dat = x.dat, x.dat = nullptr; return *this; }
        DataType& operator = (const std::nullptr_t &nptr) { return dat = nptr, *this; }
    };
    struct Block {
        DataType* dat;
        int st, ed; // visit st for the first element, ed for the last element.
        Block *prv, *nxt;
        Block():st(iniSt), ed(iniSt - 1), prv(nullptr), nxt(nullptr) { dat = new DataType[BlockSiz]; }
        Block(const DataType* src, const int siz): st(iniSt), ed(iniSt + siz - 1), prv(nullptr), nxt(nullptr)
        { dat = new DataType[BlockSiz]; for(int i = 0; i < siz; i++) dat[st + i].dat = src[i].dat; } // start from src[0]
        Block(const Block &b): st(b.st), ed(b.ed), prv(nullptr), nxt(nullptr)
        { dat = new DataType[BlockSiz]; for(int i = st; i <= ed; i++) dat[i] = DataType(*b.dat[i].dat); }
        ~Block() { delete[] dat; };
        int size() { return ed - st + 1; }
        friend void moveNext(Block* &cur, DataType* &tar, int n) {
            int cat = tar - cur->dat;
            while(cat + n > cur->ed) {
                if(!cur->nxt->size()) break; // out of range.
                n -= cur->ed - cat;
                cur = cur->nxt;
                tar = cur->dat + cur->st - 1;
                cat = tar - cur->dat;
            }
            tar += n;
        }
        friend void movePrev(Block* &cur, DataType* &tar, int n) {
            int cat = tar - cur->dat;
            while(cat - n < cur->st) {
                if(!cur->prv->size()) break; // out of range.
                n -= cat - cur->st;
                cur = cur->prv;
                tar = cur->dat + cur->ed + 1;
                cat = tar - cur->dat;
            }
            tar -= n;
        }
        friend void moveNext(const Block* &cur, const DataType* &tar, int n) {
            int cat = tar - cur->dat;
            while(cat + n > cur->ed) {
                if(!cur->nxt->size()) break; // out of range.
                n -= cur->ed - cat;
                cur = cur->nxt;
                tar = cur->dat + cur->st - 1;
                cat = tar - cur->dat;
            }
            tar += n;
        }
        friend void movePrev(const Block* &cur, const DataType* &tar, int n) {
            int cat = tar - cur->dat;
            while(cat - n < cur->st) {
                if(!cur->prv->size()) break; // out of range.
                n -= cat - cur->st;
                cur = cur->prv;
                tar = cur->dat + cur->ed + 1;
                cat = tar - cur->dat;
            }
            tar -= n;
        }
        void movEle() {
            int nst = iniSt, ned = nst + size() - 1;
            if(st > maxSt) for(int i = 0; i < size(); i++) dat[nst + i].dat = dat[st + i].dat, dat[st + i] = nullptr;
            else for(int i = size() - 1; ~i; i--) dat[nst + i].dat = dat[st + i].dat, dat[st + i] = nullptr;
            st = nst, ed = ned;
        }
        void trySplit() {
            if(size() < maxSiz) {
                if(st > maxSt || st < minSt) movEle();
                return;
            }
            const int siz1 = size() / 2, siz2 = size() - siz1;
            Block *n1 = new Block(dat + st, siz1), *n2 = new Block(dat + st + siz1, siz2);
            n1->prv = prv, n1->nxt = n2, n2->prv = n1, n2->nxt = nxt;
#ifdef DEBUG
            assert(prv != nullptr && nxt != nullptr);
#endif
            prv->nxt = n1, nxt->prv = n2;
            for(int i = st; i <= ed; i++) dat[i] = nullptr;
            delete this;
        }
        void tryRemove() {
            if(size()) return;
            if(prv != nullptr && prv == nxt) {
#ifdef DEBUG
                assert(prv->nxt == this && nxt->prv == this);
#endif
                return; // don't delete last real block.
            }
            if(prv != nullptr) prv->nxt = nxt;
            if(nxt != nullptr) nxt->prv = prv;
            delete this;
        }
        void removeKth(int k) {
            k += st - 1, --ed;
            delete dat[k].dat;
            for(int i = k; i <= ed; i++) dat[i].dat = dat[i + 1].dat;
            dat[ed + 1] = nullptr;
            tryRemove();
        }
        void insertKth(const T &v, int k) {
            k += st - 1, ++ed;
            for(int i = ed; i > k; i--) dat[i].dat = dat[i - 1].dat;
            dat[k] = DataType(v), trySplit();
        }
        void push_front(const T &x) { dat[--st] = DataType(x), trySplit(); }
        void push_back(const T &x)  { dat[++ed] = DataType(x), trySplit(); }
        void pop_front() { delete dat[st].dat; dat[st++] = nullptr, tryRemove(); }
        void pop_back()  { delete dat[ed].dat; dat[ed--] = nullptr, tryRemove(); }
    }root; // root -> nxt is the head, root -> prv is the tail.
    void deleteAll() {
        auto p = root.nxt;
        while(p != nullptr && p != &root) {
            auto p2 = p->nxt;
            delete p;
            p = p2;
        }
        root.nxt = root.prv = nullptr;
    }
    void checkRoot() {
        if(root.nxt == nullptr) {
#ifdef DEBUG
            assert(root.prv == nullptr);
#endif
            root.nxt = root.prv = new Block;
            root.prv->nxt = &root, root.nxt->prv = &root;
        }
    }
    void copyAll(const Block &root2) {
        if(root2.nxt == nullptr) {
#ifdef DEBUG
            assert(root2.prv == nullptr);
#endif
            return;
        }
        root.nxt = new Block(*root2.nxt), root.nxt->prv = &root;
        auto cur = root.nxt, cur2 = root2.nxt;
        while(cur2->nxt != &root2) {
            cur->nxt = new Block(*cur2->nxt), cur->nxt->prv = cur;
            cur = cur->nxt, cur2 = cur2->nxt;
        }
        root.prv = cur, cur->nxt = &root;
    }
    T& accessKth(int n) {
        ++n;
        auto p = root.nxt;
        while(n > p->size()) n -= p->size(), p = p->nxt;
        return *p->dat[p->st + n - 1].dat;
    }
    const T& accessKth(int n) const {
        ++n;
        auto p = root.nxt;
        while(n > p->size()) n -= p->size(), p = p->nxt;
        return *p->dat[p->st + n - 1].dat;
    }
    int fullSiz;
    iterator iteratorKth(int n) {
        const int nn = n;
        auto p = root.nxt;
        while(n > p->size()) n -= p->size(), p = p->nxt;
        return iterator(this, p, p->dat + p->st + n - 1, nn);
    }
    void insertKth(int n, const T &v) {
        checkRoot();
        auto p = root.nxt;
        while(n > p->size() + 1) n -= p->size(), p = p->nxt;
        p->insertKth(v, n);
    }
    void removeKth(int n) {
        auto p = root.nxt;
        while(n > p->size()) n -= p->size(), p = p->nxt;
        p->removeKth(n);
    }
    bool checkAccessIterator(const iterator &it) const {
        if(it.id > size() || it.id < 1) return 0;
        return 1;
    }
    bool checkAccessIterator(const const_iterator &it) const {
        if(it.id > size() || it.id < 1) return 0;
        return 1;
    }
public:
	class iterator {
	private:
	    Block* blk;
        DataType* tar;
    public:
        deque* fa;
        int id;
        iterator() = default;
        iterator(deque* _fa, Block* _blk, DataType* _tar, int _id): fa(_fa), blk(_blk), tar(_tar), id(_id) {}
		iterator operator + (const int &n) const { auto ret = *this; ret.id += n, n >= 0 ? moveNext(ret.blk, ret.tar, n) : movePrev(ret.blk, ret.tar, -n); return ret; }
		iterator operator - (const int &n) const { auto ret = *this; ret.id -= n, n >= 0 ? movePrev(ret.blk, ret.tar, n) : moveNext(ret.blk, ret.tar, -n); return ret; }
		int operator - (const iterator &rhs) const { if(fa != rhs.fa) throw invalid_iterator(); else return id - rhs.id; }
		iterator& operator += (const int &n) { return *this = *this + n; }
		iterator& operator -= (const int &n) { return *this = *this - n; }
		iterator operator ++ (int) { auto ret = *this; return *this = *this + 1, ret; }
		iterator& operator ++ ()   { return *this = *this + 1; }
		iterator operator -- (int) { auto ret = *this; return *this = *this - 1, ret; }
		iterator& operator -- ()   { return *this = *this - 1; }
        T& operator * () { if(!fa->checkAccessIterator(*this)) throw invalid_iterator(); else return *tar->dat; }
        const T& operator * () const { if(!fa->checkAccessIterator(*this)) throw invalid_iterator(); return *tar->dat; }
        T* operator -> () const { if(!fa->checkAccessIterator(*this)) throw invalid_iterator(); return tar->dat; }
		bool operator == (const iterator &rhs) const { return fa == rhs.fa && blk == rhs.blk && tar == rhs.tar && id == rhs.id; }
		bool operator == (const const_iterator &rhs) const { return fa == rhs.fa && blk == rhs.blk && tar == rhs.tar && id == rhs.id; }
		bool operator != (const iterator &rhs) const { return !(*this == rhs); }
		bool operator != (const const_iterator &rhs) const { return !(*this == rhs); }
	};
	class const_iterator {
        private:
            const Block* blk;
            const DataType* tar;
        public:
            const deque* fa;
            int id;
            const_iterator(): fa(nullptr), blk(nullptr), tar(nullptr), id(-1) {}
            const_iterator(const deque* _fa, const Block* _blk, const DataType* _tar, int _id): fa(_fa), blk(_blk), tar(_tar), id(_id) {}
			const_iterator(const const_iterator &other): fa(other.fa), blk(other.blk), tar(other.tar), id(other.id) {}
			const_iterator(const iterator &other): fa(other.fa), blk(other.blk), tar(other.tar), id(other.id) {}
            const_iterator operator + (const int &n) const { auto ret = *this; ret.id += n, n >= 0 ? moveNext(ret.blk, ret.tar, n) : movePrev(ret.blk, ret.tar, -n); return ret; }
            const_iterator operator - (const int &n) const { auto ret = *this; ret.id -= n, n >= 0 ? movePrev(ret.blk, ret.tar, n) : moveNext(ret.blk, ret.tar, -n); return ret; }
            int operator - (const const_iterator &rhs) const { if(fa != rhs.fa) throw invalid_iterator(); else return id - rhs.id; }
            const_iterator& operator += (const int &n) { return *this = *this + n; }
            const_iterator& operator -= (const int &n) { return *this = *this - n; }
            const_iterator operator ++ (int) { auto ret = *this; return *this = *this + 1, ret; }
            const_iterator& operator ++ ()   { return *this = *this + 1; }
            const_iterator operator -- (int) { auto ret = *this; return *this = *this - 1, ret; }
            const_iterator& operator -- ()   { return *this = *this - 1; }
            const T& operator * () const { if(!fa->checkAccessIterator(*this)) throw invalid_iterator(); else return *tar->dat; }
            const T* operator -> () const noexcept { if(!fa->checkAccessIterator(*this)) throw invalid_iterator(); else return tar->dat; }
            bool operator == (const iterator &rhs) const { return fa == rhs.fa && blk == rhs.blk && tar == rhs.tar && id == rhs.id; }
            bool operator == (const const_iterator &rhs) const { return fa == rhs.fa && blk == rhs.blk && tar == rhs.tar && id == rhs.id; }
            bool operator != (const iterator &rhs) const { return !(*this == rhs); }
            bool operator != (const const_iterator &rhs) const { return !(*this == rhs); }
	};
	deque(): fullSiz(0) { checkRoot(); }
	deque(const deque &other): fullSiz(other.fullSiz) { copyAll(other.root); }
	~deque() { deleteAll(); }
	deque &operator=(const deque &other) { if(&other != this) deleteAll(), copyAll(other.root), fullSiz = other.fullSiz; return *this; }
    T & at(const size_t &pos) { if(pos >= size() || pos < 0) throw index_out_of_bound(); else return accessKth(pos); }
    const T & at(const size_t &pos) const { if(pos >= size() || pos < 0) throw index_out_of_bound(); else return accessKth(pos); }
    T & operator[] (const size_t &pos) { if(pos >= size() || pos < 0) throw index_out_of_bound(); else return accessKth(pos); }
    const T & operator[] (const size_t &pos) const { if(pos >= size() || pos < 0) throw index_out_of_bound(); else return accessKth(pos); }
	const T & front() const { if(empty()) throw container_is_empty(); else return *root.nxt->dat[root.nxt->st].dat; }
	const T & back() const  { if(empty()) throw container_is_empty(); else return *root.prv->dat[root.prv->ed].dat; }
	iterator begin() { return iterator(this, root.nxt, root.nxt->dat + root.nxt->st, 1); }
    const_iterator cbegin() const { return const_iterator(this, root.nxt, root.nxt->dat + root.nxt->st, 1); }
	iterator end() { return iterator(this, root.prv, root.prv->dat + root.prv->ed + 1, size() + 1); }
	const_iterator cend() const { return const_iterator(this, root.prv, root.prv->dat + root.prv->ed + 1, size() + 1); }
	bool empty() const { return size() == 0; }
	size_t size() const { return fullSiz; }
	void clear() { deleteAll(), checkRoot(), fullSiz = 0; }
	iterator insert(iterator pos, const T &value) {
        if(pos.fa != this) throw invalid_iterator();
        if(size_t(pos.id) > size() + 1) throw invalid_iterator();
        insertKth(pos.id, value), ++fullSiz;
        return iteratorKth(pos.id);
	}
	iterator erase(iterator pos) {
	    if(pos.fa != this) throw invalid_iterator();
	    if(size_t(pos.id) > size()) throw container_is_empty();
	    removeKth(pos.id), --fullSiz;
	    return size_t(pos.id) <= size() ? iteratorKth(pos.id) : end();
	}
	void push_back(const T &value) { checkRoot(), ++fullSiz, root.prv->push_back(value); }
	void pop_back() { if(empty()) throw container_is_empty(); else --fullSiz, root.prv->pop_back(); }
	void push_front(const T &value) { checkRoot(), ++fullSiz, root.nxt->push_front(value); }
	void pop_front() { if(empty()) throw container_is_empty(); else --fullSiz, root.nxt->pop_front(); }
};

}

#endif
