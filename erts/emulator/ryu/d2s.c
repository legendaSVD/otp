#include "ryu.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef RYU_DEBUG
#include <inttypes.h>
#include <stdio.h>
#endif
#include "common.h"
#include "digit_table.h"
#include "d2s_intrinsics.h"
#include "d2s_full_table.h"
#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_EXPONENT_BITS 11
#define DOUBLE_BIAS 1023
static inline uint32_t decimalLength17(const uint64_t v) {
  assert(v < 100000000000000000L);
  if (v >= 10000000000000000L) { return 17; }
  if (v >= 1000000000000000L) { return 16; }
  if (v >= 100000000000000L) { return 15; }
  if (v >= 10000000000000L) { return 14; }
  if (v >= 1000000000000L) { return 13; }
  if (v >= 100000000000L) { return 12; }
  if (v >= 10000000000L) { return 11; }
  if (v >= 1000000000L) { return 10; }
  if (v >= 100000000L) { return 9; }
  if (v >= 10000000L) { return 8; }
  if (v >= 1000000L) { return 7; }
  if (v >= 100000L) { return 6; }
  if (v >= 10000L) { return 5; }
  if (v >= 1000L) { return 4; }
  if (v >= 100L) { return 3; }
  if (v >= 10L) { return 2; }
  return 1;
}
typedef struct floating_decimal_64 {
  uint64_t mantissa;
  int32_t exponent;
} floating_decimal_64;
#include "to_chars.h"
static inline floating_decimal_64 d2d(const uint64_t ieeeMantissa, const uint32_t ieeeExponent) {
  int32_t e2;
  uint64_t m2;
  if (ieeeExponent == 0) {
    e2 = 1 - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
    m2 = ieeeMantissa;
  } else {
    e2 = (int32_t) ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS - 2;
    m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  }
  const bool even = (m2 & 1) == 0;
  const bool acceptBounds = even;
#ifdef RYU_DEBUG
  printf("-> %" PRIu64 " * 2^%d\n", m2, e2 + 2);
#endif
  const uint64_t mv = 4 * m2;
  const uint32_t mmShift = ieeeMantissa != 0 || ieeeExponent <= 1;
  uint64_t vr, vp, vm;
  int32_t e10;
  bool vmIsTrailingZeros = false;
  bool vrIsTrailingZeros = false;
  if (e2 >= 0) {
    const uint32_t q = log10Pow2(e2) - (e2 > 3);
    e10 = (int32_t) q;
    const int32_t k = DOUBLE_POW5_INV_BITCOUNT + pow5bits((int32_t) q) - 1;
    const int32_t i = -e2 + (int32_t) q + k;
    vr = mulShiftAll64(m2, DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mmShift);
#ifdef RYU_DEBUG
    printf("%" PRIu64 " * 2^%d / 10^%u\n", mv, e2, q);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 21) {
      const uint32_t mvMod5 = ((uint32_t) mv) - 5 * ((uint32_t) div5(mv));
      if (mvMod5 == 0) {
        vrIsTrailingZeros = multipleOfPowerOf5(mv, q);
      } else if (acceptBounds) {
        vmIsTrailingZeros = multipleOfPowerOf5(mv - 1 - mmShift, q);
      } else {
        vp -= multipleOfPowerOf5(mv + 2, q);
      }
    }
  } else {
    const uint32_t q = log10Pow5(-e2) - (-e2 > 1);
    e10 = (int32_t) q + e2;
    const int32_t i = -e2 - (int32_t) q;
    const int32_t k = pow5bits(i) - DOUBLE_POW5_BITCOUNT;
    const int32_t j = (int32_t) q - k;
    vr = mulShiftAll64(m2, DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mmShift);
#ifdef RYU_DEBUG
    printf("%" PRIu64 " * 5^%d / 10^%u\n", mv, -e2, q);
    printf("%u %d %d %d\n", q, i, k, j);
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
#endif
    if (q <= 1) {
      vrIsTrailingZeros = true;
      if (acceptBounds) {
        vmIsTrailingZeros = mmShift == 1;
      } else {
        --vp;
      }
    } else if (q < 63) {
      vrIsTrailingZeros = multipleOfPowerOf2(mv, q);
#ifdef RYU_DEBUG
      printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    }
  }
#ifdef RYU_DEBUG
  printf("e10=%d\n", e10);
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("vm is trailing zeros=%s\n", vmIsTrailingZeros ? "true" : "false");
  printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
  int32_t removed = 0;
  uint8_t lastRemovedDigit = 0;
  uint64_t output;
  if (vmIsTrailingZeros || vrIsTrailingZeros) {
    for (;;) {
      const uint64_t vpDiv10 = div10(vp);
      const uint64_t vmDiv10 = div10(vm);
      if (vpDiv10 <= vmDiv10) {
        break;
      }
      const uint32_t vmMod10 = ((uint32_t) vm) - 10 * ((uint32_t) vmDiv10);
      const uint64_t vrDiv10 = div10(vr);
      const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
      vmIsTrailingZeros &= vmMod10 == 0;
      vrIsTrailingZeros &= lastRemovedDigit == 0;
      lastRemovedDigit = (uint8_t) vrMod10;
      vr = vrDiv10;
      vp = vpDiv10;
      vm = vmDiv10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
    printf("d-10=%s\n", vmIsTrailingZeros ? "true" : "false");
#endif
    if (vmIsTrailingZeros) {
      for (;;) {
        const uint64_t vmDiv10 = div10(vm);
        const uint32_t vmMod10 = ((uint32_t) vm) - 10 * ((uint32_t) vmDiv10);
        if (vmMod10 != 0) {
          break;
        }
        const uint64_t vpDiv10 = div10(vp);
        const uint64_t vrDiv10 = div10(vr);
        const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
        vrIsTrailingZeros &= lastRemovedDigit == 0;
        lastRemovedDigit = (uint8_t) vrMod10;
        vr = vrDiv10;
        vp = vpDiv10;
        vm = vmDiv10;
        ++removed;
      }
    }
#ifdef RYU_DEBUG
    printf("%" PRIu64 " %d\n", vr, lastRemovedDigit);
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    if (vrIsTrailingZeros && lastRemovedDigit == 5 && vr % 2 == 0) {
      lastRemovedDigit = 4;
    }
    output = vr + ((vr == vm && (!acceptBounds || !vmIsTrailingZeros)) || lastRemovedDigit >= 5);
  } else {
    bool roundUp = false;
    const uint64_t vpDiv100 = div100(vp);
    const uint64_t vmDiv100 = div100(vm);
    if (vpDiv100 > vmDiv100) {
      const uint64_t vrDiv100 = div100(vr);
      const uint32_t vrMod100 = ((uint32_t) vr) - 100 * ((uint32_t) vrDiv100);
      roundUp = vrMod100 >= 50;
      vr = vrDiv100;
      vp = vpDiv100;
      vm = vmDiv100;
      removed += 2;
    }
    for (;;) {
      const uint64_t vpDiv10 = div10(vp);
      const uint64_t vmDiv10 = div10(vm);
      if (vpDiv10 <= vmDiv10) {
        break;
      }
      const uint64_t vrDiv10 = div10(vr);
      const uint32_t vrMod10 = ((uint32_t) vr) - 10 * ((uint32_t) vrDiv10);
      roundUp = vrMod10 >= 5;
      vr = vrDiv10;
      vp = vpDiv10;
      vm = vmDiv10;
      ++removed;
    }
#ifdef RYU_DEBUG
    printf("%" PRIu64 " roundUp=%s\n", vr, roundUp ? "true" : "false");
    printf("vr is trailing zeros=%s\n", vrIsTrailingZeros ? "true" : "false");
#endif
    output = vr + (vr == vm || roundUp);
  }
  const int32_t exp = e10 + removed;
#ifdef RYU_DEBUG
  printf("V+=%" PRIu64 "\nV =%" PRIu64 "\nV-=%" PRIu64 "\n", vp, vr, vm);
  printf("O=%" PRIu64 "\n", output);
  printf("EXP=%d\n", exp);
#endif
  floating_decimal_64 fd;
  fd.exponent = exp;
  fd.mantissa = output;
  return fd;
}
static inline bool d2d_small_int(const uint64_t ieeeMantissa, const uint32_t ieeeExponent,
  floating_decimal_64* const v) {
  const uint64_t m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  const int32_t e2 = (int32_t) ieeeExponent - DOUBLE_BIAS - DOUBLE_MANTISSA_BITS;
  if (e2 > 0) {
    return false;
  }
  if (e2 < -52) {
    return false;
  }
  const uint64_t mask = (1ull << -e2) - 1;
  const uint64_t fraction = m2 & mask;
  if (fraction != 0) {
    return false;
  }
  v->mantissa = m2 >> -e2;
  v->exponent = 0;
  return true;
}
int d2s_buffered_n(double f, char* result) {
  const uint64_t bits = double_to_bits(f);
#ifdef RYU_DEBUG
  printf("IN=");
  for (int32_t bit = 63; bit >= 0; --bit) {
    printf("%d", (int) ((bits >> bit) & 1));
  }
  printf("\n");
#endif
  const bool ieeeSign = ((bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1) != 0;
  const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
  const uint32_t ieeeExponent = (uint32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1));
  if (ieeeExponent == ((1u << DOUBLE_EXPONENT_BITS) - 1u) || (ieeeExponent == 0 && ieeeMantissa == 0)) {
    return copy_special_str(result, ieeeSign, ieeeExponent, ieeeMantissa);
  }
  floating_decimal_64 v;
  const bool isSmallInt = d2d_small_int(ieeeMantissa, ieeeExponent, &v);
  if (isSmallInt) {
    for (;;) {
      const uint64_t q = div10(v.mantissa);
      const uint32_t r = ((uint32_t) v.mantissa) - 10 * ((uint32_t) q);
      if (r != 0) {
        break;
      }
      v.mantissa = q;
      ++v.exponent;
    }
  } else {
    v = d2d(ieeeMantissa, ieeeExponent);
  }
  return to_chars(v, ieeeSign, result);
}
void d2s_buffered(double f, char* result) {
  const int index = d2s_buffered_n(f, result);
  result[index] = '\0';
}
char* d2s(double f) {
  char* const result = (char*) malloc(25);
  d2s_buffered(f, result);
  return result;
}