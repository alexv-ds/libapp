// -*- C++ -*-
//
// Copyright 2025 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_BASE_MATH_HPP
#define DMITIGR_BASE_MATH_HPP

#include "assert.hpp"

#include <stdexcept>
#include <utility>

namespace dmitigr::math {

// -----------------------------------------------------------------------------
// Alignment
// -----------------------------------------------------------------------------

/// @returns `true` if `number` is a power of 2.
template<typename T>
constexpr bool is_power_of_two(const T number) noexcept
{
  return (number & (number - 1)) == 0;
}

/**
 * @returns The size of padding.
 *
 * @param value A value for which a padding need to be calculated.
 * @param alignment An aligment to calculated the padding.
 *
 * @par Requires
 * `(value >= 0 && is_power_of_two(alignment))`.
 */
template<typename T>
constexpr auto padding(const T value, const T alignment)
{
  if (!(value >= 0))
    throw std::invalid_argument{"cannot calculate padding for a negative value"};
  else if (!is_power_of_two(alignment))
    throw std::invalid_argument{"cannot calculate padding with alignment that is not "
      "power of 2"};
  return (static_cast<T>(0) - value) & static_cast<T>(alignment - 1);
}

/**
 * @returns The value aligned by using `alignment`.
 *
 * @par Requires
 * `(value >= 0 && is_power_of_two(alignment))`.
 */
template<typename T>
constexpr T aligned(const T value, const T alignment)
{
  if (!(value >= 0))
    throw std::invalid_argument{"cannot align a negative value"};
  else if (!is_power_of_two(alignment))
    throw std::invalid_argument{"cannot align a value with alignment that is not "
      "power of 2"};
  return (value + (alignment - 1)) & -alignment;
}

// -----------------------------------------------------------------------------
// Interval
// -----------------------------------------------------------------------------

/// Represents a type of interval.
enum class Interval_type {
  /// Denotes [min, max] interval.
  closed,
  /// Denotes (min, max) interval.
  open,
  /// Denotes (min, max] interval.
  lopen,
  /// Denotes [min, max) interval.
  ropen
};

/// Represents an interval.
template<typename T>
class Interval final {
public:
  /// An alias of T.
  using Value_type = T;

  /// An alias of Interval_type.
  using Type = Interval_type;

  /// Constructs closed [{},{}] interval.
  Interval() = default;

  /**
   * @brief Constructs closed [min, max] interval.
   *
   * @par Requires
   * `min <= max`.
   */
  explicit Interval(T min, T max)
    : type_{Type::closed}
    , min_{std::move(min)}
    , max_{std::move(max)}
  {
    if (!(min_ <= max_))
      throw std::invalid_argument{"interval is invalid (min > max)"};
  }

  /**
   * @brief Constructs the interval of the specified type.
   *
   * @par Requires
   * `(type == Type::closed && min <= max) || (type != Type::closed && min < max)`.
   */
  explicit Interval(const Type type, T min, T max)
    : type_{type}
    , min_{std::move(min)}
    , max_{std::move(max)}
  {
    const bool requirement = (type_ == Type::closed && min_ <= max_) ||
      (type_ != Type::closed && min_ < max_);
    if (!requirement)
      throw std::invalid_argument{"interval is invalid (min > max or min >= max)"};
  }

  /// @returns [min, max] interval.
  static Interval make_closed(T min, T max)
  {
    return {Type::closed, std::move(min), std::move(max)};
  }

  /// @returns (min, max) interval.
  static Interval make_open(T min, T max)
  {
    return {Type::open, std::move(min), std::move(max)};
  }

  /// @returns (min, max] interval.
  static Interval make_lopen(T min, T max)
  {
    return {Type::lopen, std::move(min), std::move(max)};
  }

  /// @returns [min, max) interval.
  static Interval make_ropen(T min, T max)
  {
    return {Type::ropen, std::move(min), std::move(max)};
  }

  /// @returns The type of interval.
  Type type() const noexcept
  {
    return type_;
  }

  /// @returns The minimum of interval.
  const T& min() const noexcept
  {
    return min_;
  }

  /// @returns The maximum of interval.
  const T& max() const noexcept
  {
    return max_;
  }

  /// @returns `true` if value belongs to interval, or `false` otherwise.
  bool has(const T& value) const noexcept
  {
    switch (type_) {
    case Type::closed: return (min_ <= value) && (value <= max_); // []
    case Type::open:   return (min_ <  value) && (value <  max_); // ()
    case Type::lopen:  return (min_ <  value) && (value <= max_); // (]
    case Type::ropen:  return (min_ <= value) && (value <  max_); // [)
    }
    DMITIGR_ASSERT(false);
  }

  /**
   * @returns A pair of {min, max}.
   *
   * @par Effects
   * The state of this instance as if it default constructed.
   */
  std::pair<T, T> release() noexcept
  {
    std::pair<T, T> result{std::move(min_), std::move(max_)};
    *this = {};
    return result;
  }

private:
  Type type_{Type::closed};
  T min_{};
  T max_{};
};

// -----------------------------------------------------------------------------
// Statistic
// -----------------------------------------------------------------------------

/**
 * @returns An average of values.
 *
 * @param data Input data
 */
template<class Container>
constexpr double avg(const Container& data) noexcept
{
  double result{};
  const auto data_size = data.size();
  for (const double num : data)
    result += (num / static_cast<double>(data_size));
  return result;
}

/**
 * @returns A variance of values.
 *
 * @param data Input data.
 * @param avg An average of `data`.
 * @param general Is the `data` represents general population?
 */
template<class Container>
constexpr double variance(const Container& data, const double avg,
  const bool general = true) noexcept
{
  const auto den = static_cast<double>(data.size() - !general);
  double result{};
  for (const double num : data) {
    const double d = num - avg;
    result += (d / den) * d; // (d * d) / den
  }
  DMITIGR_ASSERT(result >= 0);
  return result;
}

/// @overload
template<class Container>
constexpr double variance(const Container& data,
  const bool general = true) noexcept
{
  return variance(data, avg(data), general);
}

} // namespace dmitigr::math

#endif  // DMITIGR_BASE_MATH_HPP
