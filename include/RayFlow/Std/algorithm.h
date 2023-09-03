#pragma once

namespace rayflow {

namespace rstd {

template <typename InputIt, typename T>
InputIt find_cat(InputIt first, InputIt last, const T& value, std::input_iterator_tag) {
    for (; first != last; ++first) {
        if (*first == value) {
            return first;
        }
    }
    return last;
}

template <typename RandomIt, typename T>
RandomIt find_cat(RandomIt first, RandomIt last, const T& value, std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (*first == value) {
            return first;
        }
    }
    return last;
}

template <typename InputIt, typename T>
InputIt find(InputIt first, InputIt last, const T& value) {
    return find_cat(first, last, value, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename InputIt, typename UnaryPredicate>
InputIt find_if_cat(InputIt first, InputIt last, UnaryPredicate p, std::input_iterator_tag) {
    for (; first != last; ++first) {
        if (p(*first)) {
            return first;
        }
    }
    return last;
}

template <typename RandomIt, typename UnaryPredicate>
RandomIt find_if_cat(RandomIt first, RandomIt last, UnaryPredicate p, std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (p(*first)) {
            return first;
        }
    }
    return last;
}

template <typename InputIt, typename UnaryPredicate>
InputIt find_if(InputIt first, InputIt last, UnaryPredicate p) {
    return find_if_cat(first, last, p, typename std::iterator_traits<Input>::iterator_category());
}

template <typename InputIt, typename OutputIt>
OutputIt copy_unchecked_cat(InputIt first, InputIt last, OutputIt d_first, std::input_iterator_tag) {
    for (; first != last; ++first, ++d_first) {
        *d_first = *first;
    }
    return d_first;
}

template <typename RandomIt, typename OutputIt>
OutputIt copy_unchecked_cat(RandomIt first, RandomIt last, OutputIt d_first, std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, ++d_first, --count) {
        *d_first = *first;
    }
    return d_first; 
}

template <typename InputIt, typename OutputIt>
OutputIt copy_unchecked(InputIt first, InputIt last, OutputIt d_first) {
    return copy_unchecked_cat(first, last, d_first, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename Tp, typename Up>
std::enable_if_t<std::is_same_v<typename std::remove_const_t<Tp>, Up> &&
                 std::is_trivially_copy_assignable_v<Up>, Up*>
copy_unchecked(Tp* first, Tp* last, Up* d_first) {
    size_t count = last - first;
    if (count != 0) {
        std::memmove(d_first, first, count * sizeof(Up));
    }
    return d_first + count;
}

template <typename InputIt, typename OutputIt>
OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
    return copy_unchecked(first, last, d_first);
}

template <typename InputIt, typename OutputIt, typename UnaryPredicate>
OutputIt copy_if_cat(InputIt first, InputIt last,
                     OutputIt d_first,
                     UnaryPredicate pred,
                     std::input_iterator_tag) {
    for (; first != last; ++first) {
        if (pred(*first)) {
            *d_first = *first;
            ++d_first;
        }
    }
    return d_first;
}

template <typename RandomIt, typename OutputIt, typename UnaryPredicate>
OutputIt copy_if_cat(RandomIt first, RandomIt last,
                     OutputIt d_first,
                     UnaryPredicate pred,
                     std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (pred(*first)) {
            *d_first = *first;
            ++d_first;
        }
    }
    return d_first;
}

template <typename InputIt, typename OutputIt, typename UnaryPredicate>
OutputIt copy_if(InputIt first, InputIt last,
                 OutputIt d_first,
                 UnaryPredicate pred) {
    return copy_if_cat(first, last, d_first, pred, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename InputIt, typename Size, typename OutputIt>
OutputIt copy_n_cat(InputIt first, Size count, OutputIt d_first, std::input_iterator_tag) {
    for (; count > 0; ++first, ++d_first, --count) {
        *d_first = *first;
    }
    return d_first;
}

template <typename RandomIt, typename Size, typename OutputIt>
OutputIt copy_n_cat(RandomIt first, Size count, OutputIt d_first, std::random_access_iterator_tag) {
    RandomIt last = first + count;
    rstd::copy(first, last, d_first);
    return first + count;
}

template <typename InputIt, typename Size, typename OutputIt>
OutputIt copy_n(InputIt first, Size count, OutputIt d_first) {
    return copy_n_cat(first, count, d_first, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename BidirIt1, typename BidirIt2>
BidirIt2 copy_backward_unchecked_cat(BidirIt1 first, BidirIt1 last, BidirIt2 d_last, std::bidirectional_iterator_tag) {
    for (; first != last;) {
        *(--d_last) = *(--last);
    }
    return d_last;
}

template <typename RandomIt, typename BidirIt>
BidirIt copy_backward_unchecked_cat(RandomIt first, RandomIt last, BidirIt d_last, std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; --count) {
        *(--d_last) = *(--last);
    }
    return d_last;
}

template <typename BidirIt1, typename BidirIt2>
BidirIt2 copy_backward_unchecked(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    return copy_backward_unchecked_cat(first, last, d_last, typename std::iterator_traits<BidirIt1>::iterator_catrgory());
}

template <typename Tp, typename Up>
std::enable_if_t<std::is_same_v<typename std::remove_const_t<Tp>, Up> &&
                 std::is_trivially_copy_assignable_v<Up>, Up*>
copy_backward_unchecked(Tp* first, Tp* last, Up* d_last) {
    auto count = last - first;
    if (count > 0) {
        d_last -= count;
        std::memmove(d_last, first, count * sizeof(Up));
    }
    return d_last;
}

template <typename BidirIt1, typename BidirIt2>
BidirIt2 copy_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    return copy_backward_unchecked(first, last, d_last);
}

template <typename InputIt, typename OutputIt>
OutputIt move_unchecked_cat(InputIt first, InputIt last, OutputIt d_first, std::input_iterator_tag) {
    for (; first != last; ++first, ++d_first) {
        *d_first = std::move(*first);
    }
    return d_first;
}

template <typename RandomIt, typename OutputIt>
OutputIt move_unchecked_cat(RandomIt first, RandomIt last, OutputIt d_first, std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, ++d_first, --count) {
        *d_first = std::move(*first);
    }
    return d_first;
}

template <typename InputIt, typename OutputIt>
OutputIt move_unchecked(InputIt first, InputIt last, OutputIt d_first) {
    return move_unchecked_cat(first, last, d_first, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename Tp, typename Up>
std::enable_if_t<std::is_same_v<typename std::remove_const_t<Tp>, Up> &&
                 std::is_trivially_move_assignable_v<Up>, Up*>
move_unchecked(Tp* first, Tp* last, Up* d_first) {
    auto count = last - first;
    if (count > 0) {
        std::memmove(d_first, first, count * sizeof(Up));
    }
    return d_first + count;
}

template <typename InputIt, typename OutputIt>
OutputIt move(InputIt first, InputIt last, OutputIt d_first) {
    return move_unchecked(first, last, d_first);
}

template <typename BidirIt1, typename BidirIt2>
BidirIt2 move_backward_unchecked_cat(BidirIt1 first, BidirIt1 last, BidirIt2 d_last, std::bidirectional_iterator_tag) {
    for (; first != last; ) {
        *(--d_last) = std::move(*(--last));
    }
    return d_last;
}

template <typename RandomIt, typename BidirIt>
BidirIt move_backward_unchecked_cat(RandomIt first, RandomIt last, BidirIt d_last, std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; --count) {
        *(--d_last) = std::move(*(--last));
    }
    return d_last;
}

template <typename BidirIt1, typename BidirIt2>
BidirIt2 move_backward_unchecked(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    return move_backward_unchecked_cat(first, last, d_last, typename std::iterator_traits<BidirIt1>::iterator_category());
}

template <typename Tp, typename Up>
std::enable_if_t<std::is_same_v<typename std::remove_const_t<Tp>, Up> &&
                 std::is_trivially_move_assignable_v<Up>, Up*>
move_backward_unchecked(Tp* first, Tp* last, Up* d_last) {
    auto count = last - first;
    if (count > 0) {
        d_last -= count;
        std::memmove(d_last, first, count * sizeof(Up));
    }
    return d_last;
}

template <typename BidirIt1, typename BidirIt2 >
BidirIt2 move_backward(BidirIt1 first, BidirIt1 last, BidirIt2 d_last) {
    return move_backward_unchecked(first, last, d_last);
}

template <typename OutputIt, typename Size, typename T>
OutputIt fill_n_unchecked(OutputIt first, Size count, const T& value) {
    for (; count > 0; ++first, --count) {
        *first = value;
    }
    return first;
}

template <typename Tp, typename Size, typename T>
std::enable_if_t<std::is_integral<Tp>::value && sizeof(Tp) == 1 &&
                 !std::is_same<Tp, bool>::value &&
                 std::is_integral<T>::value && sizeof(T) == 1, Tp*>
fill_n_unchecked(Tp* first, Size count, const T& value) {
    if (count > 0) {
        std::memset(first, static_cast<unsigned char>(value), count);
    }
    return first + count;
}

template <typename OutputIt, typename Size, typename T>
OutputIt fill_n(OutputIt first, Size count, const T& value) {
    return fill_n_unchecked(first, count, value);
}

template <typename ForwardIt, typename T>
void fill_cat(ForwardIt first, ForwardIt last, const T& value, std::forward_iterator_tag) {
    for (; first != last; ++first) {
        *first = value;
    }
}

template <typename RandomIt, typename T>
void fill_cat(RandomIt first, RandomIt last, const T& value, std::random_access_iterator_tag) {
    fill_n(first, last, value);
}

template <typename ForwardIt, typename T>
void fill(ForwardIt first, ForwardIt last, const T& value) {
    fill_cat(first, last, value, typename std::iterator_traits<ForwardIt>::iterator_category());
}

template <typename ForwardIt, typename T>
ForwardIt remove_cat(ForwardIt first, ForwardIt last, const T& value, std::forward_iterator_tag) {
    first = rstd::find(first, last, value);
    for (auto it = first; it != last; ++it) {
        if (!(*it == value)) {
            *(first++) = std::move(*it);
        }
    }
    return first;
}

template <typename RandomIt, typename T>
RandomIt remove_cat(RandomIt first, RandomIt last, const T& value, std::random_access_iterator_tag) {
    first = rstd::find(first, last, value);
    auto count = last - first;
    for (auto it = first; count > 0; ++it, --count) {
        if (!(*it == value)) {
            *(first++) = std::move(*it);
        }
    }
    return first;
}

template <typename ForwardIt, typename T>
ForwardIt remove(ForwardIt first, ForwardIt last, const T& value) {
    return remove_cat(first, last, value, typename std::iterator_traits<ForwardIt>::iterator_category());
}

template <typename ForwardIt, typename UnaryPredicate>
ForwardIt remove_if_cat(ForwardIt first, ForwardIt last, UnaryPredicate p, std::forward_iterator_tag) {
    first = rstd::find_if(first, last, p);
    for (auto it = first; it != last; ++it) {
        if (!p(*it)) {
            *(first++) = std::move(*it);
        }
    }
    return first;
}

template <typename ForwardIt, typename UnaryPredicate>
ForwardIt remove_if_cat(ForwardIt first, ForwardIt last, UnaryPredicate p, std::random_access_iterator_tag) {
    first = rstd::find_if(first, last, p);
    auto count = last - first;
    for (auto it = first; count > 0; ++it, --count) {
        if (!p(*it)) {
            *(first++) = std::move(*it);
        }
    }
    return first;
}

template <typename ForwardIt, typename UnaryPredicate>
ForwardIt remove_if(ForwardIt first, ForwardIt last, UnaryPredicate p) {
    return remove_if_cat(first, last, p, typename std::iterator_traits<ForwardIt>::iterator_category());
}

template <typename InputIt, typename OutputIt, typename T>
OutputIt remove_copy_cat(InputIt first, InputIt last,
                         OutputIt d_first, const T& value,
                         std::input_iterator_tag) {
    for (; first != last; ++first) {
        if (!(*it == value)) {
            *(d_first++) = *first;
        }
    }
    return d_first;
}

template <typename RandomIt, typename OutputIt, typename T>
OutputIt remove_copy_cat(RandomIt first, RandomIt last,
                         OutputIt d_first, const T& value,
                         std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (!(*first == value)) {
            *(d_first++) = *first;
        }
    }
    return d_first;
}


template <typename InputIt, typename OutputIt, typename T>
OutputIt remove_copy(InputIt first, InputIt last,
                     OutputIt d_first, const T& value) {
    return remove_copy_cat(first, last, d_first, value, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename InputIt, typename OutputIt, typename UnaryPredicate>
OutputIt remove_copy_if_cat(InputIt first, InputIt last,
                            OutputIt d_first, UnaryPredicate p,
                            std::input_iterator_tag) {
    for (; first != last; ++first) {
        if (!p(first)) {
            *(d_first++) = *first;
        }
    }
    return d_first;
}

template <typename RandomIt, typename OutputIt, typename UnaryPredicate>
OutputIt remove_copy_if_cat(RandomIt first, RandomIt last,
                            OutputIt d_first, UnaryPredicate p,
                            std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (!p(first)) {
            *(d_first) = *first;
        }
    }
    return d_first;
}

template <typename InputIt, typename OutputIt, typename UnaryPredicate>
OutputIt remove_copy_if(InputIt first, InputIt last,
                        OutputIt d_first, UnaryPredicate p) {
    remove_copy_if_cat(first, last, d_first, p, typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename ForwardIt, typename T>
void replace_cat(ForwardIt first, ForwardIt last,
                 const T& old_value, const T& new_value,
                 std::forward_iterator_tag) {
    for (; first != last; ++first) {
        if (*first == old_value) {
            *first = new_value;
        }
    }
}

template <typename RandomIt, typename T>
void replace_cat(RandomIt first, RandomIt last,
                 const T& old_value, const T& new_value,
                 std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (*first == old_value) {
            *first = new_value;
        }
    }
}

template <typename ForwardIt, typename T>
void replace(ForwardIt first, ForwardIt last,
             const T& old_value, const T& new_value) {
    replace_cat(first, last, old_value, new_value, typename std::iterator_traits<ForwardIt>::iterator_category());
}

template <typename ForwardIt, typename UnaryPredicate, typename T>
void replace_if_cat(ForwardIt first, ForwardIt last,
                UnaryPredicate p, const T& new_value,
                std::forward_iterator_tag) {
    for (; first != last; ++first) {
        if (p(*first)) {
            *first = new_value;
        }
    }
}

template <typename RandomIt, typename UnaryPredicate, typename T>
void replace_if_cat(RandomIt first, RandomIt last,
                    UnaryPredicate p, const T& new_value,
                    std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, --count) {
        if (p(*first)) {
            *first = new_value;
        }
    }
}

template <typename ForwardIt, typename UnaryPredicate, typename T>
void replace_if(ForwardIt first, ForwardIt last,
                UnaryPredicate p, const T& new_value) {
    replace_if_cat(first, last, p, new_value, typename std::iterator_traits<ForwardIt>::iterator_category());
}

template <typename InputIt, typename OutputIt, typename T>
OutputIt replace_copy_cat(InputIt first, InputIt last, OutputIt d_first,
                          const T& old_value, const T& new_value,
                          std::input_iterator_tag) {
    for (; first != last; ++first, ++d_first) {
        *d_first = (*first == old_value) ? new_value : *first;
    }

}

template <typename RandomIt, typename OutputIt, typename T>
OutputIt replace_copy_cat(RandomIt first, RandomIt last, OutputIt d_first,
                          const T& old_value, const T& new_value,
                          std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, ++d_first, --count) {
        *d_first = (*first == old_value) ? new_value : *first;
    }
    return d_first;
}

template <typename InputIt, typename OutputIt, typename T>
OutputIt replace_copy(InputIt first, InputIt last, OutputIt d_first,
                      const T& old_value, const T& new_value) {
    return replace_copy_cat(first, last, d_first, old_value, new_value, 
                            typename std::iterator_traits<InputIt>::iterator_category());
}

template <typename InputIt, typename OutputIt, typename UnaryPredicate, typename T>
OutputIt replace_copy_if_cat(InputIt first, InputIt last, OutputIt d_first,
                             UnaryPredicate p, const T& new_value,
                             std::input_iterator_tag) {
    for (; first != last; ++first, ++d_first) {
        *d_first = p(*first) ? new_value : *first;
    }
    return d_first;
}

template <typename RandomIt, typename OutputIt, typename UnaryPredicate, typename T>
OutputIt replace_copy_if_cat(RandomIt first, RandomIt last, OutputIt d_first,
                             UnaryPredicate p, const T& new_value,
                             std::random_access_iterator_tag) {
    for (auto count = last - first; count > 0; ++first, ++d_first, --count) {
        *d_first = p(*first) ? new_value : *first;
    }
    return *d_first;
}

template <typename InputIt, typename OutputIt, typename UnaryPredicate, typename T>
OutputIt replace_copy_if(InputIt first, InputIt last, OutputIt d_first,
                         UnaryPredicate p, const T& new_value) {
    return replace_copy_if_cat(first, last, d_first, p, new_value,
                               typename std::iterator_traits<InputIt>::iterator_category());
}

}
}