#pragma once

#include <RayFlow/rayflow.h>
#include <RayFlow/Std/algorithm.h>
#include <RayFlow/Std/iterator.h>
#include <RayFlow/Std/memory.h>
#include <RayFlow/Std/memory_resource.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>

namespace rayflow {

namespace rstd {

template <typename T, typename Allocator = rstd::pmr::polymorphic_allocator<T>>
class vector {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const iterator>;

private:

    void construct_n(size_type count, const T& value) {
        if (count > 0) {
            buy_raw(count);
            mLast_ = rstd::uninitialized_fill_n(mFirst_, count, value);
        }
    }

    template <typename InputIt>
    void construct_range(InputIt first, InputIt last) {
        const auto count = rstd::distance(first, last);
        if (count > 0) {
            buy_raw(count);
            mLast_ = rstd::uninitialized_copy(first, last, mFirst_);
        }
    }

public:

    vector() {}

    explicit vector(const Allocator& alloc) : mAlloc_(alloc) {}

    vector(size_type count, 
           const T& value, 
           const Allocator& alloc = Allocator()) : mAlloc_(alloc) {
        construct_n(count, value);
    }
    
    explicit vector(size_type count, const Allocator& alloc = Allocator()) : mAlloc_(alloc) {
        construct_n(count, T());
    }

    template <typename InputIt, 
              typename std::enable_if_t<rstd::is_iterator_v<InputIt>, int> = 0>
    vector(InputIt first, InputIt last, const Allocator& alloc = Allocator()) : mAlloc_(alloc) {
        construct_range(first, last);
    }

    vector(const vector& other) : mAlloc_(other.mAlloc_) {
        construct_range(other.begin(), other.end());
    }

    vector(const vector& other, const Allocator& alloc) : mAlloc_(alloc) {
        construct_range(other.begin(), other.end());
    }

public:

    vector(vector&& other) :
        mAlloc_(other.mAlloc_),
        mFirst_(other.mFirst_),
        mLast_(other.mLast_),
        mEnd_(other.mEnd_) {
        other.mFirst_ = nullptr;
        other.mLast_ = nullptr;
        other.mEnd_ = nullptr;
    }

    vector(vector&& other, const Allocator& alloc) : mAlloc_(alloc) {
        if (mAlloc_ == other.mAlloc_) {
            mFirst_ = other.mFirst_;
            mLast_ = other.mLast_;
            mEnd_ = other.mEnd_;
            other.mFirst_ = nullptr;
            other.mLast_ = nullptr;
            other.mEnd_ = nullptr;
        }
        else {
            if (other.mFirst_ != other.mLast_) {
                buy_raw(static_cast<size_type>(other.mLast_ - other.mFirst_));
                mLast_ = rstd::uninitialized_move(other.begin(), other.end(), mFirst_);
            }
        }
    }
        
    vector(std::initializer_list<T> init, const Allocator& alloc = Allocator()) : mAlloc_(alloc) {
        construct_range(init.begin(), init.end());
    }
private:
    void tidy() {
        if (mFirst_) {
            rstd::destroy_n(mFirst_, static_cast<size_type>(mLast_ - mFirst_));
            mAlloc_.deallocate(mFirst_, static_cast<size_type>(mEnd_ - mFirst_));
            mFirst_ = nullptr;
            mLast_ = nullptr;
            mEnd_ = nullptr;
        }
    }

public:

    ~vector() { tidy(); }

public:

    template <typename InputIt> 
    void assign_range(InputIt first, InputIt last) {
        const size_type oldSize = rstd::distance(mFirst_, mLast_);
        const size_type newSize = rstd::distance(first, last);
        const size_type oldCapacity = static_cast<size_type>(mEnd_ - mFirst_);
        if (newSize > oldSize) {
            if (oldCapacity < newSize) {
                reallocate(newSize);
            }
            InputIt mid = rstd::next(first, static_cast<difference_type>(mLast_ - mEnd_));
            mLast_ = rstd::copy(first, mid, mFirst_);
            mLast_ = rstd::uninitialized_copy(mid, last, mLast_);
        }
        else {
            rstd::destroy_n(mFirst_ + newSize, oldSize - newSize);
            mLast_ = rstd::copy(first, last, mFirst_);
        }
    }

    template <typename InputIt>
    void move_assign(InputIt first, InputIt last) {
        const size_type oldSize = rstd::distance(mFirst_, mLast_);
        const size_type newSize = rstd::distance(first, last);
        const size_type oldCapacity = static_cast<size_type>(mEnd_ - mFirst_);
        if (newSize > oldSize) {
            if (oldCapacity < newSize) {
                reallocate(newSize);
            }
            InputIt mid = rstd::next(first, static_cast<difference_type>(mLast_ - mEnd_));
            mLast_ = rstd::move(first, mid, mFirst_);
            mLast_ = rstd::uninitialized_move(mid, last, mLast_);
        }
        else {
            rstd::destroy_n(mFirst_ + newSize, oldSize - newSize);
            mLast_ = rstd::move(first, last, mFirst_);
        }
    }

public:
    vector& operator=(const vector& other) {
        if (this != std::addressof(other)) {
            if (mAlloc_ != other.mAlloc_) {
                tidy();
            }
            assign(other.mFirst_, other.mLast_);
        }
        return *this;
    }

    vector& operator=(vector&& other) {
        if (this != std::addressof(other)) {
            if (mAlloc_ != other.mAlloc_) {
                tidy();
                move_assign(other.begin(), other.end());
            }
            else {
                mFirst_ = other.mFirst_;
                mLast_ = other.mLast_;
                mEnd_ = other.mEnd_;
                other.mFirst_ = nullptr;
                other.mLast_ = nullptr;
                other.mEnd_ = nullptr;
            }
        }
        return *this;
    }

    vector& operator=(std::initializer_list<T> init) {
        assign_range(init.begin(), init.end());
        return *this;
    }

    void assign(size_type count, const T& value) {
        const size_type oldSize = rstd::distance(mFirst_, mLast_);
        const size_type newSize = count;
        const size_type oldCapacity = static_cast<size_type>(mEnd_ - mFirst_);
        if (newSize > oldSize) {
            if (oldCapacity < newSize) {
                reallocate(newSize);
                mLast_ = rstd::uninitialized_fill_n(mFirst_, newSize, value);
            } 
            else {
                mLast_ = rstd::fill_n(mFirst_, size(), value);
                mLast_ = rstd::uninitialized_fill_n(mLast_, newSize - oldSize, value);
            }
        }
        else {
            pointer newLast = mFirst_ + newSize;
            mLast_ = rstd::fill_n(mFirst_, count, value);
            rstd::destroy_n(newLast, oldSize - newSize);
            mLast_ = newLast;
        }
    }

    template <typename InputIt, std::enable_if_t<rstd::is_iterator_v<InputIt>, int> = 0>
    void assign(InputIt first, InputIt last) {
        assign_range(first, last);
    }

    void assign(std::initializer_list<T> init) {
        assign_range(init.begin(), init.end());
    }

    allocator_type get_allocator() const { return mAlloc_; }

    RAYFLOW_CPU_GPU reference at(size_type pos) { return mFirst_[pos]; }

    RAYFLOW_CPU_GPU const_reference at(size_type pos) const { return mFirst_[pos]; }

    RAYFLOW_CPU_GPU reference operator[](size_type pos) { return mFirst_[pos]; }

    RAYFLOW_CPU_GPU const_reference operator[](size_type pos) const { return mFirst_[pos]; }

    RAYFLOW_CPU_GPU reference front() { return mFirst_[0]; }

    RAYFLOW_CPU_GPU const_reference front() const { return mFirst_[0]; }

    RAYFLOW_CPU_GPU reference back() { return mLast_[-1]; }

    RAYFLOW_CPU_GPU const_reference back() const { return mLast_[-1]; }

    RAYFLOW_CPU_GPU pointer data() { return mFirst_; }

    RAYFLOW_CPU_GPU const_pointer data() const { return mFirst_; }

    RAYFLOW_CPU_GPU iterator begin() { return iterator(mFirst_); }

    RAYFLOW_CPU_GPU const_iterator begin() const { return const_iterator(mFirst_); }

    RAYFLOW_CPU_GPU const_iterator cbegin() const { return const_iterator(mFirst_); }

    RAYFLOW_CPU_GPU iterator end() { return iterator(mLast_); }

    RAYFLOW_CPU_GPU const_iterator end() const { return const_iterator(mLast_); }

    RAYFLOW_CPU_GPU const_iterator cend() const { return const_iterator(mLast_); }

    RAYFLOW_CPU_GPU reverse_iterator rbegin() { return reverse_iterator(end()); }

    RAYFLOW_CPU_GPU const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    RAYFLOW_CPU_GPU const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }

    RAYFLOW_CPU_GPU reverse_iterator rend() { return reverse_iterator(begin()); }

    RAYFLOW_CPU_GPU const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    RAYFLOW_CPU_GPU const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    RAYFLOW_CPU_GPU bool empty() const { return mFirst_ == mLast_; }

    RAYFLOW_CPU_GPU size_type size() const { return static_cast<size_type>(mLast_ - mFirst_); }

    RAYFLOW_CPU_GPU size_type capacity() const { return static_cast<size_type>(mEnd_ - mFirst_); }

    RAYFLOW_CPU_GPU constexpr size_type max_size() const { return (size_t) - 1; }

    void reserve(size_type n) {
        if (n > capacity()) {
            reallocate_exactly(n);
        }
    }

    void shrink_to_fit() {
        if (mLast_ != mEnd_) {
            if (mFirst_ == mLast_) {
                tidy();
            }
            else {
                reallocate_exactly(size());
            }
        }
    }

    void clear() {
        rstd::destroy_n(mFirst_, size());
        mLast_ = mFirst_;
    }

private:

    template <typename InputIt>
    iterator insert_range(InputIt first, InputIt last, const_iterator cpos) {
        iterator pos = mFirst_ + (cpos - mFirst_);
        const size_type count = rstd::distance(first, last);
        const size_type posOffset = static_cast<size_type>(pos - mFirst_);
        const size_type unusedCapacity = static_cast<size_type>(mEnd_ - mLast_);
        if (count == 0) {
        }
        else if (count > unusedCapacity) {
            size_type oldSize = size();
            size_type newSize = oldSize + count;
            size_type newCapacity = calculateGrowth(newSize);
            pointer newVec = mAlloc_.allocate(newCapacity);

            pointer mid = newVec + posOffset;
            rstd::uninitialized_copy(first, last, mid);
            rstd::uninitialized_move(mFirst_, pos, newVec);
            rstd::uninitialized_move(pos, mLast_, mid + count);

            change_array(newVec, newSize, newCapacity);
        }
        else {
            pointer oldLast = mLast_;
            const size_type affectElements = static_cast<size_type>(mLast_ - pos);

            if (affectElements < count) {
                InputIt splitPos = first + affectElements;
                mLast_ = rstd::uninitialized_move(pos, mLast_, pos + count);
                rstd::destroy_n(pos, affectElements);
                rstd::uninitialized_copy(first, last, pos);
            }
            else {
                mLast_ = rstd::uninitialized_move(oldLast - count, oldLast, oldLast);
                rstd::move_backward(pos, oldLast - count, oldLast);
                rstd::destroy_n(pos, count);
                rstd::uninitialized_copy(first, last, pos);
            }
        }
        return mFirst_ + posOffset;
    }

public:
    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator cpos, size_type count, const T& value) {
        iterator pos = mFirst_ + (cpos - mFirst_);
        const size_type posOffset = static_cast<size_type>(pos - mFirst_);
        const size_type unusedCapacity = static_cast<size_type>(mEnd_ - mLast_);
        if (count == 0) {
        }
        else if (count > unusedCapacity) {
            size_type oldSize = size();
            size_type newSize = oldSize + count;
            size_type newCapacity = calculateGrowth(newSize);
            pointer newVec = mAlloc_.allocate(newCapacity);

            pointer mid = newVec + posOffset;
            rstd::uninitialized_fill_n(mid, count, value);
            rstd::uninitialized_move(mFirst_, pos, newVec);
            rstd::uninitialized_move(pos, mLast_, mid + count);

            change_array(newVec, newSize, newCapacity);
        }
        else {
            pointer oldLast = mLast_;
            const size_type affectElements = static_cast<size_type>(mLast_ - pos);

            if (affectElements < count) {
                mLast_ = rstd::uninitialized_fill_n(oldLast, count - affectElements, value);
                mLast_ = rstd::uninitialized_move(pos, mLast_, pos + count);
                rstd::fill_n(pos, affectElements, value);
            }
            else {
                mLast_ = rstd::uninitialized_move(oldLast - count, oldLast, oldLast);
                rstd::move_backward(pos, oldLast - count, oldLast);
                rstd::fill_n(pos, count, value);
            }
        }
        return mFirst_ + posOffset;
    }

    template <typename InputIt, std::enable_if_t<rstd::is_iterator_v<InputIt>, int> = 0>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        return insert_range(first, last, pos);
    }

    iterator insert(const_iterator pos, std::initializer_list<T> init) {
        return insert_range(init.begin(), init.end(), pos);
    }

    iterator erase(const_iterator pos) {
        iterator xPos = mFirst_ + (pos - mFirst_);
        rstd::move(xPos + 1, mLast_, xPos);
        rstd::destroy_at(mLast_ - 1);
        --mLast_;
        return xPos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        iterator xFirst = mFirst_ + (first - mFirst_);
        iterator xLast = mFirst_ + (last - mFirst_);

        iterator new_last = rstd::move(xLast, mLast_, xFirst);
        rstd::destroy(new_last, mLast_);
        mLast_ = new_last;
        return xFirst;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

private:

    template <typename... Args>
    reference emplace_back_with_unused_capacity(Args&&... args) {
        const pointer oldLast = mLast_;
        mAlloc_.construct(oldLast, std::forward<Args>(args)...);
        ++mLast_;
        return *oldLast;
    }

    template <typename... Args>
    iterator emplace_reallocate(const_iterator cpos, Args&&... args) {
        iterator pos = mFirst_ + (cpos - mFirst_);
        const size_type posOffset = static_cast<size_type>(pos - mFirst_);
        const size_type oldSize = static_cast<size_type>(mLast_ - mFirst_);

        const size_type newSize = oldSize + 1;
        const size_type newCapacity = calculateGrowth(newSize);
        pointer newVec = mAlloc_.allocate(newCapacity);
        
        mAlloc_.construct(newVec + posOffset, std::forward<Args>(args)...);
        uninitialized_move(mFirst_, pos, newVec);
        uninitialized_move(pos, mLast_, newVec + posOffset + 1);

        change_array(newVec, newSize, newCapacity);
        return newVec + posOffset;
    }

public:

    template <typename... Args> 
    iterator emplace(const_iterator pos, Args&&... args) {
        iterator xPos = mFirst_ + (pos - mFirst_);
        if (mLast_ != mEnd_) {
            if (xPos == mLast_) {
                emplace_back_with_unused_capacity(std::forward<Args>(args)...);
            }
            else {
                mAlloc_.construct(mLast_, std::move(*(mLast_ - 1)));
                rstd::move_backward(xPos, mLast_ - 1, mLast_);
                mAlloc_.construct(xPos, std::forward<Args>(args)...);
                ++mLast_;
            }
            return xPos;
        }
        return emplace_reallocate(xPos, std::forward<Args>(args)...);
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (mLast_ != mEnd_) {
            return emplace_back_with_unused_capacity(std::forward<Args>(args)...);
        }
        reference result = *emplace_reallocate(mLast_, std::forward<Args>(args)...);
        return result;
    }

    void pop_back() {
        if (mFirst_) {
            if (mFirst_ != mLast_) {
                rstd::destroy_at(mLast_ - 1);
                --mLast_;
            }
        }
    }

    void resize(size_type count) {
        resize(count, T());
    }

    void resize(size_type count, const value_type& value) {
        const size_type oldSize = static_cast<size_type>(mLast_ - mFirst_);
        const size_type oldCapacity = static_cast<size_type>(mEnd_ - mFirst_);
        if (count == oldSize) { return; }

        if (count > oldSize) {
            const size_type newSize = count;
            if (oldCapacity < count) {
                size_type newCapacity = calculateGrowth(count);
                pointer newVec = mAlloc_.allocate(newCapacity);
                rstd::uninitialized_move(mFirst_, mLast_, newVec);
                rstd::uninitialized_fill_n(newVec + oldSize, newSize - oldSize, value);
                change_array(newVec, newSize, newCapacity);
            }
            else {
                mLast_ = rstd::uninitialized_fill_n(mLast_, newSize - oldSize, value);
            }
        }
        else {
            rstd::destroy(mFirst_ + count, mLast_);
            mLast_ = mFirst_ + count;
        }
    }

    void swap(vector& other) {
        std::swap(mFirst_, other.mFirst_);
        std::swap(mLast_, other.mLast_);
        std::swap(mEnd_, other.mEnd_);
    }

private:

    void change_array(pointer newVec, size_type newSize, size_type newCapacity) {
        if (mFirst_) {
            rstd::destroy(mFirst_, mLast_);
            mAlloc_.deallocate(mFirst_, capacity());
        }
        mFirst_ = newVec;
        mLast_ = newVec + newSize;
        mEnd_ = newVec + newCapacity;
    }

    void buy_raw(size_type newCapacity) {
        pointer newVec = static_cast<iterator>(mAlloc_.allocate(newCapacity));
        mFirst_ = newVec;
        mLast_ = newVec;
        mEnd_ = newVec + newCapacity;
    }

    void reallocate(size_type newSize) {
        size_type newCapacity = calculateGrowth(newSize);

        if (mFirst_) {
            rstd::destroy_n(mFirst_, static_cast<size_type>(mLast_ - mFirst_));
            mAlloc_.deallocate(mFirst_, static_cast<size_type>(mEnd_ - mFirst_));

            mFirst_ = nullptr;
            mLast_ = nullptr;
            mEnd_ = nullptr;
        }

        buy_raw(newCapacity);
    }

    void reallocate_exactly(size_type newCapacity) {
        pointer newVec = mAlloc_.allocate(newCapacity);

        rstd::uninitialized_move(mFirst_, mLast_, newVec);
        
        change_array(newVec, size(), newCapacity);
    }   

    size_type calculateGrowth(size_type newSize) {
        const size_type oldcapacity = capacity();
        const auto maxSize = max_size();

        if (oldcapacity > maxSize - oldcapacity / 2) {
            return maxSize;
        }

        const size_type tmpCapacity = oldcapacity + oldcapacity / 2;

        if (tmpCapacity < newSize) {
            return newSize;
        }

        return tmpCapacity;
    }
private:
    Allocator mAlloc_;
    pointer mFirst_ = nullptr;
    pointer mLast_ = nullptr;
    pointer mEnd_ = nullptr;
};

template <typename T, typename Alloc>
inline bool operator==(const rstd::vector<T, Alloc>& lhs, const rstd::vector<T, Alloc>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (auto i = 0; i < lhs.size(); ++i) {
        if (!(lhs[i] == rhs[i])) {
            return false;
        }
    }
    return true;
}
}
}