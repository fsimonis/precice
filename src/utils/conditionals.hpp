#pragma once

#include <algorithm>
#include <array>
#include <type_traits>

namespace precice {
namespace utils {

template <int n, typename T>
struct AnyOf {
  template <typename... Args>
  constexpr AnyOf(Args... args)
      : options{args...} {}

  constexpr bool operator==(const T &other) const
  {
    return std::find(options.begin(), options.end(), other) != options.end();
  }

  constexpr bool operator!=(const T &other) const
  {
    return std::find(options.begin(), options.end(), other) == options.end();
  }

  std::array<T, n> options;
};

/// Allows to call T == oneOf(T, T)
template <int n, typename T>
inline constexpr bool operator==(const T &lhs, const AnyOf<n, T> &rhs)
{
  return rhs == lhs;
}

/// Allows to call T != oneOf(T, T)
template <int n, typename T>
inline constexpr bool operator!=(const T &lhs, const AnyOf<n, T> &rhs)
{
  return rhs != lhs;
}

template <typename... Args>
inline constexpr auto anyOf(Args... args)
{
  return AnyOf<sizeof...(Args), std::common_type_t<Args...>>(args...);
}

} // namespace utils
} // namespace precice
