#ifndef ASMJIT_SUPPORT_SUPPORT_H_INCLUDED
#define ASMJIT_SUPPORT_SUPPORT_H_INCLUDED
#include <asmjit/core/globals.h>
#include <asmjit/support/span.h>
#if defined(_MSC_VER)
  #include <intrin.h>
#elif defined(__BMI2__)
  #include <x86intrin.h>
#endif
ASMJIT_BEGIN_NAMESPACE
#define SUPPORT_ASSERT ASMJIT_ASSERT
#define SUPPORT_INLINE ASMJIT_INLINE
#define SUPPORT_INLINE_NODEBUG ASMJIT_INLINE_NODEBUG
namespace Support {
template<typename... Args>
static SUPPORT_INLINE_NODEBUG void maybe_unused(Args&&...) noexcept {}
enum class ByteOrder {
  kLE = 0,
  kBE = 1,
#if defined(__ARMEB__) || defined(__MIPSEB__) || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__))
  kNative = kBE
#else
  kNative = kLE
#endif
};
template<size_t Size, bool IsUnsigned>
struct std_int;
template<> struct std_int<1, false> { using type = int8_t;   };
template<> struct std_int<1, true > { using type = uint8_t;  };
template<> struct std_int<2, false> { using type = int16_t;  };
template<> struct std_int<2, true > { using type = uint16_t; };
template<> struct std_int<4, false> { using type = int32_t;  };
template<> struct std_int<4, true > { using type = uint32_t; };
template<> struct std_int<8, false> { using type = int64_t;  };
template<> struct std_int<8, true > { using type = uint64_t; };
template<size_t Size, bool IsUnsigned>
using std_int_t = typename std_int<Size, IsUnsigned>::type;
template<size_t Size>
using std_sint_t = typename std_int<Size, false>::type;
template<size_t Size>
using std_uint_t = typename std_int<Size, true>::type;
using BitWord = std_uint_t<sizeof(uintptr_t)>;
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86) || defined(__X86__) || defined(__i386__)
using FastUInt8 = uint8_t;
#else
using FastUInt8 = uint32_t;
#endif
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr T min(const T& a, const T& b) noexcept { return b < a ? b : a; }
template<typename T, typename... Args>
[[nodiscard]]
SUPPORT_INLINE constexpr T min(const T& a, const T& b, Args&&... args) noexcept { return min(min(a, b), std::forward<Args>(args)...); }
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr T max(const T& a, const T& b) noexcept { return a < b ? b : a; }
template<typename T, typename... Args>
[[nodiscard]]
SUPPORT_INLINE constexpr T max(const T& a, const T& b, Args&&... args) noexcept { return max(max(a, b), std::forward<Args>(args)...); }
namespace {
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr std_int_t<sizeof(T), std::is_unsigned_v<T>> as_std_int(const T& value) noexcept {
  return std_int_t<sizeof(T), std::is_unsigned_v<T>>(value);
}
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr std_sint_t<sizeof(T)> as_std_sint(const T& value) noexcept {
  return std_sint_t<sizeof(T)>(value);
}
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr std_uint_t<sizeof(T)> as_std_uint(const T& value) noexcept {
  return std_uint_t<sizeof(T)>(value);
}
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr std_int_t<sizeof(T) >= 4u ? sizeof(T) : 4u, std::is_unsigned_v<T>> as_basic_int(const T& value) noexcept {
  return std_int_t<sizeof(T) >= 4u ? sizeof(T) : 4u, std::is_unsigned_v<T>>(value);
}
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr std_sint_t<sizeof(T) >= 4u ? sizeof(T) : 4u> as_basic_sint(const T& value) noexcept {
  return std_sint_t<sizeof(T) >= 4u ? sizeof(T) : 4u>(value);
}
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr std_uint_t<sizeof(T) >= 4u ? sizeof(T) : 4u> as_basic_uint(const T& value) noexcept {
  return std_uint_t<sizeof(T) >= 4u ? sizeof(T) : 4u>(value);
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_between(const T& value, const T& a, const T& b) noexcept {
  return value >= a && value <= b;
}
template<typename Dst, typename Src, typename Offset>
SUPPORT_INLINE_NODEBUG Dst* offset_ptr(Src* ptr, const Offset& n) noexcept {
  return static_cast<Dst*>(
    static_cast<void*>(static_cast<char*>(static_cast<void*>(ptr)) + n)
  );
}
template<typename Dst, typename Src, typename Offset>
SUPPORT_INLINE_NODEBUG const Dst* offset_ptr(const Src* ptr, const Offset& n) noexcept {
  return static_cast<const Dst*>(
    static_cast<const void*>(static_cast<const char*>(static_cast<const void*>(ptr)) + n)
  );
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG constexpr T neg(const T& value) noexcept { return T(as_std_uint(T(0)) - as_std_uint(value)); }
template<typename A, typename B>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG constexpr bool test(A a, B b) noexcept { return (as_std_uint(a) & as_std_uint(b)) != 0u; }
template<typename T>
SUPPORT_INLINE_NODEBUG constexpr T bool_as(bool b) noexcept { return T(b); }
template<typename Dst, typename Src>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG constexpr Dst bool_as_mask(const Src& b) noexcept { return Dst(neg(as_std_uint(Dst(b)))); }
template<typename... Args>
SUPPORT_INLINE constexpr bool bool_and(Args&&... args) noexcept { return bool((... & bool_as<unsigned>(args))); }
template<typename... Args>
SUPPORT_INLINE constexpr bool bool_or(Args&&... args) noexcept { return bool((... | bool_as<unsigned>(args))); }
template<typename T>
inline constexpr uint32_t bit_size_of = uint32_t(sizeof(T) * 8u);
template<typename T>
inline constexpr T bit_ones = T(~T(0));
template<typename Dst, typename Src>
SUPPORT_INLINE_NODEBUG Dst bit_cast(const Src& x) noexcept {
  static_assert(sizeof(Dst) == sizeof(Src),
                "bit_cast() can only be used to cast between types of the same size");
  if constexpr ((std::is_integral_v<Dst> || std::is_enum_v<Dst>) &&
                (std::is_integral_v<Src> || std::is_enum_v<Src>)) {
    return Dst(x);
  }
  else {
    union { Src src; Dst dst; } v{x};
    return v.dst;
  }
}
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr bool bit_test(const T& value, const N& n) noexcept {
  return (as_std_uint(value) & (as_std_uint(T(1)) << as_std_uint(n))) != 0u;
}
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr T shl(const T& value, const N& n) noexcept { return T(as_std_uint(value) << n); }
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr T shr(const T& value, const N& n) noexcept { return T(as_std_uint(value) >> n); }
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr T sar(const T& value, const N& n) noexcept { return T(as_std_sint(value) >> n); }
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr T ror(const T& value, const N& n) noexcept {
  uint32_t opposite_n =  uint32_t(bit_size_of<T>) - uint32_t(n);
  return T((as_std_uint(value) >> n) | (as_std_uint(value) << opposite_n));
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr uint32_t clz_t(const T& value) noexcept {
  auto w = as_std_uint(value);
  uint32_t n = 0;
  if constexpr (sizeof(T) > 4u) {
    uint32_t m = bool_as_mask<uint32_t>(w <= uint64_t(0xFFFFFFFFu));
    n += 32u & m;
    w >>= 32u & ~m;
  }
  uint32_t v = uint32_t(w & 0xFFFFFFFFu);
  if constexpr (sizeof(T) > 2u) {
    uint32_t m = bool_as_mask<uint32_t>(v <= uint32_t(0xFFFFu));
    n += 16u & m;
    v >>= 16u & ~m;
  }
  if constexpr (sizeof(T) > 1u) {
    uint32_t m = bool_as_mask<uint32_t>(v <= uint32_t(0xFFu));
    n += 8u & m;
    v >>= 8u & ~m;
  }
  {
    uint32_t m = bool_as_mask<uint32_t>(v <= uint32_t(0xFu));
    n += 4u & m;
    v >>= 4u & ~m;
  }
  constexpr uint32_t predicate =
    (4u << (0b0000 * 4u)) | (3u << (0b0001 * 4u)) | (2u << (0b0010 * 4u)) | (2u << (0b0011 * 4u)) |
    (1u << (0b0100 * 4u)) | (1u << (0b0101 * 4u)) | (1u << (0b0110 * 4u)) | (1u << (0b0111 * 4u)) ;
  uint32_t i = v * 4u;
  if constexpr (sizeof(uintptr_t) > 4u) {
    n += uint32_t((uint64_t(predicate) >> i) & 0xFu);
  }
  else {
    n += (predicate >> min<uint32_t>(i, 31u)) & 0xFu;
  }
  return n;
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr uint32_t ctz_t(const T& value) noexcept {
  auto w = as_std_uint(value);
  uint32_t n = 0;
  if constexpr (sizeof(T) > 4u) {
    uint32_t m = bool_as_mask<uint32_t>((w & 0xFFFFFFFFu) == 0u);
    n += 32u & m;
    w >>= 32u & m;
  }
  uint32_t v = uint32_t(w & 0xFFFFFFFFu);
  if constexpr (sizeof(T) > 2u) {
    uint32_t m = bool_as_mask<uint32_t>((v & 0xFFFFu) == 0u);
    n += 16u & m;
    v >>= 16u & m;
  }
  if constexpr (sizeof(T) > 1u) {
    uint32_t m = bool_as_mask<uint32_t>((v & 0xFFu) == 0u);
    n += 8u & m;
    v >>= 8u & m;
  }
  {
    uint32_t m = bool_as_mask<uint32_t>((v & 0xFu) == 0u);
    n += 4u & m;
    v >>= 4u & m;
  }
  v &= 0xFu;
  if constexpr (sizeof(uintptr_t) > 4u) {
    constexpr uint64_t predicate =
      (uint64_t(4u) << (0b0000 * 4u)) | (uint64_t(0u) << (0b0001 * 4u)) | (uint64_t(1u) << (0b0010 * 4u)) | (uint64_t(0u) << (0b0011 * 4u)) |
      (uint64_t(2u) << (0b0100 * 4u)) | (uint64_t(0u) << (0b0101 * 4u)) | (uint64_t(1u) << (0b0110 * 4u)) | (uint64_t(0u) << (0b0111 * 4u)) |
      (uint64_t(3u) << (0b1000 * 4u)) | (uint64_t(0u) << (0b1001 * 4u)) | (uint64_t(1u) << (0b1010 * 4u)) | (uint64_t(0u) << (0b1011 * 4u)) |
      (uint64_t(2u) << (0b1100 * 4u)) | (uint64_t(0u) << (0b1101 * 4u)) | (uint64_t(1u) << (0b1110 * 4u)) | (uint64_t(0u) << (0b1111 * 4u)) ;
    n += uint32_t((predicate >> (v * 4u)) & 0xFu);
  }
  else {
    constexpr uint32_t predicate =
      (3u << (0b0000 * 2u)) | (0u << (0b0001 * 2u)) | (1u << (0b0010 * 2u)) | (0u << (0b0011 * 2u)) |
      (2u << (0b0100 * 2u)) | (0u << (0b0101 * 2u)) | (1u << (0b0110 * 2u)) | (0u << (0b0111 * 2u)) |
      (3u << (0b1000 * 2u)) | (0u << (0b1001 * 2u)) | (1u << (0b1010 * 2u)) | (0u << (0b1011 * 2u)) |
      (2u << (0b1100 * 2u)) | (0u << (0b1101 * 2u)) | (1u << (0b1110 * 2u)) | (0u << (0b1111 * 2u)) ;
    n += uint32_t((predicate >> (v * 2u)) & 0x3u);
    n += uint32_t(v == 0);
  }
  return n;
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const T& x) noexcept { return clz_t(as_std_uint(x)); }
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const T& x) noexcept { return ctz_t(as_std_uint(x)); }
#if defined(__GNUC__)
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const uint8_t& x) noexcept { return uint32_t(__builtin_clz(uint32_t(x))) - 24u; }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const uint16_t& x) noexcept { return uint32_t(__builtin_clz(uint32_t(x))) - 16u; }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const uint32_t& x) noexcept { return uint32_t(__builtin_clz(x)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const uint64_t& x) noexcept { return uint32_t(__builtin_clzll(x)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const uint8_t& x) noexcept { return uint32_t(__builtin_ctz(uint32_t(x) | 0x10u)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const uint16_t& x) noexcept { return uint32_t(__builtin_ctz(uint32_t(x) | 0x1000u)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const uint32_t& x) noexcept { return uint32_t(__builtin_ctz(x)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const uint64_t& x) noexcept { return uint32_t(__builtin_ctzll(x)); }
#elif defined(_MSC_VER)
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const uint32_t& x) noexcept { unsigned long i; _BitScanReverse(&i, x); return uint32_t(i ^ 31); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const uint32_t& x) noexcept { unsigned long i; _BitScanForward(&i, x); return uint32_t(i); }
#if defined(_M_X64) || defined(_M_ARM64)
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz_impl(const uint64_t& x) noexcept { unsigned long i; _BitScanReverse64(&i, x); return uint32_t(i ^ 63); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz_impl(const uint64_t& x) noexcept { unsigned long i; _BitScanForward64(&i, x); return uint32_t(i); }
#endif
#endif
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t clz(const T& x) noexcept { return clz_impl(as_std_uint(x)); }
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t ctz(const T& x) noexcept { return ctz_impl(as_std_uint(x)); }
template<auto V>
inline constexpr uint32_t ctz_const = ctz_t(as_std_uint(V));
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG constexpr uint32_t popcnt_t(const T& value) noexcept {
  auto v = as_basic_uint(value);
  if constexpr (sizeof(T) <= 4) {
    v = v - ((v >> 1) & 0x55555555u);
    v = (v & 0x33333333u) + ((v >> 2) & 0x33333333u);
    return (((v + (v >> 4)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24;
  }
  else {
    if constexpr (sizeof(uintptr_t) >= 8) {
      v = v - ((v >> 1) & 0x5555555555555555u);
      v = (v & 0x3333333333333333u) + ((v >> 2) & 0x3333333333333333u);
      return uint32_t((((v + (v >> 4)) & 0x0F0F0F0F0F0F0F0Fu) * 0x0101010101010101u) >> 56);
    }
    else {
      uint32_t lo = uint32_t(v & 0xFFFFFFFFu);
      uint32_t hi = uint32_t(v >> 32);
      return popcnt_t(lo) + popcnt_t(hi);
    }
  }
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t popcnt_impl(const T& value) noexcept { return popcnt_t(value); }
#if defined(__GNUC__)
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t popcnt_impl(const uint8_t& value) noexcept { return uint32_t(__builtin_popcount(value)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t popcnt_impl(const uint16_t& value) noexcept { return uint32_t(__builtin_popcount(value)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t popcnt_impl(const uint32_t& value) noexcept { return uint32_t(__builtin_popcount(value)); }
template<>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t popcnt_impl(const uint64_t& value) noexcept { return uint32_t(__builtin_popcountll(value)); }
#endif
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t popcnt(const T& value) noexcept { return popcnt_impl(as_std_uint(value)); }
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool has_at_least_2_bits_set(const T& value) noexcept {
  auto v = as_basic_uint(value);
  return !(v & (v - 1u));
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr T blsi(const T& value) noexcept { return T(as_std_uint(value) & neg(as_std_uint(value))); }
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG constexpr T lsb_mask_t(const N& n) noexcept {
  using U = std_uint_t<sizeof(T)>;
  if constexpr (sizeof(U) < sizeof(uintptr_t)) {
    return T(U((uintptr_t(1) << n) - uintptr_t(1)));
  }
  else {
    return n ? T(shr(bit_ones<T>, bit_size_of<T> - size_t(n))) : T(0);
  }
}
#if defined(__BMI2__)
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T lsb_mask_impl(const N& n) noexcept {
  if constexpr (sizeof(T) == 4) {
    return uint32_t(_bzhi_u32(bit_ones<uint32_t>, uint32_t(n)));
  }
  else if constexpr (sizeof(T) == 8) {
    return uint64_t(_bzhi_u64(bit_ones<uint64_t>, uint32_t(n)));
  }
  else {
    return lsb_mask_t<T, N>(n);
  }
}
#else
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T lsb_mask_impl(const N& n) noexcept { return lsb_mask_t<T, N>(n); }
#endif
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T lsb_mask(const N& n) noexcept { return T(lsb_mask_impl<std_uint_t<sizeof(T)>, N>(n)); }
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T constexpr lsb_mask_const(const N& n) noexcept { return T(lsb_mask_t<std_uint_t<sizeof(T)>, N>(n)); }
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr T msb_mask(const N& n) noexcept {
  using U = std_uint_t<sizeof(T)>;
  if constexpr (sizeof(U) < sizeof(uintptr_t)) {
    return T(bit_ones<uintptr_t> >> (bit_size_of<uintptr_t> - n));
  }
  else {
    return T(sar(U(n != 0) << (bit_size_of<U> - 1), n ? uint32_t(n - 1) : uint32_t(0)));
  }
}
template<typename T, typename Index>
[[nodiscard]]
SUPPORT_INLINE constexpr T bit_mask(const Index& idx) noexcept { return (1u << as_basic_uint(idx)); }
template<typename T, typename Index, typename... Args>
[[nodiscard]]
SUPPORT_INLINE constexpr T bit_mask(const Index& idx, Args... args) noexcept { return bit_mask<T>(idx) | bit_mask<T>(args...); }
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T fill_trailing_bits(const T& value) noexcept {
  auto v = as_basic_uint(value);
  uint32_t leading_count = clz(v | 1u);
  return T(((bit_ones<decltype(v)> >> 1u) >> leading_count) | v);
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_lsb_mask(const T& x) noexcept {
  return x && ((std_uint(x) + 1u) & std_uint(x)) == 0;
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_consecutive_mask(const T& value) noexcept {
  return value && is_lsb_mask((as_std_uint(value) - 1u) | as_std_uint(value));
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_power_of_2(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U x_minus_1 = U(U(x) - U(1));
  return U(U(x) ^ x_minus_1) > x_minus_1;
}
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_power_of_2_up_to(T x, N n) noexcept {
  using U = std::make_unsigned_t<T>;
  U x_minus_1 = U(U(x) - U(1));
  return bool_and(x_minus_1 < U(n), !(U(x) & x_minus_1));
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_zero_or_power_of_2(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  return !(U(x) & (U(x) - U(1)));
}
template<typename T, typename N>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_zero_or_power_of_2_up_to(T x, N n) noexcept {
  using U = std::make_unsigned_t<T>;
  return bool_and(U(x) <= U(n), !(U(x) & (U(x) - U(1))));
}
template<size_t N, typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_int_n(const T& x) noexcept {
  constexpr size_t bit_size = bit_size_of<T>;
  if constexpr (bit_size < N || (bit_size == N && std::is_signed_v<T>)) {
    return true;
  }
  else if constexpr (std::is_signed_v<T>) {
    using U = std_uint_t<sizeof(T)>;
    T maximum_value = T(lsb_mask_const<U>(N - 1));
    T minimum_value = T(-maximum_value - 1);
    return bool_and(x >= minimum_value, x <= maximum_value);
  }
  else {
    T maximum_value = lsb_mask_const<T>(N - 1);
    return x <= maximum_value;
  }
}
template<size_t N, typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_uint_n(const T& x) noexcept {
  constexpr size_t bit_size = bit_size_of<T>;
  if constexpr (bit_size <= N && std::is_unsigned_v<T>) {
    return true;
  }
  else if constexpr (bit_size <= N && std::is_signed_v<T>) {
    return x >= T(0);
  }
  else {
    return as_std_uint(x) <= lsb_mask_const<std_uint_t<sizeof(T)>>(N);
  }
}
template<typename X, typename Y>
[[nodiscard]]
SUPPORT_INLINE constexpr bool is_aligned(X base, Y alignment) noexcept {
  using U = std_uint_t<sizeof(X)>;
  return ((U)base % (U)alignment) == 0;
}
template<typename X, typename Y>
[[nodiscard]]
SUPPORT_INLINE constexpr X align_up(X x, Y alignment) noexcept {
  using U = std_uint_t<sizeof(X)>;
  return (X)( ((U)x + ((U)(alignment) - 1u)) & ~((U)(alignment) - 1u) );
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE constexpr T align_up_power_of_2(T x) noexcept {
  using U = std_uint_t<sizeof(T)>;
  return (T)(fill_trailing_bits(U(x) - 1u) + 1u);
}
template<typename X, typename Y>
[[nodiscard]]
SUPPORT_INLINE constexpr std_uint_t<sizeof(X)> align_up_diff(X base, Y alignment) noexcept {
  using U = std_uint_t<sizeof(X)>;
  return align_up(U(base), alignment) - U(base);
}
template<typename X, typename Y>
[[nodiscard]]
SUPPORT_INLINE constexpr X align_down(X x, Y alignment) noexcept {
  using U = std_uint_t<sizeof(X)>;
  return (X)( (U)x & ~((U)(alignment) - 1u) );
}
template<auto Flag, typename T>
SUPPORT_INLINE_NODEBUG constexpr decltype(Flag) bool_as_flag(const T& v) noexcept {
  using FlagType = decltype(Flag);
  return FlagType(as_std_uint(v) << ctz_const<Flag>);
}
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint16_t byteswap16(uint16_t x) noexcept {
  return uint16_t(((x >> 8) & 0xFFu) | ((x & 0xFFu) << 8));
}
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint32_t byteswap32(uint32_t x) noexcept {
  return (x << 24) | (x >> 24) | ((x << 8) & 0x00FF0000u) | ((x >> 8) & 0x0000FF00);
}
[[nodiscard]]
SUPPORT_INLINE_NODEBUG uint64_t byteswap64(uint64_t x) noexcept {
#if (defined(__GNUC__) || defined(__clang__))
  return uint64_t(__builtin_bswap64(uint64_t(x)));
#elif defined(_MSC_VER)
  return uint64_t(_byteswap_uint64(uint64_t(x)));
#else
  return (uint64_t(byteswap32(uint32_t(uint64_t(x) >> 32        )))      ) |
         (uint64_t(byteswap32(uint32_t(uint64_t(x) & 0xFFFFFFFFu))) << 32) ;
#endif
}
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T byteswap(T x) noexcept {
  static_assert(std::is_integral_v<T>, "byteswap() expects the given type to be integral");
  if constexpr (sizeof(T) == 8) {
    return T(byteswap64(uint64_t(x)));
  }
  else if constexpr (sizeof(T) == 4) {
    return T(byteswap32(uint32_t(x)));
  }
  else if constexpr (sizeof(T) == 2) {
    return T(byteswap16(uint16_t(x)));
  }
  else {
    static_assert(sizeof(T) == 1, "byteswap() can be used with a type of size 1, 2, 4, or 8");
    return x;
  }
}
template<typename T>
SUPPORT_INLINE T add_overflow_t(T x, T y, FastUInt8* of) noexcept {
  using U = std::make_unsigned_t<T>;
  U result = U(U(x) + U(y));
  *of = FastUInt8(*of | FastUInt8(std::is_unsigned_v<T> ? result < U(x) : T((U(x) ^ ~U(y)) & (U(x) ^ result)) < 0));
  return T(result);
}
template<typename T>
SUPPORT_INLINE T sub_overflow_t(T x, T y, FastUInt8* of) noexcept {
  using U = std::make_unsigned_t<T>;
  U result = U(U(x) - U(y));
  *of = FastUInt8(*of | FastUInt8(std::is_unsigned_v<T> ? result > U(x) : T((U(x) ^ U(y)) & (U(x) ^ result)) < 0));
  return T(result);
}
template<typename T>
SUPPORT_INLINE T mul_overflow_t(T x, T y, FastUInt8* of) noexcept {
  using I = std_int_t<sizeof(T) * 2, std::is_unsigned_v<T>>;
  using U = std::make_unsigned_t<I>;
  U mask = bit_ones<U>;
  if constexpr (std::is_signed_v<T>) {
    U prod = U(I(x)) * U(I(y));
    *of = FastUInt8(*of | FastUInt8(I(prod) < I(std::numeric_limits<T>::lowest()) || I(prod) > I(std::numeric_limits<T>::max())));
    return T(I(prod & mask));
  }
  else {
    U prod = U(x) * U(y);
    *of = FastUInt8(*of | FastUInt8((prod & ~mask) != 0));
    return T(prod & mask);
  }
}
template<>
SUPPORT_INLINE int64_t mul_overflow_t(int64_t x, int64_t y, FastUInt8* of) noexcept {
  int64_t result = int64_t(uint64_t(x) * uint64_t(y));
  *of = FastUInt8(*of | FastUInt8(x && (result / x != y)));
  return result;
}
template<>
SUPPORT_INLINE uint64_t mul_overflow_t(uint64_t x, uint64_t y, FastUInt8* of) noexcept {
  uint64_t result = x * y;
  *of = FastUInt8(*of | FastUInt8(y != 0 && bit_ones<uint64_t> / y < x));
  return result;
}
template<typename T> SUPPORT_INLINE T add_overflow_impl(const T& x, const T& y, FastUInt8* of) noexcept { return add_overflow_t<T>(x, y, of); }
template<typename T> SUPPORT_INLINE T sub_overflow_impl(const T& x, const T& y, FastUInt8* of) noexcept { return sub_overflow_t<T>(x, y, of); }
template<typename T> SUPPORT_INLINE T mul_overflow_impl(const T& x, const T& y, FastUInt8* of) noexcept { return mul_overflow_t<T>(x, y, of); }
#if defined(__GNUC__)
#define SUPPORT_OVERFLOW_ARITHMETIC(func, T, result_type, builtin_func)                   \
template<>                                                                                \
SUPPORT_INLINE T func(const T& x, const T& y, FastUInt8* of) noexcept {                   \
  result_type result;                                                                     \
  *of = FastUInt8(*of | uint8_t(builtin_func((result_type)x, (result_type)y, &result)));  \
  return T(result);                                                                       \
}
SUPPORT_OVERFLOW_ARITHMETIC(add_overflow_impl, int32_t , int               , __builtin_sadd_overflow  )
SUPPORT_OVERFLOW_ARITHMETIC(add_overflow_impl, uint32_t, unsigned int      , __builtin_uadd_overflow  )
SUPPORT_OVERFLOW_ARITHMETIC(add_overflow_impl, int64_t , long long         , __builtin_saddll_overflow)
SUPPORT_OVERFLOW_ARITHMETIC(add_overflow_impl, uint64_t, unsigned long long, __builtin_uaddll_overflow)
SUPPORT_OVERFLOW_ARITHMETIC(sub_overflow_impl, int32_t , int               , __builtin_ssub_overflow  )
SUPPORT_OVERFLOW_ARITHMETIC(sub_overflow_impl, uint32_t, unsigned int      , __builtin_usub_overflow  )
SUPPORT_OVERFLOW_ARITHMETIC(sub_overflow_impl, int64_t , long long         , __builtin_ssubll_overflow)
SUPPORT_OVERFLOW_ARITHMETIC(sub_overflow_impl, uint64_t, unsigned long long, __builtin_usubll_overflow)
SUPPORT_OVERFLOW_ARITHMETIC(mul_overflow_impl, int32_t , int               , __builtin_smul_overflow  )
SUPPORT_OVERFLOW_ARITHMETIC(mul_overflow_impl, uint32_t, unsigned int      , __builtin_umul_overflow  )
SUPPORT_OVERFLOW_ARITHMETIC(mul_overflow_impl, int64_t , long long         , __builtin_smulll_overflow)
SUPPORT_OVERFLOW_ARITHMETIC(mul_overflow_impl, uint64_t, unsigned long long, __builtin_umulll_overflow)
#undef SUPPORT_OVERFLOW_ARITHMETIC
#endif
template<typename T>
SUPPORT_INLINE T add_overflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(add_overflow_impl(as_std_int(x), as_std_int(y), of)); }
template<typename T>
SUPPORT_INLINE T sub_overflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(sub_overflow_impl(as_std_int(x), as_std_int(y), of)); }
template<typename T>
SUPPORT_INLINE T mul_overflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(mul_overflow_impl(as_std_int(x), as_std_int(y), of)); }
template<typename T>
SUPPORT_INLINE T madd_overflow(const T& x, const T& y, const T& addend, FastUInt8* of) noexcept {
  T v = T(mul_overflow_impl(as_std_int(x), as_std_int(y), of));
  return T(add_overflow_impl(as_std_int(v), as_std_int(addend), of));
}
#if defined(__GNUC__) || defined(_MSC_VER)
template<size_t Size>
struct unaligned_uint;
template<> struct unaligned_uint<1> { typedef uint8_t type; };
#if defined(_MSC_VER)
template<> struct unaligned_uint<2> { typedef uint16_t __declspec(align(1)) type; };
template<> struct unaligned_uint<4> { typedef uint32_t __declspec(align(1)) type; };
template<> struct unaligned_uint<8> { typedef uint64_t __declspec(align(1)) type; };
#elif defined(__GNUC__)
template<> struct unaligned_uint<2> { typedef uint16_t __attribute__((__may_alias__, __aligned__(1))) type; };
template<> struct unaligned_uint<4> { typedef uint32_t __attribute__((__may_alias__, __aligned__(1))) type; };
template<> struct unaligned_uint<8> { typedef uint64_t __attribute__((__may_alias__, __aligned__(1))) type; };
#endif
template<size_t Size>
using unaligned_uint_t = typename unaligned_uint<Size>::type;
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T loadu(const void* p) noexcept {
  return bit_cast<T>(std_uint_t<sizeof(T)>(*static_cast<const unaligned_uint_t<sizeof(T)>*>(p)));
}
template<typename T>
SUPPORT_INLINE_NODEBUG void storeu(void* p, const T& x) noexcept {
  *static_cast<unaligned_uint_t<sizeof(T)>*>(p) = bit_cast<std_uint_t<sizeof(T)>>(x);
}
#else
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T loadu(const void* p) noexcept {
  T value;
  memcpy(&value, p, sizeof(T));
  return value;
}
template<typename T>
SUPPORT_INLINE_NODEBUG void storeu(void* p, const T& x) noexcept {
  T value = x;
  memcpy(p, &value, sizeof(T));
}
#endif
template<typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T loada(const void* p) noexcept {
  return *static_cast<const T*>(p);
}
template<ByteOrder BO, typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T loada(const void* p) noexcept {
  if constexpr (BO == ByteOrder::kNative || sizeof(T) == 1u) {
    return loada<T>(p);
  }
  else {
    return bit_cast<T>(byteswap(loada<std_uint_t<sizeof(T)>>(p)));
  }
}
template<ByteOrder BO, typename T>
[[nodiscard]]
SUPPORT_INLINE_NODEBUG T loadu(const void* p) noexcept {
  if constexpr (BO == ByteOrder::kNative || sizeof(T) == 1u) {
    return loadu<T>(p);
  }
  else {
    return bit_cast<T>(byteswap(loadu<std_uint_t<sizeof(T)>>(p)));
  }
}
template<typename T>
SUPPORT_INLINE_NODEBUG void storea(void* p, const T& x) noexcept {
  *static_cast<T*>(p) = x;
}
template<ByteOrder BO, typename T>
SUPPORT_INLINE_NODEBUG void storea(void* p, const T& x) noexcept {
  if constexpr (BO == ByteOrder::kNative || sizeof(T) == 1u) {
    storea(p, x);
  }
  else {
    storea(p, byteswap(bit_cast<std_uint_t<sizeof(T)>>(x)));
  }
}
template<ByteOrder BO, typename T>
SUPPORT_INLINE_NODEBUG void storeu(void* p, const T& x) noexcept {
  if constexpr (BO == ByteOrder::kNative || sizeof(T) == 1u) {
    storeu(p, x);
  }
  else {
    storeu(p, byteswap(bit_cast<std_uint_t<sizeof(T)>>(x)));
  }
}
[[nodiscard]] SUPPORT_INLINE_NODEBUG int8_t load_i8(const void* p) noexcept { return *static_cast<const int8_t*>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint8_t load_u8(const void* p) noexcept { return *static_cast<const uint8_t*>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int16_t loada_i16(const void* p) noexcept { return loada<int16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int16_t loadu_i16(const void* p) noexcept { return loadu<int16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint16_t loada_u16(const void* p) noexcept { return loada<uint16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint16_t loadu_u16(const void* p) noexcept { return loadu<uint16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int16_t loada_i16_le(const void* p) noexcept { return loada<ByteOrder::kLE, int16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int16_t loadu_i16_le(const void* p) noexcept { return loadu<ByteOrder::kLE, int16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint16_t loada_u16_le(const void* p) noexcept { return loada<ByteOrder::kLE, uint16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint16_t loadu_u16_le(const void* p) noexcept { return loadu<ByteOrder::kLE, uint16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int16_t loada_i16_be(const void* p) noexcept { return loada<ByteOrder::kBE, int16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int16_t loadu_i16_be(const void* p) noexcept { return loadu<ByteOrder::kBE, int16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint16_t loada_u16_be(const void* p) noexcept { return loada<ByteOrder::kBE, uint16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint16_t loadu_u16_be(const void* p) noexcept { return loadu<ByteOrder::kBE, uint16_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int32_t loada_i32(const void* p) noexcept { return loada<int32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int32_t loadu_i32(const void* p) noexcept { return loadu<int32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint32_t loada_u32(const void* p) noexcept { return loada<uint32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint32_t loadu_u32(const void* p) noexcept { return loadu<uint32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int32_t loada_i32_le(const void* p) noexcept { return loada<ByteOrder::kLE, int32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int32_t loadu_i32_le(const void* p) noexcept { return loadu<ByteOrder::kLE, int32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint32_t loada_u32_le(const void* p) noexcept { return loada<ByteOrder::kLE, uint32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint32_t loadu_u32_le(const void* p) noexcept { return loadu<ByteOrder::kLE, uint32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int32_t loada_i32_be(const void* p) noexcept { return loada<ByteOrder::kBE, int32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int32_t loadu_i32_be(const void* p) noexcept { return loadu<ByteOrder::kBE, int32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint32_t loada_u32_be(const void* p) noexcept { return loada<ByteOrder::kBE, uint32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint32_t loadu_u32_be(const void* p) noexcept { return loadu<ByteOrder::kBE, uint32_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int64_t loada_i64(const void* p) noexcept { return loada<int64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int64_t loadu_i64(const void* p) noexcept { return loadu<int64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint64_t loada_u64(const void* p) noexcept { return loada<uint64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint64_t loadu_u64(const void* p) noexcept { return loadu<uint64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int64_t loada_i64_le(const void* p) noexcept { return loada<ByteOrder::kLE, int64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int64_t loadu_i64_le(const void* p) noexcept { return loadu<ByteOrder::kLE, int64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint64_t loada_u64_le(const void* p) noexcept { return loada<ByteOrder::kLE, uint64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint64_t loadu_u64_le(const void* p) noexcept { return loadu<ByteOrder::kLE, uint64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int64_t loada_i64_be(const void* p) noexcept { return loada<ByteOrder::kBE, int64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG int64_t loadu_i64_be(const void* p) noexcept { return loadu<ByteOrder::kBE, int64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint64_t loada_u64_be(const void* p) noexcept { return loada<ByteOrder::kBE, uint64_t>(p); }
[[nodiscard]] SUPPORT_INLINE_NODEBUG uint64_t loadu_u64_be(const void* p) noexcept { return loadu<ByteOrder::kBE, uint64_t>(p); }
SUPPORT_INLINE_NODEBUG void store_i8(void* p, int8_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void store_u8(void* p, uint8_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i16(void* p, int16_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i16(void* p, int16_t x) noexcept { storeu(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u16(void* p, uint16_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u16(void* p, uint16_t x) noexcept { storeu(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i16_le(void* p, int16_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i16_le(void* p, int16_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u16_le(void* p, uint16_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u16_le(void* p, uint16_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i16_be(void* p, int16_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i16_be(void* p, int16_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u16_be(void* p, uint16_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u16_be(void* p, uint16_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i32(void* p, int32_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i32(void* p, int32_t x) noexcept { storeu(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u32(void* p, uint32_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u32(void* p, uint32_t x) noexcept { storeu(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i32_le(void* p, int32_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i32_le(void* p, int32_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u32_le(void* p, uint32_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u32_le(void* p, uint32_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i32_be(void* p, int32_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i32_be(void* p, int32_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u32_be(void* p, uint32_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u32_be(void* p, uint32_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i64(void* p, int64_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i64(void* p, int64_t x) noexcept { storeu(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u64(void* p, uint64_t x) noexcept { storea(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u64(void* p, uint64_t x) noexcept { storeu(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i64_le(void* p, int64_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i64_le(void* p, int64_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u64_le(void* p, uint64_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u64_le(void* p, uint64_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_i64_be(void* p, int64_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_i64_be(void* p, int64_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storea_u64_be(void* p, uint64_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
SUPPORT_INLINE_NODEBUG void storeu_u64_be(void* p, uint64_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
}
template<typename T>
struct Enumerate {
  using UnderlyingType = std::underlying_type_t<T>;
  UnderlyingType _begin;
  UnderlyingType _end;
  struct Iterator {
    UnderlyingType value;
    [[nodiscard]]
    SUPPORT_INLINE_NODEBUG T operator*() const { return T(value); }
    SUPPORT_INLINE_NODEBUG void operator++() { value = UnderlyingType(value + UnderlyingType(1)); }
    [[nodiscard]]
    SUPPORT_INLINE_NODEBUG bool operator==(const Iterator& other) const noexcept { return value == other.value; }
    [[nodiscard]]
    SUPPORT_INLINE_NODEBUG bool operator!=(const Iterator& other) const noexcept { return value != other.value; }
  };
  [[nodiscard]]
  SUPPORT_INLINE_NODEBUG Iterator begin() const noexcept { return Iterator{_begin}; }
  [[nodiscard]]
  SUPPORT_INLINE_NODEBUG Iterator end() const noexcept { return Iterator{_end}; }
};
template<typename T>
static SUPPORT_INLINE_NODEBUG Enumerate<T> enumerate(T to = T::kMaxValue) {
  using UnderlyingType = std::underlying_type_t<T>;
  return Enumerate<T>{
    UnderlyingType(0),
    UnderlyingType(UnderlyingType(to) + UnderlyingType(1u))
  };
}
template<typename T>
static SUPPORT_INLINE_NODEBUG Enumerate<T> enumerate(T from, T to) {
  using UnderlyingType = std::underlying_type_t<T>;
  return Enumerate<T>{
    UnderlyingType(from),
    UnderlyingType(UnderlyingType(to) + UnderlyingType(1u))
  };
}
template<typename T>
[[nodiscard]]
static SUPPORT_INLINE constexpr T ascii_to_lower(T c) noexcept { return T(c ^ T(T(c >= T('A') && c <= T('Z')) << 5)); }
template<typename T>
[[nodiscard]]
static SUPPORT_INLINE constexpr T ascii_to_upper(T c) noexcept { return T(c ^ T(T(c >= T('a') && c <= T('z')) << 5)); }
[[nodiscard]]
static SUPPORT_INLINE_NODEBUG size_t str_nlen(const char* s, size_t max_size) noexcept {
  size_t i = 0;
  while (i < max_size && s[i] != '\0')
    i++;
  return i;
}
[[nodiscard]]
static SUPPORT_INLINE constexpr uint32_t hash_char(uint32_t existing_hash, uint32_t c) noexcept {
  return existing_hash * 65599 + c;
}
[[nodiscard]]
static SUPPORT_INLINE_NODEBUG uint32_t hash_string(const char* data, size_t size) noexcept {
  uint32_t hash_code = 0;
  for (uint32_t i = 0; i < size; i++) {
    hash_code = hash_char(hash_code, uint8_t(data[i]));
  }
  return hash_code;
}
[[nodiscard]]
static SUPPORT_INLINE_NODEBUG const char* find_packed_string(const char* p, uint32_t id) noexcept {
  uint32_t i = 0;
  while (i < id) {
    while (p[0]) {
      p++;
    }
    p++;
    i++;
  }
  return p;
}
[[nodiscard]]
static SUPPORT_INLINE int compare_string_views(const char* a_data, size_t a_size, const char* b_data, size_t b_size) noexcept {
  size_t size = min(a_size, b_size);
  for (size_t i = 0; i < size; i++) {
    int c = int(uint8_t(a_data[i])) - int(uint8_t(b_data[i]));
    if (c != 0)
      return c;
  }
  return int(a_size) - int(b_size);
}
struct Set    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { Support::maybe_unused(x); return  y; } };
struct SetNot { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { Support::maybe_unused(x); return ~y; } };
struct And    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x &  y; } };
struct AndNot { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x & ~y; } };
struct NotAnd { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return ~x &  y; } };
struct Xor    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x ^  y; } };
struct Add    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x +  y; } };
struct Sub    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x -  y; } };
struct Min    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return min<T>(x, y); } };
struct Max    { template<typename T> static SUPPORT_INLINE_NODEBUG T op(T x, T y) noexcept { return max<T>(x, y); } };
struct Or {
  template<typename T> static SUPPORT_INLINE_NODEBUG T op(T a, T b) noexcept { return a | b; }
  template<typename T> static SUPPORT_INLINE_NODEBUG T op(T a, T b, T c) noexcept { return a | b | c; }
  template<typename T> static SUPPORT_INLINE_NODEBUG T op(T a, T b, T c, T d) noexcept { return  a | b | c | d; }
};
[[nodiscard]]
static SUPPORT_INLINE constexpr uint32_t bytepack32_4x8(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
  return (ByteOrder::kNative == ByteOrder::kLE)
    ? (a | (b << 8) | (c << 16) | (d << 24))
    : (d | (c << 8) | (b << 16) | (a << 24));
}
template<typename T>
[[nodiscard]]
static SUPPORT_INLINE constexpr uint32_t unpack_u32_at_0(T x) noexcept {
  constexpr uint32_t kShift = ByteOrder::kNative == ByteOrder::kLE ? 0 : 32;
  return uint32_t((uint64_t(x) >> kShift) & 0xFFFFFFFFu);
}
template<typename T>
[[nodiscard]]
static SUPPORT_INLINE constexpr uint32_t unpack_u32_at_1(T x) noexcept {
  constexpr uint32_t kShift = ByteOrder::kNative == ByteOrder::kLE ? 32 : 0;
  return uint32_t((uint64_t(x) >> kShift) & 0xFFFFFFFFu);
}
template<typename T>
class BitWordIterator {
public:
  SUPPORT_INLINE_NODEBUG explicit BitWordIterator(T bit_word) noexcept
    : _bit_word(bit_word) {}
  SUPPORT_INLINE_NODEBUG void init(T bit_word) noexcept { _bit_word = bit_word; }
  [[nodiscard]]
  SUPPORT_INLINE_NODEBUG bool has_next() const noexcept { return _bit_word != 0; }
  [[nodiscard]]
  SUPPORT_INLINE uint32_t next() noexcept {
    SUPPORT_ASSERT(_bit_word != 0);
    uint32_t index = ctz(_bit_word);
    _bit_word &= T(_bit_word - 1);
    return index;
  }
  T _bit_word;
};
namespace Internal {
  template<typename T, class OperatorT, class FullWordOpT>
  static SUPPORT_INLINE void bit_vector_op(T* buf, size_t index, size_t count) noexcept {
    if (count == 0) {
      return;
    }
    size_t word_index = index / bit_size_of<T>;
    size_t bit_index = index % bit_size_of<T>;
    buf += word_index;
    constexpr T fill_mask = bit_ones<T>;
    size_t first_n_bits = min<size_t>(bit_size_of<T> - bit_index, count);
    buf[0] = OperatorT::op(buf[0], (fill_mask >> (bit_size_of<T> - first_n_bits)) << bit_index);
    buf++;
    count -= first_n_bits;
    while (count >= bit_size_of<T>) {
      buf[0] = FullWordOpT::op(buf[0], fill_mask);
      buf++;
      count -= bit_size_of<T>;
    }
    if (count) {
      buf[0] = OperatorT::op(buf[0], fill_mask >> (bit_size_of<T> - count));
    }
  }
}
template<typename T>
static SUPPORT_INLINE_NODEBUG bool bit_vector_get_bit(T* buf, size_t index) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;
  return bool((buf[word_index] >> bit_index) & 0x1u);
}
template<typename T>
static SUPPORT_INLINE_NODEBUG void bit_vector_set_bit(T* buf, size_t index, bool value) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;
  T clear_mask = T(1u) << bit_index;
  T set_mask = T(value) << bit_index;
  buf[word_index] = T((buf[word_index] & ~clear_mask) | set_mask);
}
template<typename T>
static SUPPORT_INLINE_NODEBUG void bit_vector_or_bit(T* buf, size_t index, bool value) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;
  T bit_mask = T(value) << bit_index;
  buf[word_index] |= bit_mask;
}
template<typename T>
static SUPPORT_INLINE_NODEBUG void bit_vector_xor_bit(T* buf, size_t index, bool value) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;
  T bit_mask = T(value) << bit_index;
  buf[word_index] ^= bit_mask;
}
template<typename T>
static SUPPORT_INLINE_NODEBUG void bit_vector_fill(T* buf, size_t index, size_t count) noexcept { Internal::bit_vector_op<T, Or, Set>(buf, index, count); }
template<typename T>
static SUPPORT_INLINE_NODEBUG void bit_vector_clear(T* buf, size_t index, size_t count) noexcept { Internal::bit_vector_op<T, AndNot, SetNot>(buf, index, count); }
template<typename T>
static SUPPORT_INLINE size_t bit_vector_index_of(T* buf, size_t start, bool value) noexcept {
  size_t word_index = start / bit_size_of<T>;
  size_t bit_index = start % bit_size_of<T>;
  T* p = buf + word_index;
  const T fill_mask = bit_ones<T>;
  const T flip_mask = value ? T(0) : fill_mask;
  T bits = (*p ^ flip_mask) & (fill_mask << bit_index);
  for (;;) {
    if (bits) {
      return (size_t)(p - buf) * bit_size_of<T> + ctz(bits);
    }
    bits = *++p ^ flip_mask;
  }
}
template<typename T>
class BitVectorIterator {
public:
  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
  SUPPORT_INLINE_NODEBUG BitVectorIterator(const BitVectorIterator& other) noexcept = default;
  SUPPORT_INLINE_NODEBUG BitVectorIterator(Span<const T> data, size_t start = 0) noexcept {
    init(data, start);
  }
  SUPPORT_INLINE void init(Span<const T> data, size_t start = 0) noexcept {
    const T* ptr = data.data() + (start / bit_size_of<T>);
    size_t idx = align_down(start, bit_size_of<T>);
    size_t end = data.size() * bit_size_of<T>;
    T bit_word = T(0);
    if (idx < end) {
      bit_word = *ptr++ & (bit_ones<T> << (start % bit_size_of<T>));
      while (!bit_word && (idx += bit_size_of<T>) < end) {
        bit_word = *ptr++;
      }
    }
    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bit_word;
  }
  [[nodiscard]]
  SUPPORT_INLINE_NODEBUG bool has_next() const noexcept {
    return _current != T(0);
  }
  [[nodiscard]]
  SUPPORT_INLINE size_t next() noexcept {
    T bit_word = _current;
    SUPPORT_ASSERT(bit_word != T(0));
    uint32_t bit = ctz(bit_word);
    bit_word &= T(bit_word - 1u);
    size_t n = _idx + bit;
    while (!bit_word && (_idx += bit_size_of<T>) < _end) {
      bit_word = *_ptr++;
    }
    _current = bit_word;
    return n;
  }
  [[nodiscard]]
  SUPPORT_INLINE size_t peek_next() const noexcept {
    SUPPORT_ASSERT(_current != T(0));
    return _idx + ctz(_current);
  }
};
template<typename T, class OperatorT>
class BitVectorOpIterator {
public:
  const T* _a_ptr;
  const T* _b_ptr;
  size_t _idx;
  size_t _end;
  T _current;
  SUPPORT_INLINE_NODEBUG BitVectorOpIterator(const T* a_data, const T* b_data, size_t bit_word_count, size_t start = 0) noexcept {
    init(a_data, b_data, bit_word_count, start);
  }
  SUPPORT_INLINE_NODEBUG BitVectorOpIterator(Span<const T> a, Span<const T> b, size_t start = 0) noexcept {
    SUPPORT_ASSERT(a.size() == b.size());
    init(a.data(), b.data(), a.size(), start);
  }
  SUPPORT_INLINE void init(const T* a_data, const T* b_data, size_t bit_word_count, size_t start = 0) noexcept {
    const T* a_ptr = a_data + (start / bit_size_of<T>);
    const T* b_ptr = b_data + (start / bit_size_of<T>);
    size_t idx = align_down(start, bit_size_of<T>);
    size_t end = bit_word_count * bit_size_of<T>;
    T bit_word = T(0);
    if (idx < end) {
      bit_word = OperatorT::op(*a_ptr++, *b_ptr++) & (bit_ones<T> << (start % bit_size_of<T>));
      while (!bit_word && (idx += bit_size_of<T>) < end) {
        bit_word = OperatorT::op(*a_ptr++, *b_ptr++);
      }
    }
    _a_ptr = a_ptr;
    _b_ptr = b_ptr;
    _idx = idx;
    _end = end;
    _current = bit_word;
  }
  [[nodiscard]]
  SUPPORT_INLINE_NODEBUG bool has_next() noexcept {
    return _current != T(0);
  }
  [[nodiscard]]
  SUPPORT_INLINE size_t next() noexcept {
    T bit_word = _current;
    SUPPORT_ASSERT(bit_word != T(0));
    uint32_t bit = ctz(bit_word);
    bit_word &= T(bit_word - 1u);
    size_t n = _idx + bit;
    while (!bit_word && (_idx += bit_size_of<T>) < _end) {
      bit_word = OperatorT::op(*_a_ptr++, *_b_ptr++);
    }
    _current = bit_word;
    return n;
  }
};
enum class SortOrder : uint32_t {
  kAscending  = 0,
  kDescending = 1
};
template<SortOrder kOrder = SortOrder::kAscending>
struct Compare {
  template<typename A, typename B>
  SUPPORT_INLINE_NODEBUG int operator()(const A& a, const B& b) const noexcept {
    return kOrder == SortOrder::kAscending ? int(a > b) - int(a < b) : int(a < b) - int(a > b);
  }
};
template<typename T, typename CompareT = Compare<SortOrder::kAscending>>
static SUPPORT_INLINE void insertion_sort(T* base, size_t size, const CompareT& cmp = CompareT()) noexcept {
  for (T* pm = base + 1; pm < base + size; pm++) {
    for (T* pl = pm; pl > base && cmp(pl[-1], pl[0]) > 0; pl--) {
      std::swap(pl[-1], pl[0]);
    }
  }
}
namespace Internal {
  template<typename T, class CompareT>
  struct QSortImpl {
    static inline constexpr size_t kStackSize = 64u * 2u;
    static inline constexpr size_t kISortThreshold = 7u;
    static void sort(T* base, size_t size, const CompareT& cmp) noexcept {
      T* end = base + size;
      T* stack[kStackSize];
      T** stack_ptr = stack;
      for (;;) {
        if ((size_t)(end - base) > kISortThreshold) {
          T* pi = base + 1;
          T* pj = end - 1;
          std::swap(base[(size_t)(end - base) / 2], base[0]);
          if (cmp(*pi  , *pj  ) > 0) { std::swap(*pi  , *pj  ); }
          if (cmp(*base, *pj  ) > 0) { std::swap(*base, *pj  ); }
          if (cmp(*pi  , *base) > 0) { std::swap(*pi  , *base); }
          for (;;) {
            while (pi < pj   && cmp(*++pi, *base) < 0) continue;
            while (pj > base && cmp(*--pj, *base) > 0) continue;
            if (pi > pj) {
              break;
            }
            std::swap(*pi, *pj);
          }
          std::swap(*base, *pj);
          if (pj - base > end - pi) {
            *stack_ptr++ = base;
            *stack_ptr++ = pj;
            base = pi;
          }
          else {
            *stack_ptr++ = pi;
            *stack_ptr++ = end;
            end = pj;
          }
          SUPPORT_ASSERT(stack_ptr <= stack + kStackSize);
        }
        else {
          if (base != end) {
            insertion_sort(base, (size_t)(end - base), cmp);
          }
          if (stack_ptr == stack) {
            break;
          }
          end = *--stack_ptr;
          base = *--stack_ptr;
        }
      }
    }
  };
}
template<typename T, class CompareT = Compare<SortOrder::kAscending>>
static SUPPORT_INLINE_NODEBUG void sort(T* base, size_t size, const CompareT& cmp = CompareT{}) noexcept {
  Internal::QSortImpl<T, CompareT>::sort(base, size, cmp);
}
template<typename T, size_t N>
struct Array {
  T _data[N];
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  template<typename Index>
  SUPPORT_INLINE  T& operator[](const Index& index) noexcept {
    SUPPORT_ASSERT(as_std_uint(index) < N);
    return _data[as_std_uint(index)];
  }
  template<typename Index>
  SUPPORT_INLINE  const T& operator[](const Index& index) const noexcept {
    SUPPORT_ASSERT(as_std_uint(index) < N);
    return _data[as_std_uint(index)];
  }
  SUPPORT_INLINE constexpr bool operator==(const Array& other) const noexcept {
    for (size_t i = 0; i < N; i++) {
      if (_data[i] != other._data[i]) {
        return false;
      }
    }
    return true;
  }
  SUPPORT_INLINE constexpr bool operator!=(const Array& other) const noexcept {
    return !operator==(other);
  }
  [[nodiscard]]
  SUPPORT_INLINE constexpr bool is_empty() const noexcept { return false; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr size_t size() const noexcept { return N; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr T* data() noexcept { return _data; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T* data() const noexcept { return _data; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr T& front() noexcept { return _data[0]; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T& front() const noexcept { return _data[0]; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr T& back() noexcept { return _data[N - 1]; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T& back() const noexcept { return _data[N - 1]; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr T* begin() noexcept { return _data; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr T* end() noexcept { return _data + N; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T* begin() const noexcept { return _data; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T* end() const noexcept { return _data + N; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T* cbegin() const noexcept { return _data; }
  [[nodiscard]]
  SUPPORT_INLINE constexpr const T* cend() const noexcept { return _data + N; }
  SUPPORT_INLINE Span<T> as_span() noexcept { return Span<T>(_data, N); }
  SUPPORT_INLINE Span<const T> as_span() const noexcept { return Span<const T>(_data, N); }
  SUPPORT_INLINE void swap(Array& other) noexcept {
    for (size_t i = 0; i < N; i++) {
      std::swap(_data[i], other._data[i]);
    }
  }
  SUPPORT_INLINE void fill(const T& value) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = value;
    }
  }
  SUPPORT_INLINE void copy_from(const Array& other) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = other._data[i];
    }
  }
  template<typename Operator>
  SUPPORT_INLINE void combine(const Array& other) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = Operator::op(_data[i], other._data[i]);
    }
  }
  template<typename Operator>
  SUPPORT_INLINE T aggregate(T initial_value = T()) const noexcept {
    T value = initial_value;
    for (size_t i = 0; i < N; i++) {
      value = Operator::op(value, _data[i]);
    }
    return value;
  }
  template<typename Fn>
  SUPPORT_INLINE void for_each(Fn&& fn) noexcept {
    for (size_t i = 0; i < N; i++) {
      fn(_data[i]);
    }
  }
};
#undef SUPPORT_INLINE_NODEBUG
#undef SUPPORT_INLINE
#undef SUPPORT_ASSERT
}
ASMJIT_END_NAMESPACE
#endif