#pragma once

#include <type_traits>
#include <iterator>
#include <utility>
#include <memory>

#include <RayFlow/Std/algorithm.h>

namespace rayflow {

namespace rstd {
    
// https://en.cppreference.com/w/cpp/numeric/bit_cast
template <class To, class From>
std::enable_if_t<
    sizeof(To) == sizeof(From) &&
    std::is_trivially_copyable_v<From> &&
    std::is_trivially_copyable_v<To>,
    To>
bit_cast(const From& src) noexcept
{
    static_assert(std::is_trivially_constructible_v<To>,
        "This implementation additionally requires "
        "destination type to be trivially constructible");
 
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

template <typename InputIt>
constexpr
InputIt next(InputIt it, typename std::iterator_traits<InputIt>::difference_type n = 1) {
    for (; n > 0; ++it, --n);
    return it;
}

template <typename InputIt>
typename std::iterator_traits<InputIt>::difference_type
    distance_cat(InputIt first, InputIt last, 
                 std::input_iterator_tag) {
    using diff_type = typename std::iterator_traits<InputIt>::difference_type;
    diff_type dis = 0;
    for (; first != last; ++first, ++dis);
    return dis;
}

template <typename RandomIt>
typename std::iterator_traits<RandomIt>::difference_type
    distance_cat(RandomIt first, RandomIt last, 
                 std::random_access_iterator_tag) {
    using diff_type = typename std::iterator_traits<RandomIt>::difference_type;
    return static_cast<diff_type>(last - first);
}

template <typename InputIt>
typename std::iterator_traits<InputIt>::difference_type
distance(InputIt first, InputIt last) {
    return distance_cat(first, last, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename T>
void destroy_at(T* p) {
    if constexpr (std::is_array_v<T>) {
        for (auto &elem : *p) {
            rstd::destroy_at(std::addressof(elem));
        }
    }
    else {
        p->~T();
    }
}

template <typename ForwardIt>
void destroy(ForwardIt first, ForwardIt last) {
    for (; first != last; ++first) {
        rstd::destroy_at(std::addressof(*first));
    }
}

template <typename ForwardIt, typename Size>
ForwardIt destroy_n(ForwardIt first, Size n) {
    for (; n > 0; ++first, n--) {
        rstd::destroy_at(std::addressof(*first));
    }
    return first;
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_copy_unchecked(InputIt first, InputIt last, ForwardIt d_first, std::true_type) {
    return rstd::copy(first, last, d_first);
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_copy_unchecked(InputIt first, InputIt last, ForwardIt d_first, std::false_type) {
    ForwardIt current = d_first;
    try {
        for (; first != last; ++first, ++current) {
            ::new (static_cast<void*>(std::addressof(*current))) (typename std::iterator_traits<ForwardIt>::value_type)(*first);
        }
    }
    catch (...) {
        rstd::destroy(d_first, current);
    }
    return current;
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_copy(InputIt first, InputIt last, ForwardIt d_first) {
    return uninitialized_copy_unchecked(first, last, d_first, 
                                        std::is_trivially_copy_constructible<
                                        typename std::iterator_traits<ForwardIt>::value_type>{});
}

template <typename InputIt, typename Size, typename ForwardIt>
ForwardIt uninitialized_copy_n_unchecked(InputIt first, Size count, ForwardIt d_first, std::true_type) {
    return rstd::copy_n(first, count, d_first);
}

template <typename InputIt, typename Size, typename ForwardIt>
ForwardIt uninitialized_copy_n_unchecked(InputIt first, Size count, ForwardIt d_first, std::false_type) {
    ForwardIt current = d_first;
    try {
        for (; count > 0; ++first, ++current, --count) {
            ::new (static_cast<void*>(std::addressof(*current))) (typename std::iterator_traits<ForwardIt>::value_type)(*first);
        }
    }
    catch (...) {
        rstd::destroy(d_first, current);
    }
    return current;
}

template <typename InputIt, typename Size, typename ForwardIt>
ForwardIt uninitialized_copy_n(InputIt first, Size count, ForwardIt d_first) {
    return uninitialized_copy_n_unchecked(first, count, d_first,
                                          std::is_trivially_copy_constructible<
                                          typename std::iterator_traits<ForwardIt>::value_type>{});
}

template <typename ForwardIt, typename T>
void uninitialized_fill_unchecked(ForwardIt first, ForwardIt last, const T& value, std::true_type) {
    rstd::fill(first, last, value);
}

template <typename ForwardIt, typename T>
void uninitialized_fill_unchecked(ForwardIt first, ForwardIt last, const T& value, std::false_type) {
    ForwardIt current = first;
    try {
        for (; current != last; ++current) {
            ::new (static_cast<void*>(std::addressof(*current))) (typename std::iterator_traits<ForwardIt>::value_type)(value);
        }
    }
    catch (...) {
        std::destroy(first, last);
    }
}

template <typename ForwardIt, typename T>
void uninitialized_fill(ForwardIt first, ForwardIt last, const T& value) {
    uninitialized_fill_unchecked(first, last, value,
                                 std::is_trivially_copy_assignable<
                                 typename std::iterator_traits<ForwardIt>::value_type>{});
}

template <typename ForwardIt, typename Size, typename T>
ForwardIt uninitialized_fill_n_unchecked(ForwardIt first, Size count, const T& value, std::true_type) {
    return rstd::fill_n(first, count, value);
}

template <typename ForwardIt, typename Size, typename T>
ForwardIt uninitialized_fill_n_unchecked(ForwardIt first, Size count, const T& value, std::false_type) {
    ForwardIt current = first;
    try {
        for (; count > 0; --count, ++current) {
            ::new (static_cast<void*>(std::addressof(*current))) (typename std::iterator_traits<ForwardIt>::value_type)(value);
        }
    }
    catch (...) {
        rstd::destroy(first, current);
    }
    return current;
}

template <typename ForwardIt, typename Size, typename T>
ForwardIt uninitialized_fill_n(ForwardIt first, Size count, const T& value) {
    return uninitialized_fill_n_unchecked(first, count, value, 
                                          std::is_trivially_copy_assignable<
                                          typename std::iterator_traits<ForwardIt>::value_type>{});
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_move_unchecked(InputIt first, InputIt last, ForwardIt d_first, std::true_type) {
    return rstd::move(first, last, d_first);
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_move_unchecked(InputIt first, InputIt last, ForwardIt d_first, std::false_type) {
    ForwardIt current = d_first;
    try {
        for (; first != last; ++first, ++current) {
            ::new (static_cast<void*>(std::addressof(*current))) (typename std::iterator_traits<ForwardIt>::value_type)(*first);
        }
    }
    catch (...) {
        rstd::destroy(d_first, current);
    }
    return current;
}

template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt d_first) {
    return uninitialized_move_unchecked(first, last, d_first,
                                        std::is_trivially_move_assignable<
                                        typename std::iterator_traits<ForwardIt>::value_type>{});
}

template <typename InputIt, typename Size, typename ForwardIt>
std::pair<InputIt, ForwardIt> uninitialized_move_n_unchecked(InputIt first, Size count, ForwardIt d_first, std::true_type) {
    return std::make_pair<InputIt, ForwardIt>(first + count, rstd::move(first, first + count, d_first));
}

template <typename InputIt, typename Size, typename ForwardIt>
std::pair<InputIt, ForwardIt> uninitialized_move_n_unchecked(InputIt first, Size count, ForwardIt d_first, std::false_type) {
    ForwardIt current = d_first;
    try {
        for (; count > 0; ++first, ++current, --count) {
            ::new (static_cast<void*>(std::addressof(*current))) (typename std::iterator_traits<ForwardIt>::value_type)(std::move(*first));
        }
    }
    catch (...) {
        rstd::destroy(d_first, current);
    }
    return std::pair<InputIt, ForwardIt>(first + count, current);
}

template <typename InputIt, typename Size, typename ForwardIt>
std::pair<InputIt, ForwardIt> uninitialized_move_n(InputIt first, Size count, ForwardIt d_first) {
    return uninitialized_move_n_unchecked(first, count, d_first,
                                          std::is_trivially_move_assignable<
                                          typename std::iterator_traits<ForwardIt>::value_type>{});
}

}

}