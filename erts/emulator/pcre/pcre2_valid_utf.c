#ifndef PCRE2_PCRE2TEST
#include "pcre2_internal.h"
#endif
#ifndef SUPPORT_UNICODE
int
PRIV(valid_utf)(PCRE2_SPTR string, PCRE2_SIZE length, PCRE2_SIZE *erroroffset)
{
(void)string;
(void)length;
(void)erroroffset;
return 0;
}
#else
int
PRIV(valid_utf)(PCRE2_SPTR string, PCRE2_SIZE length, PCRE2_SIZE *erroroffset)
{
#if defined(ERLANG_INTEGRATION) && !defined(PCRE2_BUILDING_PCRE2TEST)
    return PRIV(yielding_valid_utf)(string, length, erroroffset, NULL);
}
int
PRIV(yielding_valid_utf)(PCRE2_SPTR string, PCRE2_SIZE length, PCRE2_SIZE *erroroffset, struct PRIV(valid_utf_ystate) *ystate)
{
#endif
PCRE2_SPTR p;
uint32_t c;
#if defined(ERLANG_INTEGRATION) && !defined(PCRE2_BUILDING_PCRE2TEST)
register int32_t loops_left;
if (!ystate) {
    loops_left = INT32_MAX;
}
else {
    loops_left = ystate->loops_left;
    if (ystate->yielded) {
        p = ystate->p;
        length = ystate->length;
        ystate->yielded = 0;
        goto restart_validate;
    }
}
#endif
#if PCRE2_CODE_UNIT_WIDTH == 8
for (p = string; length > 0; p++)
  {
  uint32_t ab, d;
#if defined(ERLANG_INTEGRATION) && !defined(PCRE2_BUILDING_PCRE2TEST)
  if (--loops_left <= 0)
    {
    if (ystate)
      {
      ystate->loops_left = 0;
      ystate->yielded = !0;
      ystate->length = length;
      ystate->p = p;
      return PCRE2_ERROR_UTF8_YIELD;
      }
    loops_left = INT32_MAX;
    }
  restart_validate:
#endif
  c = *p;
  length--;
  if (c < 128) continue;
  if (c < 0xc0)
    {
    *erroroffset = (PCRE2_SIZE)(p - string);
    return PCRE2_ERROR_UTF8_ERR20;
    }
  if (c >= 0xfe)
    {
    *erroroffset = (PCRE2_SIZE)(p - string);
    return PCRE2_ERROR_UTF8_ERR21;
    }
  ab = PRIV(utf8_table4)[c & 0x3f];
  if (length < ab)
    {
    *erroroffset = (PCRE2_SIZE)(p - string);
    switch(ab - length)
      {
      case 1: return PCRE2_ERROR_UTF8_ERR1;
      case 2: return PCRE2_ERROR_UTF8_ERR2;
      case 3: return PCRE2_ERROR_UTF8_ERR3;
      case 4: return PCRE2_ERROR_UTF8_ERR4;
      case 5: return PCRE2_ERROR_UTF8_ERR5;
      }
    }
  length -= ab;
  if (((d = *(++p)) & 0xc0) != 0x80)
    {
    *erroroffset = (PCRE2_SIZE)(p - string) - 1;
    return PCRE2_ERROR_UTF8_ERR6;
    }
  switch (ab)
    {
    case 1: if ((c & 0x3e) == 0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 1;
      return PCRE2_ERROR_UTF8_ERR15;
      }
    break;
    case 2:
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 2;
      return PCRE2_ERROR_UTF8_ERR7;
      }
    if (c == 0xe0 && (d & 0x20) == 0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 2;
      return PCRE2_ERROR_UTF8_ERR16;
      }
    if (c == 0xed && d >= 0xa0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 2;
      return PCRE2_ERROR_UTF8_ERR14;
      }
    break;
    case 3:
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 2;
      return PCRE2_ERROR_UTF8_ERR7;
      }
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 3;
      return PCRE2_ERROR_UTF8_ERR8;
      }
    if (c == 0xf0 && (d & 0x30) == 0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 3;
      return PCRE2_ERROR_UTF8_ERR17;
      }
    if (c > 0xf4 || (c == 0xf4 && d > 0x8f))
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 3;
      return PCRE2_ERROR_UTF8_ERR13;
      }
    break;
    case 4:
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 2;
      return PCRE2_ERROR_UTF8_ERR7;
      }
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 3;
      return PCRE2_ERROR_UTF8_ERR8;
      }
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 4;
      return PCRE2_ERROR_UTF8_ERR9;
      }
    if (c == 0xf8 && (d & 0x38) == 0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 4;
      return PCRE2_ERROR_UTF8_ERR18;
      }
    break;
    case 5:
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 2;
      return PCRE2_ERROR_UTF8_ERR7;
      }
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 3;
      return PCRE2_ERROR_UTF8_ERR8;
      }
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 4;
      return PCRE2_ERROR_UTF8_ERR9;
      }
    if ((*(++p) & 0xc0) != 0x80)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 5;
      return PCRE2_ERROR_UTF8_ERR10;
      }
    if (c == 0xfc && (d & 0x3c) == 0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 5;
      return PCRE2_ERROR_UTF8_ERR19;
      }
    break;
    }
  if (ab > 3)
    {
    *erroroffset = (PCRE2_SIZE)(p - string) - ab;
    return (ab == 4)? PCRE2_ERROR_UTF8_ERR11 : PCRE2_ERROR_UTF8_ERR12;
    }
  }
#if defined(ERLANG_INTEGRATION) && !defined(PCRE2_BUILDING_PCRE2TEST)
if (ystate)
  {
  ystate->loops_left = loops_left;
  ystate->yielded = 0;
  }
#endif
return 0;
#elif PCRE2_CODE_UNIT_WIDTH == 16
for (p = string; length > 0; p++)
  {
  c = *p;
  length--;
  if ((c & 0xf800) != 0xd800)
    {
    }
  else if ((c & 0x0400) == 0)
    {
    if (length == 0)
      {
      *erroroffset = (PCRE2_SIZE)(p - string);
      return PCRE2_ERROR_UTF16_ERR1;
      }
    p++;
    length--;
    if ((*p & 0xfc00) != 0xdc00)
      {
      *erroroffset = (PCRE2_SIZE)(p - string) - 1;
      return PCRE2_ERROR_UTF16_ERR2;
      }
    }
  else
    {
    *erroroffset = (PCRE2_SIZE)(p - string);
    return PCRE2_ERROR_UTF16_ERR3;
    }
  }
return 0;
#else
for (p = string; length > 0; length--, p++)
  {
  c = *p;
  if ((c & 0xfffff800u) != 0xd800u)
    {
    if (c > 0x10ffffu)
      {
      *erroroffset = (PCRE2_SIZE)(p - string);
      return PCRE2_ERROR_UTF32_ERR2;
      }
    }
  else
    {
    *erroroffset = (PCRE2_SIZE)(p - string);
    return PCRE2_ERROR_UTF32_ERR1;
    }
  }
return 0;
#endif
}
#endif