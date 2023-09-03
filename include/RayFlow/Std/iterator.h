#pragma once

#include <type_traits>
#include <iterator>

namespace rayflow {

namespace rstd {

template <typename Iter>
using iterator_cat_t = typename std::iterator_traits<Iter>::iterator_category;

template <typename Iter>
inline constexpr bool is_input_iterator_v = std::is_convertible_v<iterator_cat_t<Iter>, std::input_iterator_tag>;

template <typename Iter>
inline constexpr bool is_output_iterator_v = std::is_convertible_v<iterator_cat_t<Iter>, std::output_iterator_tag>;

template <typename Iter>
inline constexpr bool is_forward_iterator_v = std::is_convertible_v<iterator_cat_t<Iter>, std::forward_iterator_tag>;

template <typename Iter>
inline constexpr bool is_bidirectional_iterator_v = std::is_convertible_v<iterator_cat_t<Iter>, std::bidirectional_iterator_tag>;

template <typename Iter>
inline constexpr bool is_random_access_iterator_v = std::is_convertible_v<iterator_cat_t<Iter>, std::random_access_iterator_tag>;

template <typename Iter, typename T = void>
inline constexpr bool is_iterator_v = false;

template <typename Iter>
inline constexpr bool is_iterator_v<Iter, std::void_t<iterator_cat_t<Iter>>> = true;

template <typename Iter>
struct is_iterator : public std::bool_constant<is_iterator_v<Iter>> {};

}
}