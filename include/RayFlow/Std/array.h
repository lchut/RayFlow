#pragma once

#include <iterator>

namespace rayflow {

namespace rstd {

template <typename T, size_t N>
class array {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = value_type*;
    using const_iterator = const value_type*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const iterator>;

    array() = default;
    
    array(std::initializer_list<T>& v) {
        size_type cnt = 0;
        for (const T& e : v) {
            values[cnt++] = e;
        }
    }

    void fill(const T& v) {
        for (size_t i = 0; i < N; ++i) {
            data[i] = v;
        }
    }

    reference operator[](int i) {
        return values[i];
    }

    const_reference operator[](int i) const {
        return values[i];
    }

    reference front() {
        return values[0];
    }

    const_reference front() const {
        return values[0];
    }

    reference back() {
        return values[N-1];
    }

    const_reference back() const {
        return values[N-1];
    }

    pointer data() {
        return values;
    }

    iterator begin() {
        return values;
    }

    const_iterator begin() const {
        return values;
    }

    const_iterator cbegin() const {
        return values;
    }

    iterator rbegin() {
        return reverse_iterator(end());
    }

    const_iterator rbegin() const {
        return reverse_iterator(end());
    }

    const_iterator crbegin() const {
        return const_reverse_iterator(end());
    }

    iterator end() {
        return &values[N];
    }

    const_iterator end() const {
        return &values[N];
    }

    const_iterator cend() const {
        return &values[N];
    }

    iterator rend() {
        return reverse_iterator(begin());
    }

    const_iterator rend() const {
        return reverse_iterator(begin());
    }

    const_iterator crend() const {
        return const_reverse_iterator(begin());
    }

    bool empty() const {
        return begin() == end();
    }

    size_t size() const {
        return N;
    }

private:
    T values[N];
};

}
}