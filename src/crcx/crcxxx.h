#ifndef CRCXXX_H_
#define CRCXXX_H_

#include <array>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace crcx {

// See https://stackoverflow.com/a/19016627
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

template <typename T>
constexpr T reflect(const T &x, const size_t & N = 8 * sizeof(T)) {

  const size_t n = std::min( N, 8 * sizeof(T) );

  T y = 0;

  for (size_t i = 0; i < n; ++i) {
    bool bit = (x >> i) & 1;
    y |= bit << ((n - 1) - i);
  }

  return y;
}

template <typename T, std::size_t N, std::uintmax_t polynomial>
struct generator {

  static constexpr void checkTemplateParameters() {
    static_assert(std::is_arithmetic<T>::value, "Not an arithmetic type");
    static_assert(0 != N, "CRC length must be non-zero");
    static_assert(0 == N % 8, "CRC length must be a multiple of 8");
    static_assert(N <= 8 * sizeof(T), "CRC length must be <= 8 * sizeof(T)");
    static_assert(T(0) != polynomial, "CRC polynomial must be non-zero");
    static_assert(T(0) == (polynomial & (~mask())),
                  "CRC polynomial must be N bits");
  }

  static constexpr T msb() {
    static_assert(N <= 8 * sizeof(T), "CRC length must be <= 8 * sizeof(T)");
    return T(1ULL << (N - 1));
  }

  static constexpr T mask() {
    static_assert(N <= 8 * sizeof(T), "CRC length must be <= 8 * sizeof(T)");
    if (N == 8 * sizeof(T)) {
      return T(-1);
    } else {
      return T((1ULL << N) - 1);
    }
  }

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

template <std::size_t N, typename T, std::uintmax_t polynomial> class Crc {
public:
  Crc(T initializer, T finalizer, bool reflectInput, bool reflectOutput)
      : initializer(initializer), finalizer(finalizer),
        reflectInput(reflectInput), reflectOutput(reflectOutput),
        lfsr(initializer) {}

  void update(uint8_t data) {

    if (reflectInput) {
      data = reflect(data,8);
    }

    // https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#Multi-bit_computation
    uint8_t upper_byte = (lfsr >> (N - 8));
    uint8_t idx = data ^ upper_byte;

    lfsr <<= 8;
    lfsr &= generator<T, N, polynomial>::mask();
    lfsr ^= table[idx];
  }

  template <class iterator_type>
  void update(iterator_type begin, iterator_type end) {
    for (auto it = begin; it != end; it++) {
      update(*it);
    }
  }

  T fini() {
    lfsr ^= finalizer;
    lfsr &= generator<T, N, polynomial>::mask();

    if (reflectOutput) {
      return reflect(lfsr, N);
    } else {
      return lfsr;
    }
  }

  // C++17 removes the need for inline here
  inline static constexpr auto table =
      generate_array<256>(&generator<T, N, polynomial>::func);

protected:
  const T initializer;
  const T finalizer;
  const bool reflectInput;
  const bool reflectOutput;

  T lfsr;
};

} /* namespace crcx */

#endif /* CRCXXX_H_ */
