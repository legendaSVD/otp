enum chars_format {
    FMT_SCIENTIFIC,
    FMT_FIXED,
    FMT_GENERAL
};
static inline int to_chars(const floating_decimal_64 v, const bool sign, char* const result) {
  uint64_t __output = v.mantissa;
  const int32_t _Ryu_exponent = v.exponent;
  const uint32_t __olength = decimalLength17(__output);
  int32_t _Scientific_exponent = _Ryu_exponent + ((int32_t) __olength) - 1;
  enum chars_format _Fmt;
  int32_t _Lower;
  int32_t _Upper;
  if (__olength == 1) {
    _Lower = -4;
    _Upper = 2;
  } else if (_Scientific_exponent >= 10) {
    _Lower = - (int32_t) (__olength + 2);
    _Upper = 2;
  } else {
    _Lower = - (int32_t) (__olength + 2);
    _Upper = 1;
  }
  if (_Lower <= _Ryu_exponent && _Ryu_exponent <= _Upper) {
    if ((__output >= (1ull << 53) && _Ryu_exponent == 0)
          || (__output > ((1ull << 52) / 5) && _Ryu_exponent == 1)
          || (__output > ((1ull << 51) / 25) && _Ryu_exponent == 2)) {
      _Fmt = FMT_SCIENTIFIC;
    } else {
      _Fmt = FMT_FIXED;
    }
  } else {
    _Fmt = FMT_SCIENTIFIC;
  }
  if (sign) {
    result[0] = '-';
  }
  char* const __result = result + sign;
  if (_Fmt == FMT_FIXED) {
    const int32_t _Whole_digits = (int32_t) (__olength) + _Ryu_exponent;
    uint32_t _Total_fixed_length;
    if (_Ryu_exponent >= 0) {
      _Total_fixed_length = (uint32_t) (_Whole_digits) + 2;
    } else if (_Whole_digits > 0) {
      _Total_fixed_length = __olength + 1;
    } else {
      _Total_fixed_length = (uint32_t) (2 - _Ryu_exponent);
    }
    char* _Mid;
    if (_Ryu_exponent >= 0) {
      _Mid = __result + __olength;
    } else {
      _Mid = __result + _Total_fixed_length;
    }
    if ((__output >> 32) != 0) {
      const uint64_t __q = div1e8(__output);
      uint32_t __output2 = (uint32_t) (__output - 100000000 * __q);
      __output = __q;
      const uint32_t __c = __output2 % 10000;
      __output2 /= 10000;
      const uint32_t __d = __output2 % 10000;
      const uint32_t __c0 = (__c % 100) << 1;
      const uint32_t __c1 = (__c / 100) << 1;
      const uint32_t __d0 = (__d % 100) << 1;
      const uint32_t __d1 = (__d / 100) << 1;
      memcpy(_Mid -= 2, DIGIT_TABLE + __c0, 2);
      memcpy(_Mid -= 2, DIGIT_TABLE + __c1, 2);
      memcpy(_Mid -= 2, DIGIT_TABLE + __d0, 2);
      memcpy(_Mid -= 2, DIGIT_TABLE + __d1, 2);
    }
    uint32_t __output2 = (uint32_t) __output;
    while (__output2 >= 10000) {
#ifdef __clang__
      const uint32_t __c = __output2 - 10000 * (__output2 / 10000);
#else
      const uint32_t __c = __output2 % 10000;
#endif
      __output2 /= 10000;
      const uint32_t __c0 = (__c % 100) << 1;
      const uint32_t __c1 = (__c / 100) << 1;
      memcpy(_Mid -= 2, DIGIT_TABLE + __c0, 2);
      memcpy(_Mid -= 2, DIGIT_TABLE + __c1, 2);
    }
    if (__output2 >= 100) {
      const uint32_t __c = (__output2 % 100) << 1;
      __output2 /= 100;
      memcpy(_Mid -= 2, DIGIT_TABLE + __c, 2);
    }
    if (__output2 >= 10) {
      const uint32_t __c = __output2 << 1;
      memcpy(_Mid -= 2, DIGIT_TABLE + __c, 2);
    } else {
      *--_Mid = (char) ('0' + __output2);
    }
    if (_Ryu_exponent > 0) {
      memset(__result + __olength, '0', (size_t) _Ryu_exponent);
      __result[__olength + (size_t) _Ryu_exponent] = '.';
      __result[__olength + (size_t) _Ryu_exponent + 1] = '0';
    } else if (_Ryu_exponent == 0) {
      __result[__olength] = '.';
      __result[__olength + 1] = '0';
    } else if (_Whole_digits > 0) {
      memmove(__result, __result + 1, (size_t) _Whole_digits);
      __result[_Whole_digits] = '.';
    } else {
      __result[0] = '0';
      __result[1] = '.';
      memset(__result + 2, '0', (size_t) (-_Whole_digits));
    }
    return _Total_fixed_length + sign;
  }
  uint32_t _Scientific_exponent_length;
  if (_Scientific_exponent <= -100) {
    _Scientific_exponent_length = 5;
  } else if (_Scientific_exponent <= -10 || _Scientific_exponent >= 100) {
    _Scientific_exponent_length = 4;
  } else if ((_Scientific_exponent > -10 && _Scientific_exponent < 0) || _Scientific_exponent >= 10) {
    _Scientific_exponent_length = 3;
  } else {
    _Scientific_exponent_length = 2;
  }
  const uint32_t _Total_scientific_length = __olength + 1 +(__olength == 1)
    + _Scientific_exponent_length;
  uint32_t __i = 0;
  if ((__output >> 32) != 0) {
    const uint64_t __q = div1e8(__output);
    uint32_t __output2 = (uint32_t) (__output) - 100000000 * (uint32_t) (__q);
    __output = __q;
    const uint32_t __c = __output2 % 10000;
    __output2 /= 10000;
    const uint32_t __d = __output2 % 10000;
    const uint32_t __c0 = (__c % 100) << 1;
    const uint32_t __c1 = (__c / 100) << 1;
    const uint32_t __d0 = (__d % 100) << 1;
    const uint32_t __d1 = (__d / 100) << 1;
    memcpy(__result + __olength - __i - 1, DIGIT_TABLE + __c0, 2);
    memcpy(__result + __olength - __i - 3, DIGIT_TABLE + __c1, 2);
    memcpy(__result + __olength - __i - 5, DIGIT_TABLE + __d0, 2);
    memcpy(__result + __olength - __i - 7, DIGIT_TABLE + __d1, 2);
    __i += 8;
  }
  uint32_t __output2 = (uint32_t) (__output);
  while (__output2 >= 10000) {
#ifdef __clang__
    const uint32_t __c = __output2 - 10000 * (__output2 / 10000);
#else
    const uint32_t __c = __output2 % 10000;
#endif
    __output2 /= 10000;
    const uint32_t __c0 = (__c % 100) << 1;
    const uint32_t __c1 = (__c / 100) << 1;
    memcpy(__result + __olength - __i - 1, DIGIT_TABLE + __c0, 2);
    memcpy(__result + __olength - __i - 3, DIGIT_TABLE + __c1, 2);
    __i += 4;
  }
  if (__output2 >= 100) {
    const uint32_t __c = (__output2 % 100) << 1;
    __output2 /= 100;
    memcpy(__result + __olength - __i - 1, DIGIT_TABLE + __c, 2);
    __i += 2;
  }
  if (__output2 >= 10) {
    const uint32_t __c = __output2 << 1;
    __result[2] = DIGIT_TABLE[__c + 1];
    __result[0] = DIGIT_TABLE[__c];
  } else {
    __result[0] = (char) ('0' + __output2);
  }
  uint32_t __index;
  if (__olength > 1) {
    __result[1] = '.';
    __index = __olength + 1;
  } else {
    __result[1] = '.';
    __result[2] = '0';
    __index = __olength + 2;
  }
  __result[__index++] = 'e';
  if (_Scientific_exponent < 0) {
    __result[__index++] = '-';
    _Scientific_exponent = -_Scientific_exponent;
  }
  if (_Scientific_exponent >= 100) {
    const int32_t __c = _Scientific_exponent % 10;
    memcpy(__result + __index, DIGIT_TABLE + 2 * (_Scientific_exponent / 10), 2);
    __result[__index + 2] = (char) ('0' + __c);
    __index += 3;
  } else if (_Scientific_exponent >= 10) {
    memcpy(__result + __index, DIGIT_TABLE + 2 * _Scientific_exponent, 2);
    __index += 2;
  } else {
    __result[__index++] = (char) ('0' + _Scientific_exponent);
  }
  return _Total_scientific_length + sign;
}