/*
 * MIT License
 *
 * Copyright (c) 2020 Christopher Friedt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 *
 * LibCRC3x - The C++ API for CRCx
 *
 * Yes, it may have been shortsighted to name the library CRCx, because if
 * one appends the C++-typical xx or XX to CRCx..
 *
 * Typical use cases of LibCRC3x would be something like the following:
 *
 * @code{.cpp}
 * #include <vector>
 *
 * #include <crcx/crc3x.h>
 *
 * using namespace std;
 * using nampespace ::crc3x;
 *
 * int main() {
 *   vector<uin8t_t> data = { 'W' };
 *   // Initialize an 8-bit CRC with polynomial 0x[1]07
 *   // The C++ API checks template parameters for correctness
 *   using Crc3x = Crc<8,uint8_t,0x07>;
 *   Crc3x crc(0, 0, false, false);
 *   crc.update(data.begin(), data.end());
 *   Crc3x::crc_type result = crc.fini();
 *   // The value of result should be uint8_t(0xa2).
 *   return 0;
 * }
 * @endcode
 *
 */

#ifndef CRC3X_H_
#define CRC3X_H_

#include <array>
#include <iterator>
#include <type_traits>

namespace crc3x {

/**
 * Reflect the least-significant @p M bits of @p x
 *
 * This function limits @p M to @code{.c}8 * sizeof(@ref T)@endcode bits.
 *
 * It does not fail.
 *
 * @tparam T  a primitive integer type (e.g. uint8_t, uint16_t, uintmax_t)
 *
 * @param x  the variable containing the bits to reflect
 * @param M  the number of bits to reflect
 * @return   x with the least-significant @p M bits of @p x reflected
 */
template <typename T> static constexpr T reflect(const T &x, const size_t &M) {

  static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type");

  const size_t m = std::min(M, 8 * sizeof(T));

  T y = 0;

  for (size_t i = 0; i < m; ++i) {
    bool bit = (x >> i) & 1;
    y |= bit << ((m - 1) - i);
  }

  return y;
}

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
// Adapted from https://stackoverflow.com/a/19016627
template <size_t... Is> struct seq {};
template <size_t N, size_t... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};
template <size_t... Is> struct gen_seq<0, Is...> : seq<Is...> {};

template <class Generator, std::size_t... Is>
constexpr auto generate_array_helper(Generator g, seq<Is...>)
    -> std::array<decltype(g(std::size_t{})), sizeof...(Is)> {
  return {{g(Is)...}};
}

template <std::size_t N, class Generator>
constexpr auto generate_array(Generator g)
    -> decltype(generate_array_helper(g, gen_seq<N>{})) {
  return generate_array_helper(g, gen_seq<N>{});
}
#endif /* !defined(DOXYGEN_SHOULD_SKIP_THIS) */

/**
 * A structure that generates CRC tables
 *
 * The CRC table values that are produced with this are all
 * compile-time constant (i.e. constexpr). This has the advantage
 * that there is no runtime overhead cost to generating the tables
 * regardless of the CRC algorithm used.
 *
 * The generator is typically not used directly by applications
 * but by instantiating @ref Crc.
 *
 * @tparam T           the storage type for the CRC LUT
 * @tparam N           the number of bits in the CRC
 * @tparam polynomial  the CRC polynomial
 *
 * @see <a
 * href="https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Generating_the_tables">Computation
 * of cyclic redundancy checks: Generating the tables</a>
 */
template <typename T, std::size_t N, std::uintmax_t polynomial>
struct generator {

  /// Verify that template parameters are acceptable
  static constexpr void checkTemplateParameters() {
    static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type");
    static_assert(0 != N, "CRC length must be non-zero");
    static_assert(0 == N % 8, "CRC length must be a multiple of 8");
    static_assert(N <= 8 * sizeof(T), "CRC length must be <= 8 * sizeof(T)");
    static_assert(T(0) != polynomial, "CRC polynomial must be non-zero");
    static_assert(T(0) == (polynomial & (~mask())),
                  "CRC polynomial must be N bits");
  }

  /// Return a mask with the most-significant bit of the CRC set
  static constexpr T msb() {
    static_assert(N <= 8 * sizeof(T), "CRC length must be <= 8 * sizeof(T)");
    return T(1ULL << (N - 1));
  }

  /// Return a mask all of the bits in the CRC set
  static constexpr T mask() {
    static_assert(N <= 8 * sizeof(T), "CRC length must be <= 8 * sizeof(T)");
    if (N == 8 * sizeof(T)) {
      return T(-1);
    } else {
      return T((1ULL << N) - 1);
    }
  }

  /// Perform one CRC table entry for for array position, @p x
  static constexpr T func(T x) {

    checkTemplateParameters();

    T index = (x << (N - 8)) & mask();
    for (auto bit = 0; bit < 8; ++bit) {
      if (0 != (index & msb())) {
        index <<= 1;
        index ^= polynomial;
      } else {
        index <<= 1;
      }
    }

    return index & mask();
  }
};

/**
 * The main LibCRC3x class
 *
 * This structure contains all of the necessary context to compute the cyclic
 * redundancy check for binary data.
 * The "normal" polynomial representation is used. It only works for CRC's that
 * are a multiple of 8 bits in length.
 *
 * @tparam N           The length of the CRC in bits
 * @tparam T           The storage type for the CRC lookup table
 * @tparam polynomial  The polynomial specifier
 *
 * @see <a href="https://en.wikipedia.org/wiki/Cyclic_redundancy_check">Cyclic
 * redundancy check (CRC)</a>
 * @see <a
 * href="https://en.wikipedia.org/wiki/Cyclic_redundancy_check#Polynomial_representations_of_cyclic_redundancy_checks">Polynomial
 * representations of cyclic redundancy checks</a>
 */
template <std::size_t N, typename T, T polynomial> class Crc {

public:
  /**
   * Create a Crc instance
   *
   * This constructor is used to create an @p Crc.N -bit CRC object
   * using @p Crc.T as the storage type and with a given @p Crc.polynomial.
   *
   * @param initializer    the initial value stored in the @p Crc.lfsr
   * @param finalizer      the final value xor'ed with the @p Crc.lfsr
   * @param reflectInput   perform a bitwise reversal of each input byte
   * @param reflectOutput  perform a bitwise reversal of the result of the CRC
   * calculation
   */
  Crc(T initializer, T finalizer, bool reflectInput, bool reflectOutput)
      : initializer(initializer), finalizer(finalizer),
        reflectInput(reflectInput), reflectOutput(reflectOutput),
        lfsr(initializer) {}

  /**
   * Update the CRC calculation with new @p data
   *
   * This function reflects the input data if specified by @ref Crc.reflectInput
   * is specified.
   *
   * @param data  the data with which the CRC should be updated
   */
  void update(uint8_t data) {

    if (reflectInput) {
      data = reflect(data, 8);
    }

    // https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Multi-bit_computation
    uint8_t upper_byte = (lfsr >> (N - 8));
    uint8_t idx = data ^ upper_byte;

    if (N <= 8) {
      lfsr = 0;
    } else {
      lfsr <<= 8;
    }
    lfsr &= generator<T, N, polynomial>::mask();
    lfsr ^= table[idx];
  }

  /**
   * Update the CRC calculation multiple times
   *
   * @param begin  the beginning of the data with which the CRC should be
   * updated
   * @param end    the end of the data with which the CRC should be updated
   */
  template <class iterator_type>
  void update(iterator_type begin, iterator_type end) {
    for (auto it = begin; it != end; it++) {
      update(*it);
    }
  }

  /**
   * Finalize a CRC calculation
   *
   * This function xor's the @p Crc.lfsr with the value specified by @p
   * Crc.finalizer.
   *
   * This function reflects the input data if specified by @p Crc.reflectInput.
   */
  T fini() {
    lfsr ^= finalizer;
    lfsr &= generator<T, N, polynomial>::mask();

    if (reflectOutput) {
      return reflect(lfsr, N);
    } else {
      return lfsr;
    }
  }

protected:
  /// the initial value stored in the @p Crc.lfsr
  const T initializer;
  /// the final value xor'ed with the @p Crc.lfsr
  const T finalizer;
  /// perform a bitwise reversal of each input byte
  const bool reflectInput;
  /// perform a bitwise reversal of the result of the CRC
  const bool reflectOutput;

  /// the modeled linear feedback shift register
  T lfsr;

public:
  /// an alias for @p Crc.T
  using crc_type = T;

  /**
   * A lookup table for the CRC value of each possible input byte
   *
   * The entire CRC table is compile-time constant and translates
   * to numeric literals (i.e. it is constexpr). This has the
   * advantage that there is only one copy of the table at runtime.
   * It also means there is no runtime overhead or cost to generating
   * the tables because they are generated at compile time, regardless
   * of the CRC algorithm used. It also allows static assertions to
   * throw errors if incorrect parameters are provided.
   */
  inline static constexpr auto table =
      generate_array<256>(&generator<T, N, polynomial>::func);
};

} /* namespace crc3x */

#endif /* CRC3X_H_ */
