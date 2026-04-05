#ifndef RYU_COMMON_H
#define RYU_COMMON_H
#include <assert.h>
#include <stdint.h>
#include <string.h>
#if defined(_M_IX86) || defined(_M_ARM)
#define RYU_32_BIT_PLATFORM
#endif
static inline uint32_t decimalLength9(const uint32_t v) {
  assert(v < 1000000000);
  if (v >= 100000000) { return 9; }
  if (v >= 10000000) { return 8; }
  if (v >= 1000000) { return 7; }
  if (v >= 100000) { return 6; }
  if (v >= 10000) { return 5; }
  if (v >= 1000) { return 4; }
  if (v >= 100) { return 3; }
  if (v >= 10) { return 2; }
  return 1;
}
static inline int32_t log2pow5(const int32_t e) {
  assert(e >= 0);
  assert(e <= 3528);
  return (int32_t) ((((uint32_t) e) * 1217359) >> 19);
}
static inline int32_t pow5bits(const int32_t e) {
  assert(e >= 0);
  assert(e <= 3528);
  return (int32_t) (((((uint32_t) e) * 1217359) >> 19) + 1);
}
static inline int32_t ceil_log2pow5(const int32_t e) {
  return log2pow5(e) + 1;
}
static inline uint32_t log10Pow2(const int32_t e) {
  assert(e >= 0);
  assert(e <= 1650);
  return (((uint32_t) e) * 78913) >> 18;
}
static inline uint32_t log10Pow5(const int32_t e) {
  assert(e >= 0);
  assert(e <= 2620);
  return (((uint32_t) e) * 732923) >> 20;
}
static inline int copy_special_str(char * const result, const bool sign, const bool exponent, const bool mantissa) {
  if (mantissa) {
    memcpy(result, "NaN", 3);
    return 3;
  }
  if (sign) {
    result[0] = '-';
  }
  if (exponent) {
    memcpy(result + sign, "Infinity", 8);
    return sign + 8;
  }
  memcpy(result + sign, "0.0", 3);
  return sign + 3;
}
static inline uint32_t float_to_bits(const float f) {
  uint32_t bits = 0;
  memcpy(&bits, &f, sizeof(float));
  return bits;
}
static inline uint64_t double_to_bits(const double d) {
  uint64_t bits = 0;
  memcpy(&bits, &d, sizeof(double));
  return bits;
}
#endif