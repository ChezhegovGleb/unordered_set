#ifndef UNORDERED_SET_H
#define UNORDERED_SET_H

#include <memory>
#include <vector>
#include <iostream>

template<class T, class Hash = std::hash<T> >

struct unordered_set {
public:
    struct bucket {
        T value;
        bucket *next;
        bucket *prev;

        explicit bucket(const T &val) : value(val), next(nullptr), prev(nullptr) {};

        bucket(const T &value, bucket *prev) : value(value), next(nullptr), prev(prev) {
            prev->next = this;
        }

        ~bucket() {
            if (next != nullptr) {
                delete next;
                next = nullptr;
            }
        }
    };


    struct setIterator {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_tag = std::forward_iterator_tag;

        setIterator() : ptr(nullptr) {};

        setIterator(const setIterator &other) {
            this->ptr = other.ptr;
        }

        setIterator(bucket *ptr, unordered_set *own) : ptr(ptr), own(own) {}

        explicit setIterator(bucket *ptr) : ptr(ptr) {};

        setIterator &operator++() {
            if (ptr == own->_end)
                return *this;
            if (ptr->next != nullptr)
                ptr = ptr->next;
            else {
                size_t hash_value = own->hash_fn(ptr->value) % own->_length + 1;
                if (hash_value >= own->table.size()) {
                    ptr = own->_end;
                    return *this;
                }
                while ((hash_value + 1 < own->table.size()) && own->table[hash_value] == nullptr)
                    hash_value++;
                if (own->table[hash_value] == nullptr) {
                    ptr = own->_end;
                    return *this;
                }
                ptr = own->table[hash_value];
            }
            return *this;
        }

        setIterator operator++(int) {
            setIterator ret(ptr);
            ++(*this);
            return ret;
        }

        const T &operator*() const {
            return ptr->value;
        }

        const T *operator->() const {
            return &(ptr->value);
        }

        friend bool operator==(setIterator const& a, setIterator const& b) {
            return a.ptr == b.ptr;
        }

        friend bool operator!=(setIterator const& a, setIterator const& b) {
            return a.ptr != b.ptr;
        }

    private:
        bucket *ptr;
        unordered_set *own;
    };

    ~unordered_set() {
        clear();
    }

    unordered_set() {
        table.resize(_length, nullptr);
    }

    explicit unordered_set(size_t const &_length) : _length(_length) {
        table.resize(_length, nullptr);
    }

    setIterator begin() {
        if (empty())
            return end();
        size_t hash_value = 0;
        while (hash_value < table.size() && table[hash_value] == nullptr)
            hash_value++;
        return setIterator(table[hash_value], this);
    }

    setIterator end() {
        return setIterator(_end, this);
    }

    std::pair<setIterator, bool> insert(T const& value) {
        if (_size == _length)
            ensureCap();
        size_t _hash = hash_fn(value) % _length;
        bucket *pos = getPos(value);
        if (pos == nullptr) {
            ++_size;
            table[_hash] = new bucket(value);
            return {setIterator(table[_hash], this), true};
        } else {
            if (pos->value == value)
                return {setIterator(pos, this), false};
            _size++;
            if (pos->next != nullptr) {
                pos->next = nullptr;
                std::cout << "\n";
            }
            pos->next = new bucket(value, pos);
            pos->next->prev = pos;
            return {setIterator(pos->next, this), true};
        }
    }

    void erase(T const& value) {
        if (!contains(value))
            return;
        bucket *item = getPos(value);
        size_t _hash = hash_fn(value) % _length;
        if (item->next == nullptr && item->prev == nullptr) {
            table[_hash] = nullptr;
        } else if (item->next == nullptr) {
            item->prev->next = nullptr;
        } else if (item->prev == nullptr) {
            item->next->prev = nullptr;
            bucket *next = item->next;
            table[_hash] = item->next;
            item->next = nullptr;
        } else {
            item->prev->next = item->next;
            item->next->prev = item->prev;
            item->next = nullptr;
        }
        delete item;
        --_size;
    }

    bool contains(T const &value) {
        bucket *item = getPos(value);
        if (item == nullptr)
            return false;
        return item->value == value;
    }

    setIterator find(T const &value) {
        bucket *item = getPos(value);
        if (item == nullptr || item->value != value)
            return end();
        else
            return setIterator(item, this);
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return size() == 0;
    }

    void clear() {
        for (int i = 0; i < table.size(); ++i) {
            delete table[i];
            table[i] = nullptr;
        }
        _size = 0;
    }

    void swap(unordered_set &other) {
        std::swap(table, other.table);
        std::swap(hash_fn, other.hash_fn);
        std::swap(_end, other._end);
        std::swap(_length, other._length);
    }

private:
    void ensureCap() {
        size_t new_length = _length * 2;
        unordered_set<T> newSet(new_length);
        auto i = begin();
        while (i != end()) {
            newSet.insert((*i));
            ++i;
        }
        swap(newSet);
    }

    bucket *getPos(T const &value) {
        size_t _hash = hash_fn(value) % _length;
        bucket *pos = table[_hash];
        if (pos == nullptr) {
            return nullptr;
        } else {
            bucket *cur = pos;
            while (cur->next != nullptr && cur->value != value) {
                cur = cur->next;
            }
            return cur;
        }
    }

    Hash hash_fn;
    bucket *_end;
    size_t _size = 0;
    size_t _length = 32;

    std::vector<bucket*> table;

};

#endif
